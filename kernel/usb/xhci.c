/*
 * Copyright (c) 2025 Antti Tiihala
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * usb/xhci.c
 *      Extensible Host Controller Interface (xHCI)
 */

#include <dancy.h>

struct xhci_slot {
	void *device_context;
	void *io_buffer;

	struct {
		uint32_t *tr;
		int cycle;
		int enqueue;
	} endpoints[32];

	uint32_t trb[4];

	int max_packet_size;
	int context_entries;
	int state;
};

struct xhci_port {
	uint8_t rev_major;
	uint8_t rev_minor;
	char name[6];

	uint32_t *psi;
	int psi_count;

	int slot_type;
	int slot_id;

	uint32_t *portsc;
	uint32_t _task;

	void *xhci;
	int port_id;
	int lock;

	struct dancy_usb_device *dev;
};

struct xhci {
	struct pci_id *pci;
	uint8_t *base;
	size_t size;

	uint32_t irq_count;
	event_t irq_event;

	uint32_t cap_length;
	uint32_t hci_version;
	uint32_t hcs_params[3];
	uint32_t hcc_params[2];
	uint32_t page_size;

	uint32_t db_off;
	uint32_t rts_off;

	uint8_t *base_cap;
	uint8_t *xecp;

	int max_slots;
	int max_intrs;
	int max_ports;
	int max_scratchpads;

	struct xhci_slot *slots;
	struct xhci_port *ports;

	uint32_t *usb_cmd;
	uint32_t *usb_sts;
	uint32_t *dn_ctrl;
	uint32_t *crcr;
	uint32_t *dcbaap;
	uint32_t *config;

	uint32_t *buffer_crcr;
	uint32_t *buffer_crcr_high;
	uint64_t *buffer_dcbaap;

	uint32_t *buffer_er;
	uint64_t *buffer_erst;
	uint64_t *buffer_scratch;

	int buffer_crcr_lock;
	int buffer_crcr_cycle;
	int buffer_crcr_enqueue;

	int buffer_er_cycle;
	int buffer_er_dequeue;

	struct dancy_usb_controller *hci;
};

static void *xhci_alloc(size_t size)
{
	if (size <= 0x1000) {
		phys_addr_t addr = mm_alloc_pages(mm_addr28, 0);
		return pg_map_kernel(addr, 0x1000, pg_uncached);
	}

	if (size <= 0x2000) {
		phys_addr_t addr = mm_alloc_pages(mm_addr28, 1);
		return pg_map_kernel(addr, 0x2000, pg_uncached);
	}

	return NULL;
}

static void *xhci_mm_page(void *page)
{
	static int lock;
	void *r = page;

	spin_lock_yield(&lock);

	{
		static void *array[64];
		static size_t state;

		if (r == NULL) {
			if (state == 0)
				r = xhci_alloc(0x1000);
			else
				r = array[--state];

		} else if (state < sizeof(array) / sizeof(*array)) {
			array[state++] = r;
			r = NULL;
		}
	}

	spin_unlock(&lock);

	return r;
}

static int write_command(struct xhci *xhci, const uint32_t *in, uint32_t *out)
{
	int i, r = DE_WRITE;

	spin_lock_yield(&xhci->buffer_crcr_lock);

	{
		int offset = xhci->buffer_crcr_enqueue * 4;
		uint32_t trb[4];

		for (i = 0; i < 4; i++)
			trb[i] = in[i];

		trb[3] &= 0xFFFFFFFE;
		trb[3] |= ((xhci->buffer_crcr_cycle ? 1u : 0u) << 0);

		for (i = 0; i < 4; i++) {
			xhci->buffer_crcr[offset + i] = trb[i];
			xhci->buffer_crcr_high[offset + i] = 0;

			if (out != NULL)
				out[i] = 0;
		}
	}

	/*
	 * Ring the host controller doorbell (0).
	 */
	cpu_write32((void *)(xhci->base + xhci->db_off), 0);

	/*
	 * Wait for the command completion event.
	 */
	for (i = 0; i < 400; i++) {
		int offset = xhci->buffer_crcr_enqueue * 4;
		uint32_t *p = &xhci->buffer_crcr_high[offset];
		uint32_t completion_trb[4];

		completion_trb[3] = cpu_read32(p + 3);
		completion_trb[2] = cpu_read32(p + 2);
		completion_trb[1] = cpu_read32(p + 1);
		completion_trb[0] = cpu_read32(p + 0);

		if (((completion_trb[3] >> 10) & 0x3F) == 33) {
			int valid_completion_trb = 0;

			p = &xhci->buffer_crcr[offset];

			if (completion_trb[0] == (uint32_t)((addr_t)p))
				valid_completion_trb = 1;

			if (completion_trb[1] != 0)
				valid_completion_trb = 0;

			if (valid_completion_trb) {
				if (out != NULL) {
					out[0] = completion_trb[0];
					out[1] = completion_trb[1];
					out[2] = completion_trb[2];
					out[3] = completion_trb[3];
				}
				r = 0;
				break;
			}
		}

		if (i < 100)
			cpu_halt(1);
		else
			task_sleep(10);
	}

	/*
	 * Advance the enqueue pointer.
	 */
	if ((xhci->buffer_crcr_enqueue += 1) >= 127) {
		int offset = xhci->buffer_crcr_enqueue * 4;
		uint32_t *p = &xhci->buffer_crcr[offset];

		/*
		 * Set the ring segment pointer high and low.
		 */
		p[0] = (uint32_t)((addr_t)xhci->buffer_crcr);
		p[1] = 0;
		p[2] = 0;

		/*
		 * Set the cycle bit, toggle cycle, and Link TRB Type (6).
		 */
		{
			uint32_t ctrl = 0;

			ctrl |= ((xhci->buffer_crcr_cycle ? 1u : 0u) << 0);
			ctrl |= (1u << 1);
			ctrl |= (6u << 10);

			p[3] = ctrl;
		}

		xhci->buffer_crcr_cycle = !xhci->buffer_crcr_cycle;
		xhci->buffer_crcr_enqueue = 0;
	}

	spin_unlock(&xhci->buffer_crcr_lock);

	return r;
}

static void advance_enqueue_locked(struct xhci_slot *slot, int i, int chained)
{
	if ((slot->endpoints[i].enqueue += 1) >= 255) {
		int offset = slot->endpoints[i].enqueue * 4;
		uint32_t *p = &slot->endpoints[i].tr[offset];

		/*
		 * Set the ring segment pointer high and low.
		 */
		p[0] = (uint32_t)((addr_t)slot->endpoints[i].tr);
		p[1] = 0;
		p[2] = 0;

		/*
		 * Set the relevant bits, toggle cycle, and Link TRB Type (6).
		 */
		{
			uint32_t ctrl = 0;

			ctrl |= ((slot->endpoints[i].cycle ? 1u : 0u) << 0);
			ctrl |= (1u << 1);
			ctrl |= ((chained ? 1u : 0u) << 4);
			ctrl |= (6u << 10);

			p[3] = ctrl;
		}

		slot->endpoints[i].cycle = !slot->endpoints[i].cycle;
		slot->endpoints[i].enqueue = 0;
	}
}

