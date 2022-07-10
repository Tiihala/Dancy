/*
 * Copyright (c) 2021, 2022 Antti Tiihala
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
 * misc/serial.c
 *      Serial ports
 */

#include <dancy.h>

event_t serial_event[4];

static int serial_lock;
static int serial_ready;

static const int default_baud = 2400;

static int port_com1 = 0x03F8;
static int port_com2 = 0x02F8;
static int port_com3 = 0x03E8;
static int port_com4 = 0x02E8;

#define COM_BUFFER_SIZE (1024)

static struct {
	int port;
	int baud;
	int start;
	int end;
	uint8_t base[COM_BUFFER_SIZE];
} com_buffer[4];

static int set_port_com(int port, int *port_com)
{
	if (port == 1 && port_com1 != 0) {
		if (port_com)
			*port_com = port_com1;
		return 0;
	}

	if (port == 2 && port_com2 != 0) {
		if (port_com)
			*port_com = port_com2;
		return 0;
	}

	if (port == 3 && port_com3 != 0) {
		if (port_com)
			*port_com = port_com3;
		return 0;
	}

	if (port == 4 && port_com4 != 0) {
		if (port_com)
			*port_com = port_com4;
		return 0;
	}

	return 1;
}

static int read_buffer_byte(int i)
{
	int start, end;
	int r = -1;

	start = com_buffer[i].start;
	end = com_buffer[i].end;

	if (start != end) {
		r = (int)com_buffer[i].base[start];
		com_buffer[i].base[start] = 0;

		com_buffer[i].start = (start + 1) % COM_BUFFER_SIZE;
	}

	if (start == end)
		event_reset(serial_event[i]);

	return r;
}

static int write_buffer_byte(int i, int port_com)
{
	int start = com_buffer[i].start;
	int old_end = com_buffer[i].end;
	int new_end = (old_end + 1) % COM_BUFFER_SIZE;
	int r = -1;

	if (start != new_end) {
		com_buffer[i].base[old_end] = cpu_in8((uint16_t)port_com);
		com_buffer[i].end = new_end;
		r = 0;
	}

	event_signal(serial_event[i]);

	return r;
}

static void serial_irq3_func(int irq, void *arg)
{
	void *lock_local = &serial_lock;

	(void)irq;
	(void)arg;

	/*
	 * This function can be called in the interrupt context and
	 * directly also. The latter is the reason why "spin_enter"
	 * is used and not "spin_lock".
	 */
	spin_enter(&lock_local);

	while (port_com2 != 0) {
		int line_status = (int)cpu_in8((uint16_t)(port_com2 + 5));

		if ((line_status & 1) == 0 || line_status == 255)
			break;
		if (write_buffer_byte(1, port_com2))
			break;
	}

	while (port_com4 != 0) {
		int line_status = (int)cpu_in8((uint16_t)(port_com4 + 5));

		if ((line_status & 1) == 0 || line_status == 255)
			break;
		if (write_buffer_byte(3, port_com4))
			break;
	}

	spin_leave(&lock_local);
}

static void serial_irq4_func(int irq, void *arg)
{
	void *lock_local = &serial_lock;

	(void)irq;
	(void)arg;

	/*
	 * This function can be called in the interrupt context and
	 * directly also. The latter is the reason why "spin_enter"
	 * is used and not "spin_lock".
	 */
	spin_enter(&lock_local);

	while (port_com1 != 0) {
		int line_status = (int)cpu_in8((uint16_t)(port_com1 + 5));

		if ((line_status & 1) == 0 || line_status == 255)
			break;
		if (write_buffer_byte(0, port_com1))
			break;
	}

	while (port_com3 != 0) {
		int line_status = (int)cpu_in8((uint16_t)(port_com3 + 5));

		if ((line_status & 1) == 0 || line_status == 255)
			break;
		if (write_buffer_byte(2, port_com3))
			break;
	}

	spin_leave(&lock_local);
}