static int write_request_locked(struct xhci_port *port,
	const struct usb_device_request *request, void *buffer)
{
	struct xhci *xhci = port->xhci;
	struct xhci_slot *slot;
	uint32_t size = 0;
	int i, r = DE_WRITE;

	if (port->slot_id == 0)
		return DE_UNINITIALIZED;

	slot = &xhci->slots[port->slot_id - 1];

	if (slot->state == 0)
		return DE_UNINITIALIZED;

	/*
	 * Write the setup stage TRB.
	 */
	{
		int offset = slot->endpoints[0].enqueue * 4;
		uint32_t *p = &slot->endpoints[0].tr[offset];

		uint32_t transfer_type = 2;
		uint32_t transfer_length = 8;

		if ((request->bmRequestType & 0x80) != 0)
			transfer_type = 3;

		if (request->wLength == 0)
			transfer_type = 0;

		p[0] = 0;
		p[0] |= ((uint32_t)request->bmRequestType << 0);
		p[0] |= ((uint32_t)request->bRequest << 8);
		p[0] |= ((uint32_t)request->wValue << 16);

		p[1] = 0;
		p[1] |= ((uint32_t)request->wIndex << 0);
		p[1] |= ((uint32_t)request->wLength << 16);

		p[2] = 0;
		p[2] |= (transfer_length << 0);

		{
			const uint32_t trb_type = 2;
			const uint32_t immediate_data = 1;

			uint32_t ctrl = 0;

			ctrl |= ((slot->endpoints[0].cycle ? 1u : 0u) << 0);
			ctrl |= (immediate_data << 6);
			ctrl |= (trb_type << 10);
			ctrl |= (transfer_type << 16);

			p[3] = ctrl;
		}

		advance_enqueue_locked(slot, 0, 0);
	}

	/*
	 * Write the data stage TRBs.
	 */
	while (size < request->wLength) {
		int offset = slot->endpoints[0].enqueue * 4;
		uint32_t *p = &slot->endpoints[0].tr[offset];

		uint32_t wLength = request->wLength;
		uint32_t transfer_length, td_size = 0;
		int chained = 0;

		transfer_length = 8;

		if (slot->max_packet_size > 8)
			transfer_length = (uint32_t)slot->max_packet_size;

		if (size + transfer_length < wLength)
			chained = 1;

		while (size + ((td_size + 1) * transfer_length) < wLength)
			td_size += 1;

		if (td_size > 31)
			td_size = 31;

		while (size + transfer_length > wLength)
			transfer_length -= 1;

		p[0] = (uint32_t)((addr_t)buffer) + size;
		p[1] = 0;

		p[2] = 0;
		p[2] |= (transfer_length << 0);
		p[2] |= (td_size << 17);

		{
			const uint32_t trb_type = 3;
			const uint32_t direction = 1;

			uint32_t ctrl = 0;

			ctrl |= ((slot->endpoints[0].cycle ? 1u : 0u) << 0);
			ctrl |= ((chained ? 1u : 0u) << 4);
			ctrl |= (trb_type << 10);
			ctrl |= (direction << 16);

			p[3] = ctrl;
		}

		advance_enqueue_locked(slot, 0, chained);
		size += transfer_length;
	}

	/*
	 * Write the status stage TRB.
	 */
	{
		int offset = slot->endpoints[0].enqueue * 4;
		uint32_t *p = &slot->endpoints[0].tr[offset];

		p[0] = 0;
		p[1] = 0;
		p[2] = 0;

		{
			const uint32_t trb_type = 4;
			const uint32_t interrupt_on_completion = 1;

			uint32_t ctrl = 0;

			ctrl |= ((slot->endpoints[0].cycle ? 1u : 0u) << 0);
			ctrl |= (interrupt_on_completion << 5);
			ctrl |= (trb_type << 10);

			p[3] = ctrl;
		}

		advance_enqueue_locked(slot, 0, 0);
	}

	cpu_write32(&slot->trb[0], 0);
	cpu_write32(&slot->trb[1], 0);
	cpu_write32(&slot->trb[2], 0);
	cpu_write32(&slot->trb[3], 0);

	/*
	 * Ring the doorbell.
	 */
	{
		uint32_t *db = (void *)((addr_t)(xhci->base + xhci->db_off));

		pg_write_memory((phys_addr_t)(db + port->slot_id), 1, 4);
	}

	/*
	 * Wait for the transfer event.
	 */
	for (i = 0; i < 400; i++) {
		uint32_t trb[4];

		trb[3] = cpu_read32(&slot->trb[3]);
		trb[2] = cpu_read32(&slot->trb[2]);
		trb[1] = cpu_read32(&slot->trb[1]);
		trb[0] = cpu_read32(&slot->trb[0]);

		if ((int)((trb[3] >> 24) & 0xFF) == port->slot_id) {
			if (((trb[2] >> 24) & 0xFF) == 1)
				r = 0;
			break;
		}

		if (i < 100)
			cpu_halt(1);
		else
			task_sleep(10);
	}

	return r;
}

static int write_ep_locked(struct xhci_port *port, struct xhci_slot *slot,
	int i, size_t *size, void *buffer)
{
	struct xhci *xhci = port->xhci;
	size_t buffer_size = *size;
	int r = DE_WRITE;

	*size = 0;

	/*
	 * Write the normal TRB.
	 */
	{
		int offset = slot->endpoints[i].enqueue * 4;
		uint32_t *p = &slot->endpoints[i].tr[offset];

		uint32_t transfer_length = (uint32_t)buffer_size;
		uint32_t td_size = 0;

		if (transfer_length > 0x10000)
			transfer_length = 0x10000;

		p[0] = (uint32_t)((addr_t)buffer);
		p[1] = 0;

		p[2] = 0;
		p[2] |= (transfer_length << 0);
		p[2] |= (td_size << 17);

		{
			const uint32_t trb_type = 1;
			const uint32_t interrupt_on_short_packet = 1;
			const uint32_t interrupt_on_completion = 1;

			uint32_t ctrl = 0;

			ctrl |= ((slot->endpoints[i].cycle ? 1u : 0u) << 0);
			ctrl |= (interrupt_on_short_packet << 2);
			ctrl |= (interrupt_on_completion << 5);
			ctrl |= (trb_type << 10);

			p[3] = ctrl;
		}

		advance_enqueue_locked(slot, i, 0);
	}

	cpu_write32(&slot->trb[0], 0);
	cpu_write32(&slot->trb[1], 0);
	cpu_write32(&slot->trb[2], 0);
	cpu_write32(&slot->trb[3], 0);

	/*
	 * Ring the doorbell.
	 */
	{
		uint32_t *db = (void *)((addr_t)(xhci->base + xhci->db_off));
		uint64_t val = (uint64_t)i;

		pg_write_memory((phys_addr_t)(db + port->slot_id), val, 4);
	}

	/*
	 * Wait for the transfer event.
	 */
	for (i = 0; i < 400; i++) {
		uint32_t trb[4];

		trb[3] = cpu_read32(&slot->trb[3]);
		trb[2] = cpu_read32(&slot->trb[2]);
		trb[1] = cpu_read32(&slot->trb[1]);
		trb[0] = cpu_read32(&slot->trb[0]);

		if ((int)((trb[3] >> 24) & 0xFF) == port->slot_id) {
			int completion_code = (int)((trb[2] >> 24) & 0xFF);

			if (completion_code != 1 && completion_code != 13)
				break;

			*size += (buffer_size - (size_t)(trb[2] & 0xFFFFFF));
			r = 0;

			if (*size > buffer_size) {
				*size = buffer_size;
				r = DE_UNEXPECTED;
			}

			break;
		}

		if (i < 100)
			cpu_halt(1);
		else
			task_sleep(10);
	}

	return r;
}