static int port_check(int port_com)
{
	uint8_t b;

	/*
	 * Disable the interrupt requests.
	 */
	cpu_out8((uint16_t)(port_com + 1), 0x00);

	/*
	 * Check whether Line Control Register can be modified.
	 */
	cpu_out8((uint16_t)(port_com + 3), 0x00), delay(100000);
	b = cpu_in8((uint16_t)(port_com + 3));
	cpu_out8((uint16_t)(port_com + 3), 0x03), delay(100000);

	if (b == cpu_in8((uint16_t)(port_com + 3)))
		return 1;

	return 0;
}

static void port_init(int port_com, int baud)
{
	int d;

	if (port_com == 0 || baud == 0)
		return;

	d = 115200 / baud;

	/*
	 * Disable the interrupt requests.
	 */
	cpu_out8((uint16_t)(port_com + 1), 0x00);

	/*
	 * Set the divisor (low and high bytes).
	 */
	cpu_out8((uint16_t)(port_com + 3), 0x80);
	cpu_out8((uint16_t)(port_com + 0), (uint8_t)((d >> 0) & 0xFF));
	cpu_out8((uint16_t)(port_com + 1), (uint8_t)((d >> 8) & 0xFF));

	/*
	 * Do not use the "FIFO" feature.
	 */
	cpu_out8((uint16_t)(port_com + 2), 0x00);

	/*
	 * Use the "8 bits, one stop bit, no parity" configuration.
	 */
	cpu_out8((uint16_t)(port_com + 3), 0x03);

	/*
	 * Initialize the "Modem Control" register.
	 */
	cpu_out8((uint16_t)(port_com + 4), 0x0B);

	/*
	 * Enable the interrupt requests.
	 */
	cpu_out8((uint16_t)(port_com + 1), 0x01);
}

static void n_release(struct vfs_node **node)
{
	struct vfs_node *n = *node;

	*node = NULL;

	if (vfs_decrement_count(n) == 0) {
		memset(n, 0, sizeof(*n));
		free(n);
	}
}

static int n_read(struct vfs_node *node,
	uint64_t offset, size_t *size, void *buffer)
{
	const int *port = node->internal_data;
	size_t requested_size = *size;

	(void)offset;

	if ((*size = serial_receive(*port, buffer, requested_size)) != 0)
		return 0;

	event_wait(serial_event[*port - 1], 2000);
	*size = serial_receive(*port, buffer, requested_size);

	return 0;
}

static int n_write(struct vfs_node *node,
	uint64_t offset, size_t *size, const void *buffer)
{
	const int *port = node->internal_data;
	size_t requested_size = *size;

	(void)offset;
	*size = serial_send(*port, buffer, requested_size);

	return 0;
}

static int mount_serial_port(int port)
{
	struct vfs_node *dev_node, *node;
	char name[16];
	int r;

	if ((dev_node = malloc(sizeof(*dev_node))) == NULL)
		return DE_MEMORY;

	vfs_init_node(dev_node, 0);

	dev_node->count = 1;
	dev_node->type  = vfs_type_character;

	dev_node->internal_data = &com_buffer[port - 1].port;
	dev_node->n_release = n_release;

	dev_node->n_read  = n_read;
	dev_node->n_write = n_write;

	snprintf(&name[0], sizeof(name), "/dev/ttyS%d", port - 1);

	if ((r = vfs_open(name, &node, 0, vfs_mode_create)) != 0) {
		dev_node->n_release(&dev_node);
		return r;
	}

	node->n_release(&node);

	r = vfs_mount(name, dev_node);
	dev_node->n_release(&dev_node);

	return r;
}