static int u_write_request(struct dancy_usb_device *dev_locked,
	const struct usb_device_request *request, void *buffer)
{
	struct xhci *xhci = dev_locked->hci->hci;
	struct xhci_port *port = NULL;
	struct xhci_slot *slot = NULL;

	size_t wLength = (size_t)request->wLength;
	int r = 0;

	if (wLength > 1024)
		return DE_OVERFLOW;

	if (buffer == NULL)
		buffer = (void *)((addr_t)16);

	if (dev_locked->port >= 0 && dev_locked->port <= xhci->max_ports)
		port = &xhci->ports[dev_locked->port - 1];

	if (port == NULL)
		return DE_UNEXPECTED;

	spin_lock_yield(&port->lock);

	if (port->slot_id > 0)
		slot = &xhci->slots[port->slot_id - 1];

	while (slot != NULL && slot->state > 1) {
		int device_to_host = (request->bmRequestType & 0x80) != 0;

		if (device_to_host)
			memset(slot->io_buffer, 0, wLength);
		else
			memcpy(slot->io_buffer, buffer, wLength);

		if (write_request_locked(port, request, slot->io_buffer)) {
			if (device_to_host)
				r = DE_READ;
			else
				r = DE_WRITE;
			break;
		}

		if (device_to_host)
			memcpy(buffer, slot->io_buffer, wLength);

		break;
	}

	spin_unlock(&port->lock);

	return r;
}

static int u_write_ep(struct dancy_usb_device *dev_locked,
	const struct usb_endpoint_descriptor *endpoint,
	size_t *size, void *buffer)
{
	struct xhci *xhci = dev_locked->hci->hci;
	struct xhci_port *port = NULL;
	struct xhci_slot *slot = NULL;

	size_t buffer_size = *size;
	int r = 0;

	*size = 0;

	if (dev_locked->port >= 0 && dev_locked->port <= xhci->max_ports)
		port = &xhci->ports[dev_locked->port - 1];

	if (port == NULL)
		return DE_UNEXPECTED;

	spin_lock_yield(&port->lock);

	if (port->slot_id > 0)
		slot = &xhci->slots[port->slot_id - 1];

	while (slot != NULL && slot->state > 1 && buffer_size > 0) {
		int ep_number = (int)(endpoint->bEndpointAddress & 0x0F);
		int ep_in = ((endpoint->bEndpointAddress & 0x80) != 0);
		int i = (ep_number << 1) + ep_in;

		if (ep_number == 0) {
			r = DE_UNEXPECTED;
			break;
		}

		if (slot->endpoints[i].tr == NULL) {
			r = DE_UNINITIALIZED;
			break;
		}

		if (buffer_size > 0x1000)
			buffer_size = 0x1000;

		if (ep_in != 0)
			memset(slot->io_buffer, 0, buffer_size);
		else
			memcpy(slot->io_buffer, buffer, buffer_size);

		*size = buffer_size;
		r = write_ep_locked(port, slot, i, size, slot->io_buffer);

		if (ep_in != 0)
			memcpy(buffer, slot->io_buffer, *size);

		break;
	}

	spin_unlock(&port->lock);

	return r;
}

static int u_configure_ep(struct dancy_usb_device *dev_locked,
	const struct usb_endpoint_descriptor *endpoint)
{
	struct xhci *xhci = dev_locked->hci->hci;
	struct xhci_port *port = NULL;
	struct xhci_slot *slot = NULL;
	int r = 0;

	if (dev_locked->port >= 0 && dev_locked->port <= xhci->max_ports)
		port = &xhci->ports[dev_locked->port - 1];

	if (port == NULL)
		return DE_UNEXPECTED;

	spin_lock_yield(&port->lock);

	if (port->slot_id > 0)
		slot = &xhci->slots[port->slot_id - 1];

	while (slot != NULL && slot->state > 1) {
		uint32_t *m = slot->io_buffer;
		uint32_t in[4], out[4];

		int ep_number = (int)(endpoint->bEndpointAddress & 0x0F);
		int ep_in = ((endpoint->bEndpointAddress & 0x80) != 0);
		int i = (ep_number << 1) + ep_in;

		if (ep_number == 0) {
			r = DE_UNEXPECTED;
			break;
		}

		/*
		 * Allocate the transfer ring.
		 */
		if ((m = slot->endpoints[i].tr) == NULL) {
			if ((m = xhci_mm_page(NULL)) == NULL) {
				r = DE_MEMORY;
				break;
			}
			slot->endpoints[i].tr = m;
		}

		memset(m, 0, 0x1000);
		slot->endpoints[i].cycle = 1;
		slot->endpoints[i].enqueue = 0;

		m = slot->io_buffer;
		memset(m, 0, 0x1000);

		/*
		 * Initialize the input context.
		 */
		{
			int csz = (xhci->hcc_params[0] & (1u << 2)) ? 1 : 0;
			int ici = i + 1;

			/*
			 * Initialize the input control context.
			 */
			{
				uint32_t *c = &m[(0 * (8 << csz))];

				/*
				 * Set the "add context" flags.
				 */
				c[1] |= (1u << 0) | (1u << i);
			}

			/*
			 * Initialize the slot context.
			 */
			{
				uint32_t *c = &m[(1 * (8 << csz))];

				/*
				 * Set the context entries.
				 */
				if (slot->context_entries < i) {
					c[0] |= ((uint32_t)i << 27);
					slot->context_entries = i;
				}
			}

			/*
			 * Initialize the endpoint context.
			 */
			{
				uint32_t *c = &m[(ici * (8 << csz))];

				uint32_t error_count = 3;
				uint32_t ep_type, max_packet_size;
				uint32_t average_trb_length, max_esit_payload;
				void *tr;

				ep_type = endpoint->bmAttributes & 0x03;
				max_packet_size = endpoint->wMaxPacketSize;

				if (ep_type == 0) {
					r = DE_UNSUPPORTED;
					break;
				}

				if (ep_in)
					ep_type += 4;

				average_trb_length = endpoint->wMaxPacketSize;
				max_esit_payload = endpoint->wMaxPacketSize;

				/*
				 * Set the error count.
				 */
				c[1] |= (error_count << 1);

				/*
				 * Set the endpoint type.
				 */
				c[1] |= (ep_type << 3);

				/*
				 * Set the maximum packet size.
				 */
				c[1] |= (max_packet_size << 16);

				/*
				 * Set the transfer ring dequeue pointer.
				 */
				tr = slot->endpoints[i].tr;
				c[2] |= ((uint32_t)((addr_t)tr) | 1u);

				/*
				 * Set the average TRB length.
				 */
				c[4] |= (average_trb_length << 0);

				/*
				 * Set the maximum ESIT payload (low).
				 */
				c[4] |= (max_esit_payload << 16);
			}
		}

		/*
		 * Write the configure endpoint command.
		 */
		in[0] = (uint32_t)((addr_t)m);
		in[1] = 0;
		in[2] = 0;
		in[3] = (uint32_t)((port->slot_id << 24) | (12 << 10));

		printk("[xHCI] Configure Endpoint Command, Context %d, "
			"Port ID %d, Slot ID %d\n",
			ep_number, port->port_id, port->slot_id);

		if (write_command(xhci, &in[0], &out[0])) {
			r = DE_UNEXPECTED;
			break;
		}

		/*
		 * Check the completion code (Success).
		 */
		if (((out[2] >> 24) & 0xFF) != 1) {
			r = DE_UNEXPECTED;
			break;
		}

		printk("[xHCI] Configure Endpoint OK, Context %d, "
			"Port ID %d, Slot ID %d\n",
			ep_number, port->port_id, port->slot_id);

		break;
	}

	spin_unlock(&port->lock);

	return r;
}

static int xhci_port_task(void *arg)
{
	struct xhci_port *port = arg;
	struct xhci *xhci = port->xhci;

	int port_id = port->port_id;
	char cline[16];

	if (snprintf(&cline[0], sizeof(cline), "[xhci-port%d]", port_id) <= 0)
		return EXIT_FAILURE;

	task_set_cmdline(task_current(), NULL, &cline[0]);

	spin_lock_yield(&port->lock);

	if (cpu_btr32(&port->_task, 0) == 0)
		return spin_unlock(&port->lock), 0;

	if (port->dev != NULL)
		usb_remove_device(port->dev);

	if (port->slot_id != 0) {
		uint32_t in[4], out[4];

		xhci->slots[port->slot_id - 1].max_packet_size = 0;
		xhci->slots[port->slot_id - 1].context_entries = 0;
		xhci->slots[port->slot_id - 1].state = 0;

		/*
		 * Write the disable slot command.
		 */
		in[0] = 0;
		in[1] = 0;
		in[2] = 0;
		in[3] = (uint32_t)((port->slot_id << 24) | (10 << 10));

		printk("[xHCI] Disable Slot Command, "
			"Port ID %d, Slot ID %d\n",
			port->port_id, port->slot_id);

		if (write_command(xhci, &in[0], &out[0]))
			return spin_unlock(&port->lock), EXIT_FAILURE;

		/*
		 * Check the completion code (Success).
		 */
		if (((out[2] >> 24) & 0xFF) != 1)
			return spin_unlock(&port->lock), EXIT_FAILURE;

		/*
		 * Clear the device context address.
		 */
		xhci->buffer_dcbaap[port->slot_id] = (uint64_t)(0);

		/*
		 * Optimize memory page usage, but it is not an error
		 * if xhci_mm_page() returns a non-null pointer.
		 */
		{
			struct xhci_slot *s = &xhci->slots[port->slot_id - 1];
			size_t i, count;
			void *page;

			if ((page = s->device_context) != NULL)
				s->device_context = xhci_mm_page(page);

			if ((page = s->io_buffer) != NULL)
				s->io_buffer = xhci_mm_page(page);

			count = sizeof(s->endpoints) / sizeof(*s->endpoints);

			for (i = 0; i < count; i++) {
				if ((page = s->endpoints[i].tr) == NULL)
					continue;
				s->endpoints[i].tr = xhci_mm_page(page);
			}
		}

		port->slot_id = 0;

		printk("[xHCI] Disable Slot OK, Port ID %d\n", port->port_id);
	}

	if ((cpu_read32(port->portsc) & 3) != 3)
		return spin_unlock(&port->lock), 0;

	while (port->slot_id == 0) {
		uint32_t in[4], out[4];
		int slot_id;

		/*
		 * Write the enable slot command.
		 */
		in[0] = 0;
		in[1] = 0;
		in[2] = 0;
		in[3] = (uint32_t)((port->slot_type << 16) | (9 << 10));

		printk("[xHCI] Enable Slot Command, "
			"Port ID %d, Slot Type %d\n",
			port->port_id, port->slot_type);

		if (write_command(xhci, &in[0], &out[0]))
			break;

		/*
		 * Check the completion code (Success).
		 */
		if (((out[2] >> 24) & 0xFF) != 1)
			break;

		/*
		 * Get the slot ID.
		 */
		slot_id = (int)((out[3] >> 24) & 0xFF);

		if (slot_id < 1 || slot_id > xhci->max_slots)
			break;

		port->slot_id = slot_id;

		printk("[xHCI] Port ID %d, Slot ID %d\n",
			port->port_id, port->slot_id);

		break;
	}

	while (port->slot_id != 0) {
		uint32_t in[4], out[4];
		int slot_id = port->slot_id;
		uint32_t *m;

		xhci->slots[slot_id - 1].max_packet_size = 0;
		xhci->slots[slot_id - 1].context_entries = 0;
		xhci->slots[slot_id - 1].state = 0;

		/*
		 * Allocate the device context.
		 */
		if ((m = xhci->slots[slot_id - 1].device_context) == NULL) {
			if ((m = xhci_mm_page(NULL)) == NULL)
				break;
			xhci->slots[slot_id - 1].device_context = m;
		}

		memset(m, 0, 0x1000);

		/*
		 * Set the device context.
		 */
		{
			xhci->buffer_dcbaap[slot_id] = (uint64_t)((addr_t)m);
		}

		/*
		 * Allocate the transfer ring for endpoint 0.
		 */
		if ((m = xhci->slots[slot_id - 1].endpoints[0].tr) == NULL) {
			if ((m = xhci_mm_page(NULL)) == NULL)
				break;
			xhci->slots[slot_id - 1].endpoints[0].tr = m;
		}

		memset(m, 0, 0x1000);

		xhci->slots[slot_id - 1].endpoints[0].cycle = 1;
		xhci->slots[slot_id - 1].endpoints[0].enqueue = 0;

		/*
		 * Allocate the I/O buffer.
		 */
		if ((m = xhci->slots[slot_id - 1].io_buffer) == NULL) {
			if ((m = xhci_mm_page(NULL)) == NULL)
				break;
			xhci->slots[slot_id - 1].io_buffer = m;
		}

		memset(m, 0, 0x1000);

		/*
		 * Initialize the input context.
		 */
		{
			int csz = (xhci->hcc_params[0] & (1u << 2)) ? 1 : 0;

			/*
			 * Initialize the input control context.
			 */
			{
				uint32_t *c = &m[(0 * (8 << csz))];

				/*
				 * Set "add context flags" A0 and A1.
				 */
				c[1] |= (1u << 0) | (1u << 1);
			}

			/*
			 * Initialize the slot context.
			 */
			{
				uint32_t *c = &m[(1 * (8 << csz))];

				uint32_t portsc = cpu_read32(port->portsc);
				uint32_t speed = (portsc >> 10) & 0x0F;

				/*
				 * Set the route string.
				 */
				c[0] |= (0u << 0);

				/*
				 * Set the speed (deprecated).
				 */
				c[0] |= (speed << 20);

				/*
				 * Set the context entries.
				 */
				c[0] |= (1u << 27);

				/*
				 * Set the root hub port number.
				 */
				c[1] |= ((uint32_t)port->port_id << 16);

				/*
				 * Set the interrupter target.
				 */
				c[2] |= (0u << 22);
			}

			/*
			 * Initialize the endpoint context 0.
			 */
			{
				uint32_t *c = &m[(2 * (8 << csz))];

				uint32_t error_count = 3;
				uint32_t ep_type = 4;
				uint32_t max_packet_size = 8;
				void *tr;

				/*
				 * Set the error count.
				 */
				c[1] |= (error_count << 1);

				/*
				 * Set the endpoint type.
				 */
				c[1] |= (ep_type << 3);

				/*
				 * Set the maximum packet size.
				 */
				c[1] |= (max_packet_size << 16);

				/*
				 * Set the transfer ring dequeue pointer.
				 */
				tr = xhci->slots[slot_id - 1].endpoints[0].tr;
				c[2] |= ((uint32_t)((addr_t)tr) | 1u);
			}
		}

		/*
		 * Write the address device command (BSR 0).
		 */
		in[0] = (uint32_t)((addr_t)m);
		in[1] = 0;
		in[2] = 0;

		in[3] = (uint32_t)(port->slot_id << 24);
		in[3] |= (uint32_t)((11 << 10) | (0 << 9));

		printk("[xHCI] Address Device Command, BSR 0, "
			"Port ID %d, Slot ID %d\n",
			port->port_id, port->slot_id);

		if (write_command(xhci, &in[0], &out[0]))
			break;

		/*
		 * Check the completion code (Success).
		 */
		xhci->slots[slot_id - 1].state = ((out[2] >> 24) & 0xFF) == 1;

		printk("[xHCI] Address Device %s, Port ID %d, Slot ID %d\n",
			xhci->slots[slot_id - 1].state != 0 ? "OK" : "Error",
			port->port_id, port->slot_id);

		break;
	}

	while (port->slot_id && xhci->slots[port->slot_id - 1].state != 0) {
		struct xhci_slot *slot = &xhci->slots[port->slot_id - 1];
		struct usb_device_request request;

		uint32_t in[4], out[4];
		uint32_t *m, max_packet_size;

		memset(&request, 0, sizeof(request));

		/*
		 * Initialize the GET_DESCRIPTOR request structure.
		 */
		request.bmRequestType = 0x80;
		request.bRequest      = 6;
		request.wValue        = 0x0100;
		request.wIndex        = 0;
		request.wLength       = 8;

		memset(slot->io_buffer, 0, (size_t)request.wLength);

		printk("[xHCI] Request GET_DESCRIPTOR, Length 8, "
			"Port ID %d, Slot ID %d\n",
			port->port_id, port->slot_id);

		if (write_request_locked(port, &request, slot->io_buffer))
			break;

		printk("[xHCI] Completed GET_DESCRIPTOR, Length 8, "
			"Port ID %d, Slot ID %d\n",
			port->port_id, port->slot_id);

		m = slot->io_buffer;
		max_packet_size = (cpu_read32(&m[1]) >> 24) & 0xFF;

		if (port->rev_major > 2)
			max_packet_size = (1u << max_packet_size);

		if (max_packet_size < 8)
			max_packet_size = 8;

		memset(m, 0, 0x1000);

		/*
		 * Initialize the input context (evaluate).
		 */
		{
			int csz = (xhci->hcc_params[0] & (1u << 2)) ? 1 : 0;

			/*
			 * Initialize the input control context (evaluate).
			 */
			{
				uint32_t *c = &m[(0 * (8 << csz))];

				/*
				 * Set "add context flags" A0 and A1.
				 */
				c[1] |= (1u << 0) | (1u << 1);
			}

			/*
			 * Initialize the endpoint context 0 (evaluate).
			 */
			{
				uint32_t *c = &m[(2 * (8 << csz))];
				uint32_t ep_type = 4;

				/*
				 * Set the endpoint type.
				 */
				c[1] |= (ep_type << 3);

				/*
				 * Set the maximum packet size.
				 */
				c[1] |= (max_packet_size << 16);
			}
		}

		/*
		 * Write the evaluate context command.
		 */
		in[0] = (uint32_t)((addr_t)m);
		in[1] = 0;
		in[2] = 0;

		in[3] = (uint32_t)(port->slot_id << 24);
		in[3] |= (uint32_t)((13 << 10) | (0 << 9));

		printk("[xHCI] Evaluate Context Command, "
			"Port ID %d, Slot ID %d\n",
			port->port_id, port->slot_id);

		if (write_command(xhci, &in[0], &out[0]))
			break;

		/*
		 * Check the completion code (Success).
		 */
		if (((out[2] >> 24) & 0xFF) == 1)
			slot->state = 2;

		printk("[xHCI] Evaluate Context %s, Port ID %d, Slot ID %d\n",
			slot->state == 2 ? "OK" : "Error",
			port->port_id, port->slot_id);

		/*
		 * Check the endpoint context 0.
		 */
		{
			int csz = (xhci->hcc_params[0] & (1u << 2)) ? 1 : 0;
			uint32_t *c = &m[(2 * (8 << csz))];

			if (max_packet_size != ((c[1] >> 16) & 0xFFFF)) {
				printk("[xHCI] Max Packet Size Mismatch\n");
				slot->state = 0;
			}

			max_packet_size = ((c[1] >> 16) & 0xFFFF);
		}

		slot->max_packet_size = (int)max_packet_size;

		printk("[xHCI] Max Packet Size %u, %s %u.%u, "
			"Port ID %d, Slot ID %d\n",
			(unsigned int)max_packet_size, &port->name[0],
			(unsigned int)port->rev_major,
			(unsigned int)port->rev_minor,
			port->port_id, port->slot_id);

		/*
		 * Create the device structure and node.
		 */
		{
			struct dancy_usb_device *dev = port->dev;

			if (dev == NULL) {
				if ((dev = malloc(sizeof(*dev))) == NULL) {
					printk("[xHCI] Out Of Memory!\n");
					break;
				}

				memset(dev, 0, sizeof(*dev));
				dev->hci = xhci->hci;
				dev->port = port->port_id;
				dev->u_write_request = u_write_request;
				dev->u_write_endpoint = u_write_ep;
				dev->u_configure_endpoint = u_configure_ep;
			}

			if (usb_attach_device((port->dev = dev)))
				break;

			printk("[xHCI] Device Node Created, "
				"Port ID %d, Slot ID %d\n",
				port->port_id, port->slot_id);
		}

		break;
	}

	spin_unlock(&port->lock);

	return 0;
}