int serial_init(void)
{
	static int run_once;
	int r;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	com_buffer[0].port = 1;
	com_buffer[1].port = 2;
	com_buffer[2].port = 3;
	com_buffer[3].port = 4;

	if (port_check(port_com1))
		port_com1 = 0;
	if (port_check(port_com2))
		port_com2 = 0;
	if (port_check(port_com3))
		port_com3 = 0;
	if (port_check(port_com4))
		port_com4 = 0;

	/*
	 * Install IRQ handlers for the serial ports.
	 */
	{
		const int irq3 = 3;
		const int irq4 = 4;

		if (!irq_install(irq3, NULL, serial_irq3_func))
			return DE_UNEXPECTED;

		if (!irq_install(irq4, NULL, serial_irq4_func))
			return DE_UNEXPECTED;

		irq_enable(irq3);
		irq_enable(irq4);
	}

	serial_event[0] = event_create(event_type_manual_reset);
	serial_event[1] = event_create(event_type_manual_reset);
	serial_event[2] = event_create(event_type_manual_reset);
	serial_event[3] = event_create(event_type_manual_reset);

	if (!serial_event[0] || !serial_event[1])
		return DE_MEMORY;

	if (!serial_event[2] || !serial_event[3])
		return DE_MEMORY;

	com_buffer[0].baud = default_baud;
	com_buffer[1].baud = default_baud;
	com_buffer[2].baud = default_baud;
	com_buffer[3].baud = default_baud;

	port_init(port_com1, default_baud);
	port_init(port_com2, default_baud);
	port_init(port_com3, default_baud);
	port_init(port_com4, default_baud);

	serial_irq3_func(3, NULL);
	serial_irq4_func(4, NULL);

	for (;;) {
		int b0 = read_buffer_byte(0);
		int b1 = read_buffer_byte(1);
		int b2 = read_buffer_byte(2);
		int b3 = read_buffer_byte(3);

		if (b0 < 0 && b1 < 0 && b2 < 0 && b3 < 0)
			break;
	}

	cpu_write32((uint32_t *)&serial_ready, 1);

	if (port_com1 != 0 && (r = mount_serial_port(1)) != 0)
		return r;
	if (port_com2 != 0 && (r = mount_serial_port(2)) != 0)
		return r;
	if (port_com3 != 0 && (r = mount_serial_port(3)) != 0)
		return r;
	if (port_com4 != 0 && (r = mount_serial_port(4)) != 0)
		return r;

	return 0;
}

int serial_get_baud(int port)
{
	void *lock_local = &serial_lock;
	int baud;

	if (!serial_ready)
		return 0;

	if (set_port_com(port, NULL))
		return 0;

	spin_enter(&lock_local);
	baud = com_buffer[port - 1].baud;
	spin_leave(&lock_local);

	return baud;
}

int serial_set_baud(int port, int baud)
{
	static const int baud_table[] = {
		110, 220, 300, 600, 1200, 2400, 4800, 9600,
		19200, 38400, 57600, 115200
	};
	int baud_count = (int)(sizeof(baud_table) / sizeof(baud_table[0]));
	int i, port_com;

	if (!serial_ready)
		return 1;

	if (set_port_com(port, &port_com))
		return 1;

	for (i = 0; i < baud_count; i++) {
		if (baud_table[i] == baud) {
			void *lock_local = &serial_lock;

			spin_enter(&lock_local);
			com_buffer[port - 1].baud = baud;
			port_init(port_com, baud);
			spin_leave(&lock_local);

			serial_irq3_func(3, NULL);
			serial_irq4_func(4, NULL);
			return 0;
		}
	}

	return 1;
}

size_t serial_receive(int port, void *buf, size_t size)
{
	void *lock_local = &serial_lock;
	size_t r = 0;

	if (!serial_ready)
		return r;

	if (set_port_com(port, NULL))
		return r;

	spin_enter(&lock_local);

	while (r < size) {
		int b = read_buffer_byte(port - 1);

		if (b < 0)
			break;

		((uint8_t *)buf)[r++] = (uint8_t)b;
	}

	spin_leave(&lock_local);

	serial_irq3_func(3, NULL);
	serial_irq4_func(4, NULL);

	return r;
}

size_t serial_send(int port, const void *buf, size_t size)
{
	int port_com, delay_counter = 0;
	size_t r = 0;

	if (!serial_ready)
		return r;

	if (set_port_com(port, &port_com))
		return r;

	while (r < size) {
		void *lock_local = &serial_lock;
		int sent_ok = 0;

		spin_enter(&lock_local);

		if ((cpu_in8((uint16_t)(port_com + 5)) & 0x20) != 0) {
			uint8_t b = ((uint8_t *)buf)[r++];

			cpu_out8((uint16_t)port_com, b);

			delay_counter = 0;
			sent_ok = 1;
		}

		spin_leave(&lock_local);

		if (!sent_ok) {
			if (++delay_counter > 10000)
				break;
			delay(10000);
		}
	}

	return r;
}