static void event_ring_handler(struct xhci *xhci, uint32_t *trb)
{
	int type = (int)((trb[3] >> 10) & 0x3F);
	uint32_t val;

	/*
	 * The transfer event.
	 */
	if (type == 32) {
		int slot_id = (int)((trb[3] >> 24) & 0xFF);

		if (slot_id > 0 && slot_id < xhci->max_slots) {
			uint32_t *p = &xhci->slots[slot_id - 1].trb[0];

			cpu_write32(&p[0], trb[0]);
			cpu_write32(&p[1], trb[1]);
			cpu_write32(&p[2], trb[2]);
			cpu_write32(&p[3], trb[3]);
		}

		return;
	}

	/*
	 * The command completion event.
	 */
	if (type == 33) {
		addr_t a = (addr_t)xhci->buffer_crcr;

		printk("[xHCI] Event Ring, Command Completion, "
			"Address %08X\n", (unsigned int)trb[0]);

		if (trb[0] == 0 || (trb[0] & 3) != 0 || trb[1] != 0)
			return;

		if (trb[0] < a || trb[0] >= (addr_t)xhci->buffer_crcr_high)
			return;

		{
			int offset = (int)((addr_t)trb[0] - a) / 4;
			uint32_t *p = &xhci->buffer_crcr_high[offset];

			if (p[0] != 0 || p[1] != 0)
				return;

			if (p[2] != 0 || p[3] != 0)
				return;

			cpu_write32(&p[0], trb[0]);
			cpu_write32(&p[1], trb[1]);
			cpu_write32(&p[2], trb[2]);
			cpu_write32(&p[3], trb[3]);
		}

		return;
	}

	/*
	 * The port status change event.
	 */
	if (type == 34) {
		int port_id = (int)((trb[0] >> 24) & 0xFF);

		if (port_id >= 1 && port_id <= xhci->max_ports) {
			struct xhci_port *port = &xhci->ports[port_id - 1];
			const uint32_t portsc_mask = 0x4F01FFE1;
			int create_port_task = 0;

			val = cpu_read32(port->portsc);

			printk("[xHCI] Event Ring, Port %d Status Change%s\n",
				port_id, (val & (1u << 21)) ? ", Reset" : "");

			/*
			 * Clear CSC, PEC, WRC, OCC, PRC, PLC, and CEC bits.
			 */
			{
				uint32_t clear_bits = val;

				clear_bits &= portsc_mask;
				clear_bits |= 0x00FE0000;

				if (port->rev_major < 3)
					clear_bits &= 0xFF77FFFF;

				cpu_write32(port->portsc, clear_bits);
			}

			/*
			 * Reset the port if CCS == 1 and CSC == 1.
			 */
			if (((val & 1) != 0) && (val & (1u << 17)) != 0) {
				uint32_t port_reset = val;

				port_reset &= portsc_mask;
				port_reset |= (1u << 4);

				cpu_write32(port->portsc, port_reset);
			}

			/*
			 * Create a task if CCS == 0 and CSC == 1.
			 */
			if (((val & 1) == 0) && (val & (1u << 17)) != 0)
				create_port_task = 1;

			/*
			 * Create a task if CCS == 1 and PED == 1.
			 */
			if ((val & 3) == 3)
				create_port_task = 1;

			/*
			 * Set the sync bit before starting the port task.
			 */
			if (create_port_task) {
				const int t = task_detached;

				cpu_bts32(&port->_task, 0);
				task_create(xhci_port_task, port, t);
			}
		}

		/*
		 * Clear the port change detect bit (PCD).
		 */
		cpu_write32(xhci->usb_sts, (1u << 4));

		return;
	}

	printk("[xHCI] Event Ring, Unknown Type %2d\n", type);
}

static int xhci_irq_task(void *arg)
{
	struct xhci *xhci = arg;

	task_set_cmdline(task_current(), NULL, "[xhci]");

	while (xhci) {
		int dequeue_advanced = 0;

		addr_t a = (addr_t)(xhci->base + xhci->rts_off + 0x20);
		uint32_t val, *i0 = (uint32_t *)a;

		/*
		 * Wait for the interrupt request.
		 */
		event_wait(xhci->irq_event, 2000);

		/*
		 * Check the event interrupt (EINT).
		 */
		if ((cpu_read32(((struct xhci *)arg)->usb_sts) & 8) == 0)
			continue;

		/*
		 * Process the event ring.
		 */
		for (;;) {
			int cycle = xhci->buffer_er_cycle;
			uint32_t *trb;

			trb = &xhci->buffer_er[xhci->buffer_er_dequeue * 4];

			if ((int)(trb[3] & 1) != cycle)
				break;

			event_ring_handler(xhci, trb);

			if ((xhci->buffer_er_dequeue += 1) >= 64) {
				xhci->buffer_er_cycle = !cycle;
				xhci->buffer_er_dequeue = 0;
			}

			dequeue_advanced = 1;
		}

		/*
		 * Clear the event interrupt (EINT) by writing to it (RW1C).
		 */
		cpu_write32(xhci->usb_sts, 8);

		/*
		 * Clear the interrupt pending (IP) by writing to it (RW1C).
		 */
		val = cpu_read32(i0 + 0);
		cpu_write32(i0 + 0, val | 3);

		/*
		 * Clear the event handler busy (EHB) by writing to it (RW1C).
		 */
		if (dequeue_advanced) {
			val = (uint32_t)((addr_t)xhci->buffer_er);
			val += (uint32_t)(xhci->buffer_er_dequeue * 16);
			val |= 8;

			cpu_write32(i0 + 6, val);
			cpu_write32(i0 + 7, 0);
		}
	}

	return 0;
}

static void xhci_irq_func(int irq, void *arg)
{
	struct xhci *xhci = arg;

	cpu_add32(&xhci->irq_count, 1);
	event_signal(xhci->irq_event);

	(void)irq;
}

static phys_addr_t get_base(struct pci_id *pci, int offset)
{
	phys_addr_t base = (phys_addr_t)pci_read(pci, offset), base_high = 0;
	int io_space = (int)(base & 1);
	int type = (int)((base >> 1) & 3);

	if (io_space)
		return (phys_addr_t)(1);

	base &= 0xFFFFFFF0u;

	if (type == 2) {
		base_high = (phys_addr_t)pci_read(pci, offset + 4);
		base |= ((base_high << 16) << 16);
#ifdef DANCY_32
		if (base_high)
			base = 0;
#endif
	}

	return base;
}

static size_t get_size(struct pci_id *pci, int offset)
{
	uint32_t val, saved;

	saved = pci_read(pci, offset);

	pci_write(pci, offset, 0xFFFFFFFFu);
	val = pci_read(pci, offset);

	pci_write(pci, offset, saved);

	return (size_t)((~(val & 0xFFFFFFF0u)) + 1u);
}

static void check_supported_protocols(struct xhci *xhci)
{
	const uint32_t supported_protocol = 2;
	uint8_t *p = xhci->xecp;
	int i, j;

	while (xhci->xecp) {
		uint32_t val = cpu_read32(p);
		uint32_t add = ((val >> 8) & 0xFF) << 2;
		uint32_t name;

		if ((val & 0xFF) == supported_protocol) {
			uint8_t rev_major, rev_minor;
			uint8_t port_count, port_offset;
			uint8_t psi_count, slot_type;

			rev_major = (uint8_t)((val >> 24) & 0xFF);
			rev_minor = (uint8_t)((val >> 16) & 0xFF);

			name = cpu_read32(p + 0x04);
			val = cpu_read32(p + 0x08);

			port_count  = (uint8_t)((val >> 8) & 0xFF);
			port_offset = (uint8_t)((val >> 0) & 0xFF);

			psi_count = (uint8_t)((val >> 28) & 0x0F);

			val = cpu_read32(p + 0x0C);
			slot_type = (uint8_t)((val >> 0) & 0x1F);

			for (i = 0; i < (int)port_count; i++) {
				struct xhci_port *port;

				j = ((int)port_offset + i) - 1;

				if (j < 0 || j >= xhci->max_ports)
					continue;

				port = &xhci->ports[j];
				port->rev_major = rev_major;
				port->rev_minor = rev_minor;

				port->name[0] = (char)((name >> 0x00) & 0xFF);
				port->name[1] = (char)((name >> 0x08) & 0xFF);
				port->name[2] = (char)((name >> 0x10) & 0xFF);
				port->name[3] = (char)((name >> 0x18) & 0xFF);
				port->name[4] = '\0';

				for (j = 3; j >= 0; j--) {
					if (port->name[j] == ' ')
						port->name[j] = '\0';
				}

				port->psi = NULL;
				port->psi_count = (int)psi_count;
				port->slot_type = (int)slot_type;

				if (port->psi_count > 0) {
					addr_t psi = (addr_t)(p + 0x10);
					port->psi = (void *)psi;
				}
			}
		}

		if (add == 0)
			break;
		p += add;
	}
}

static int xhci_init(struct xhci *xhci)
{
	uint8_t *base = xhci->base;
	uint32_t val;
	int i;

	if (!(xhci->irq_event = event_create(0)))
		return DE_MEMORY;

	val = cpu_read32(base + 0x00);
	xhci->cap_length = (val & 0xFF);
	xhci->hci_version = ((val >> 16) & 0xFFFF);

	xhci->hcs_params[0] = cpu_read32(base + 0x04);
	xhci->hcs_params[1] = cpu_read32(base + 0x08);
	xhci->hcs_params[2] = cpu_read32(base + 0x0C);

	xhci->hcc_params[0] = cpu_read32(base + 0x10);
	xhci->hcc_params[1] = cpu_read32(base + 0x1C);

	xhci->db_off  = cpu_read32(base + 0x14) & 0xFFFFFFFC;
	xhci->rts_off = cpu_read32(base + 0x18) & 0xFFFFFFE0;

	xhci->base_cap = (base + xhci->cap_length);
	xhci->xecp = base + (((xhci->hcc_params[0] >> 16) & 0xFFFF) << 2);

	if (xhci->xecp == base)
		return DE_UNSUPPORTED;

	xhci->page_size = cpu_read32(xhci->base_cap + 0x08);

	xhci->max_slots = (int)((xhci->hcs_params[0] >>  0) & 0x00FF);
	xhci->max_intrs = (int)((xhci->hcs_params[0] >>  8) & 0x07FF);
	xhci->max_ports = (int)((xhci->hcs_params[0] >> 24) & 0x00FF);

	if (xhci->max_slots == 0)
		return DE_UNSUPPORTED;

	xhci->slots = calloc((size_t)xhci->max_slots, sizeof(*xhci->slots));
	if (xhci->slots == NULL)
		return DE_MEMORY;

	if (xhci->max_ports == 0)
		return DE_UNSUPPORTED;

	xhci->ports = calloc((size_t)xhci->max_ports, sizeof(*xhci->ports));
	if (xhci->ports == NULL)
		return DE_MEMORY;

	check_supported_protocols(xhci);

	for (i = 0; i < xhci->max_ports; i++) {
		addr_t a = (addr_t)(xhci->base_cap + 0x400 + (i * 16));
		xhci->ports[i].portsc = (uint32_t *)a;
		xhci->ports[i].xhci = xhci;
		xhci->ports[i].port_id = i + 1;
	}

	xhci->usb_cmd = (uint32_t *)((void *)(xhci->base_cap + 0x0000));
	xhci->usb_sts = (uint32_t *)((void *)(xhci->base_cap + 0x0004));
	xhci->dn_ctrl = (uint32_t *)((void *)(xhci->base_cap + 0x0014));
	xhci->crcr    = (uint32_t *)((void *)(xhci->base_cap + 0x0018));
	xhci->dcbaap  = (uint32_t *)((void *)(xhci->base_cap + 0x0030));
	xhci->config  = (uint32_t *)((void *)(xhci->base_cap + 0x0038));

	/*
	 * The boot procesures are responsible of resetting the controller
	 * and taking the ownership from the firmware. HCHalted must be 1.
	 */
	if ((cpu_read32(xhci->usb_sts) & 1) == 0) {
		kernel->print("\033[91m[WARNING]\033[m "
			"xHCI HCHalted is 0\n");
		return DE_UNSUPPORTED;
	}

	/*
	 * Check the page size register (PAGESIZE).
	 */
	if ((xhci->page_size & 0xFFFF) != 1) {
		kernel->print("\033[91m[WARNING]\033[m "
			"xHCI Page Size is not 4 KiB\n");
		return DE_UNSUPPORTED;
	}

	/*
	 * The command ring control register (CRCR).
	 */
	{
		xhci->buffer_crcr = xhci_alloc(0x1000);

		if (xhci->buffer_crcr == NULL)
			return DE_MEMORY;

		memset(xhci->buffer_crcr, 0, 0x1000);

		xhci->buffer_crcr_high = xhci->buffer_crcr + 512;
		xhci->buffer_crcr_cycle = 1;

		val = (1u << 0);
		val |= (cpu_read32(xhci->crcr + 0) & 0x30);
		val |= (uint32_t)((addr_t)xhci->buffer_crcr);

		cpu_write32(xhci->crcr + 0, val);
		cpu_write32(xhci->crcr + 1, 0);
	}

	/*
	 * The device context base address array pointer register (DCBAAP).
	 */
	{
		xhci->buffer_dcbaap = xhci_alloc(0x1000);

		if (xhci->buffer_dcbaap == NULL)
			return DE_MEMORY;

		memset(xhci->buffer_dcbaap, 0, 0x1000);

		val = (uint32_t)((addr_t)xhci->buffer_dcbaap);

		cpu_write32(xhci->dcbaap + 0, val);
		cpu_write32(xhci->dcbaap + 1, 0);
	}

	/*
	 * The configure register (CONFIG).
	 */
	{
		uint32_t slots_enabled = (uint32_t)xhci->max_slots;

		val = cpu_read32(xhci->config) & 0xFFFFFF00;
		cpu_write32(xhci->config, val | slots_enabled);
	}

	/*
	 * The scratchpad buffers.
	 */
	{
		uint32_t n_lo = (xhci->hcs_params[1] >> 27) & 0x1F;
		uint32_t n_hi = (xhci->hcs_params[1] >> 21) & 0x1F;

		xhci->max_scratchpads = (int)((n_hi << 5) | n_lo);

		if (xhci->max_scratchpads) {
			size_t size = 0x1000;
			phys_addr_t addr;

			while (size < (size_t)(xhci->max_scratchpads * 8))
				size <<= 1;

			xhci->buffer_scratch = xhci_alloc(size);

			if (xhci->buffer_scratch == NULL)
				return DE_MEMORY;

			memset(xhci->buffer_scratch, 0, size);

			for (i = 0; i < xhci->max_scratchpads; i++) {
				addr = (phys_addr_t)xhci_alloc(0x1000);

				if (addr == 0)
					return DE_MEMORY;

				memset((void *)addr, 0, 0x1000);
				xhci->buffer_scratch[i] = (uint64_t)addr;
			}

			addr = (addr_t)xhci->buffer_scratch;
			xhci->buffer_dcbaap[0] = (uint64_t)addr;
		}
	}

	/*
	 * The event ring.
	 */
	{
		xhci->buffer_er = xhci_alloc(0x1000);

		if (xhci->buffer_er == NULL)
			return DE_MEMORY;

		memset(xhci->buffer_er, 0, 0x1000);
		xhci->buffer_er_cycle = 1;
	}

	/*
	 * The event ring segment table.
	 */
	{
		xhci->buffer_erst = xhci_alloc(0x1000);

		if (xhci->buffer_erst == NULL)
			return DE_MEMORY;

		memset(xhci->buffer_erst, 0, 0x1000);

		/*
		 * Number of TRBs supported by the event ring is 64.
		 */
		xhci->buffer_erst[0] = (uint64_t)((addr_t)xhci->buffer_er);
		xhci->buffer_erst[1] = 64;
	}

	/*
	 * The primary interrupter.
	 */
	{
		addr_t a = (addr_t)(xhci->base + xhci->rts_off + 0x20);
		uint32_t *i0 = (uint32_t *)a;

		/*
		 * Set interrupter moderation register.
		 */
		cpu_write32(i0 + 1, 4000);

		/*
		 * Set event ring segment table size, which is 1.
		 */
		val = (cpu_read32(i0 + 2) & 0xFFFF0000) | 1;
		cpu_write32(i0 + 2, val);

		/*
		 * Set event ring segment table base address (ERSTBA).
		 */
		cpu_write32(i0 + 4, (uint32_t)((addr_t)xhci->buffer_erst));
		cpu_write32(i0 + 5, 0);

		/*
		 * Set event ring dequeue pointer (ERDP).
		 */
		cpu_write32(i0 + 6, (uint32_t)((addr_t)xhci->buffer_er | 8));
		cpu_write32(i0 + 7, 0);

		/*
		 * Set the interrupter management register (IMAN).
		 */
		val = (cpu_read32(i0 + 0) & 0xFFFFFFFC) | 3;
		cpu_write32(i0 + 0, val);
	}

	/*
	 * Install the IRQ handler.
	 */
	if (pci_install_handler(xhci->pci, xhci, xhci_irq_func) == NULL) {
		kernel->print("\033[91m[ERROR]\033[m xHCI IRQ Handling\n");
		return DE_UNSUPPORTED;
	}

	/*
	 * Set the INTE and R/S bits (USBCMD).
	 */
	{
		const uint32_t interrupter_enable = 4;
		const uint32_t run_stop = 1;

		val = cpu_read32(xhci->usb_cmd);
		cpu_write32(xhci->usb_cmd, val | interrupter_enable);

		val = cpu_read32(xhci->usb_cmd);
		cpu_write32(xhci->usb_cmd, val | run_stop);

		for (i = 0; /* void */; i++) {
			const uint32_t hc_halted = 1;

			if ((cpu_read32(xhci->usb_sts) & hc_halted) == 0)
				break;

			if (i == 2000) {
				kernel->print("\033[91m[ERROR]\033[m "
					"xHCI Run/Stop Failure\n");
				break;
			}

			cpu_halt(1);
		}
	}

	return 0;
}

static int usb_xhci_init(struct pci_id *pci)
{
	uint32_t cmd = pci_read(pci, 0x04) & 0xFFFF;
	phys_addr_t addr = get_base(pci, 0x10);
	int r = DE_UNSUPPORTED;

	if ((cmd & 2) != 0 && addr > 1 && addr < SIZE_MAX) {
		size_t size = get_size(pci, 0x10);
		void *base = pg_map_kernel(addr, size, pg_uncached);

		if (base != NULL) {
			const int type = task_detached;
			struct dancy_usb_controller *hci;
			struct xhci *xhci;

			if ((hci = malloc(sizeof(*hci))) == NULL)
				return DE_MEMORY;

			if ((xhci = malloc(sizeof(*xhci))) == NULL)
				return DE_MEMORY;

			memset(xhci, 0, sizeof(*xhci));
			xhci->pci = pci;
			xhci->base = base;
			xhci->size = size;
			xhci->hci = hci;

			memset(hci, 0, sizeof(*hci));
			hci->type = DANCY_USB_CONTROLLER_XHCI;
			hci->pci = pci;
			hci->hci = xhci;

			r = usb_register_controller(hci);

			pci_write(pci, 0x04, cmd | 4);

			if (!r)
				r = xhci_init(xhci);

			if (!r && !task_create(xhci_irq_task, xhci, type))
				r = DE_MEMORY;

			if (!r) {
				uint32_t no_op[4] = { 0, 0, 0, (23u << 10) };
				r = write_command(xhci, &no_op[0], NULL);
			}
		}
	}

	return (r != DE_UNSUPPORTED) ? r : 0;
}

PCI_DRIVER(usb_xhci_init, -1, -1, 0x0C, 0x03, 0x30);
