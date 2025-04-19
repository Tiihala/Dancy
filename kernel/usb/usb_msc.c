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
 * usb/usb_msc.c
 *      Mass Storage Class
 */

#include <dancy.h>

static int msc_dev_lock;
static struct vfs_node *msc_dev_node;
static struct vfs_node *msc_dev_array[6];

struct bulk_only {
	struct vfs_node *usb_node;
	struct dancy_usb_driver *driver;

	struct dancy_usb_node *data;
	struct dancy_usb_device *dev;

	int lun;
	int max_lun;
	int lock;

	struct usb_endpoint_descriptor *in;
	struct usb_endpoint_descriptor *out;

	uint8_t inquiry_data[36];

	uint64_t disk_size;
	uint32_t disk_block_size;
};

static void clear_feature_locked(struct bulk_only *state,
	struct usb_endpoint_descriptor *ep)
{
	struct dancy_usb_node *data = state->data;
	struct dancy_usb_device *dev = state->dev;

	if (!dev->lock)
		return;

	if (data->port == dev->port && data->device == dev->device) {
		struct usb_device_request request;

		memset(&request, 0, sizeof(request));

		request.bmRequestType = 0x02;
		request.bRequest      = 0x01;
		request.wValue        = 0x0000;
		request.wIndex        = (uint16_t)ep->bEndpointAddress;
		request.wLength       = 0;

		dev->u_write_request(dev, &request, NULL);
	}
}

static int write_cbw(struct bulk_only *state,
	int device_to_host, const void *cbwcb, size_t cbwcb_size,
	void *buffer, size_t buffer_size)
{
	struct dancy_usb_node *data = state->data;
	struct dancy_usb_device *dev = state->dev;

	int r = DE_MEDIA_CHANGED;
	unsigned char cbw[32];
	unsigned char csw[16];

	memset(&cbw[0], 0, sizeof(cbw));
	memset(&csw[0], 0, sizeof(csw));

	if (device_to_host && buffer_size > 0)
		memset(buffer, 0, buffer_size);

	/*
	 * Create the command block wrapper.
	 */
	{
		const unsigned long cbw_tag = 0;

		if (cbwcb_size < 1 || cbwcb_size > 16)
			return DE_ARGUMENT;

		W_LE32(&cbw[0x00], 0x43425355);
		W_LE32(&cbw[0x04], cbw_tag);
		W_LE32(&cbw[0x08], buffer_size);

		cbw[0x0C] = (unsigned char)(device_to_host ? 0x80 : 0x00);
		cbw[0x0D] = (unsigned char)state->lun;
		cbw[0x0E] = (unsigned char)cbwcb_size;

		memcpy(&cbw[0x0F], cbwcb, cbwcb_size);
	}

	spin_lock_yield(&dev->lock);

	if (data->port == dev->port && data->device == dev->device) {
		struct usb_endpoint_descriptor *ep = state->out;
		size_t size = 31;

		r = dev->u_write_endpoint(dev, ep, &size, &cbw[0]);

		if (!r && size != 31)
			r = DE_UNEXPECTED;
	}

	if (!r && device_to_host) {
		struct usb_endpoint_descriptor *ep = state->in;
		size_t size = buffer_size;

		r = dev->u_write_endpoint(dev, ep, &size, buffer);

		if (r != 0) {
			clear_feature_locked(state, ep);
			size = buffer_size;
			r = dev->u_write_endpoint(dev, ep, &size, buffer);
		}
	}

	/*
	 * Read the command status wrapper (CSW).
	 */
	if (!r) {
		struct usb_endpoint_descriptor *ep = state->in;
		size_t size = 13;

		r = dev->u_write_endpoint(dev, ep, &size, &csw[0]);

		if (r != 0) {
			clear_feature_locked(state, ep);
			size = 13;
			r = dev->u_write_endpoint(dev, ep, &size, &csw[0]);
		}

		if (r == 0 && size != 13)
			r = DE_UNEXPECTED;

		if (r == 0 && LE32(&csw[0]) != 0x53425355)
			r = DE_UNEXPECTED;

		if (csw[12] != 0)
			r = DE_READ;
	}

	spin_unlock(&dev->lock);

	return r;
}

static int scsi_inquiry(struct bulk_only *state)
{
	static const uint8_t c[6] = { 0x12, 0, 0, 0, 36, 0 };

	void *buffer = &state->inquiry_data[0];
	size_t buffer_size = sizeof(state->inquiry_data);
	int i, r;

	if ((r = write_cbw(state, 1, &c[0], 6, buffer, buffer_size)) != 0) {
		printk("[USB] Mass Storage, Inquiry Error\n");
		return r;
	}

	for (i = 15; i >= 8; i--) {
		if (state->inquiry_data[i] != 0x20)
			break;
		state->inquiry_data[i] = 0x00;
	}

	for (i = 31; i >= 16; i--) {
		if (state->inquiry_data[i] != 0x20)
			break;
		state->inquiry_data[i] = 0x00;
	}

	return 0;
}

static int scsi_read_capacity(struct bulk_only *state)
{
	static const uint8_t c[10] = { 0x25, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	unsigned char buffer[8];
	size_t buffer_size = sizeof(buffer);
	uint32_t lba, val;
	int r;

	if ((r = write_cbw(state, 1, &c[0], 10, buffer, buffer_size)) != 0) {
		printk("[USB] Mass Storage, Read Capacity Error\n");
		return r;
	}

	lba = (uint32_t)BE32(&buffer[0]);
	val = (uint32_t)BE32(&buffer[4]);

	if (lba == 0xFFFFFFFFu) {
		printk("[USB] Mass Storage, Size Overflow\n");
		return DE_OVERFLOW;
	}

	if (val < 0x200 || val > 0x1000 || (val & (val - 1)) != 0) {
		printk("[USB] Mass Storage, Block Alignment Error\n");
		return DE_ALIGNMENT;
	}

	state->disk_size = 1;
	state->disk_size += (uint64_t)lba;
	state->disk_size *= (uint64_t)val;

	state->disk_block_size = val;

	return 0;
}

static int scsi_read(struct bulk_only *state,
	uint32_t lba, uint32_t count, addr_t buffer, size_t buffer_size)
{
	uint8_t c[10] = { 0x28, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int r;

	c[2] = (uint8_t)((lba >> 24) & 0xFF);
	c[3] = (uint8_t)((lba >> 16) & 0xFF);
	c[4] = (uint8_t)((lba >>  8) & 0xFF);
	c[5] = (uint8_t)((lba >>  0) & 0xFF);

	c[7] = (uint8_t)((count >> 8) & 0xFF);
	c[8] = (uint8_t)((count >> 0) & 0xFF);

	r = write_cbw(state, 1, &c[0], 10, (void *)buffer, buffer_size);

	return r;
}

static int scsi_write(struct bulk_only *state,
	uint32_t lba, uint32_t count, addr_t buffer, size_t buffer_size)
{
	uint8_t c[10] = { 0x2A, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int r;

	c[2] = (uint8_t)((lba >> 24) & 0xFF);
	c[3] = (uint8_t)((lba >> 16) & 0xFF);
	c[4] = (uint8_t)((lba >>  8) & 0xFF);
	c[5] = (uint8_t)((lba >>  0) & 0xFF);

	c[7] = (uint8_t)((count >> 8) & 0xFF);
	c[8] = (uint8_t)((count >> 0) & 0xFF);

	r = write_cbw(state, 0, &c[0], 10, (void *)buffer, buffer_size);

	return r;
}

static void bulk_only_reset(struct bulk_only *state)
{
	struct dancy_usb_node *data = state->data;
	struct dancy_usb_device *dev = state->dev;

	struct usb_device_request request;
	int i = (int)state->driver->descriptor.interface->bInterfaceNumber;

	spin_lock_yield(&dev->lock);

	if (data->port != dev->port || data->device != dev->device) {
		spin_unlock(&dev->lock);
		return;
	}

	memset(&request, 0, sizeof(request));

	request.bmRequestType = 0x21;
	request.bRequest      = 0xFF;
	request.wValue        = 0x0000;
	request.wIndex        = (uint16_t)i;
	request.wLength       = 0;

	i = dev->u_write_request(dev, &request, NULL);

	clear_feature_locked(state, state->in);
	clear_feature_locked(state, state->out);

	spin_unlock(&dev->lock);

	if (i != 0)
		printk("[USB] Mass Storage, Reset Error\n");
}

static void check_media_changed_locked(void)
{
	int i;

	for (i = 0; i < 6; i++) {
		int media_changed = 0;
		struct vfs_node *msc_node;

		if ((msc_node = msc_dev_array[i]) != NULL) {
			struct vfs_node *usb_node = msc_node->internal_data;
			struct dancy_usb_node *data = usb_node->internal_data;
			struct dancy_usb_device *dev = data->dev;

			spin_lock_yield(&dev->lock);

			if (data->port != dev->port)
				media_changed = 1;
			if (data->device != dev->device)
				media_changed = 1;

			spin_unlock(&dev->lock);
		}

		if (media_changed) {
			msc_dev_array[i] = NULL;
			msc_node->n_release(&msc_node);
		}
	}
}

static struct bulk_only *get_state(struct vfs_node *node)
{
	struct vfs_node *n = node->internal_data;
	struct dancy_usb_node *d = n->internal_data;

	return d->_driver->mass_storage_class;
}

static int read_write_locked(struct bulk_only *state,
	uint64_t offset, size_t *size, addr_t buffer, int write_mode)
{
	size_t requested_size = *size;
	size_t transfer_size = 0;

	uint32_t disk_block_size = state->disk_block_size;
	uint64_t sector_count = (state->disk_size / disk_block_size);
	uint64_t lba = (offset / disk_block_size);
	int r = 0;

	*size = 0;

	if (((uint32_t)offset & (disk_block_size - 1)) != 0)
		return DE_ALIGNMENT;

	if (((uint32_t)requested_size & (disk_block_size - 1)) != 0)
		return DE_ALIGNMENT;

	while (transfer_size < requested_size) {
		uint32_t unit_size = 0x1000;
		uint64_t size_diff = requested_size - transfer_size;

		if (unit_size > size_diff)
			unit_size = (unsigned int)size_diff;

		if (lba + (unit_size / disk_block_size) > sector_count) {
			if (lba >= sector_count)
				break;
			unit_size = disk_block_size;
			unit_size *= (uint32_t)(sector_count - lba);
		}

		if (write_mode) {
			r = scsi_write(state, (uint32_t)lba,
				(uint32_t)(unit_size / disk_block_size),
				(addr_t)(buffer + transfer_size),
				(size_t)unit_size);
		} else {
			r = scsi_read(state, (uint32_t)lba,
				(uint32_t)(unit_size / disk_block_size),
				(addr_t)(buffer + transfer_size),
				(size_t)unit_size);
		}

		if (r != 0)
			break;

		lba += (unit_size / disk_block_size);
		transfer_size += unit_size;
	}

	return *size = transfer_size, r;
}

static void n_release(struct vfs_node **node)
{
	struct vfs_node *n = *node;
	struct vfs_node *usb_node = n->internal_data;

	*node = NULL;

	if (vfs_decrement_count(n) > 0)
		return;

	usb_node->n_release(&usb_node);
	n->internal_data = NULL;
	memset(n, 0, sizeof(*n));

	free(n);
}

static int n_open(struct vfs_node *node, const char *name,
	struct vfs_node **new_node, int type, int mode)
{
	int i, r = DE_NAME;

	*new_node = NULL;
	(void)type;

	if (node != msc_dev_node)
		return DE_UNEXPECTED;

	if (name[0] < '0' || name[0] > '6' || name[1] != '\0')
		return DE_NAME;

	if ((mode & vfs_mode_exclusive) != 0)
		return DE_BUSY;

	i = (int)name[0] - '0';

	spin_lock_yield(&msc_dev_lock);
	check_media_changed_locked();

	if (msc_dev_array[i] != NULL) {
		vfs_increment_count(msc_dev_array[i]);
		r = 0, *new_node = msc_dev_array[i];
	}

	spin_unlock(&msc_dev_lock);

	return r;
}

static int n_read(struct vfs_node *node,
	uint64_t offset, size_t *size, void *buffer)
{
	struct bulk_only *state = get_state(node);
	int r;

	spin_lock_yield(&state->lock);
	r = read_write_locked(state, offset, size, (addr_t)buffer, 0);
	spin_unlock(&state->lock);

	return r;
}

static int n_write(struct vfs_node *node,
	uint64_t offset, size_t *size, const void *buffer)
{
	struct bulk_only *state = get_state(node);
	int r;

	spin_lock_yield(&state->lock);
	r = read_write_locked(state, offset, size, (addr_t)buffer, 1);
	spin_unlock(&state->lock);

	return r;
}

static int n_readdir(struct vfs_node *node,
	uint32_t offset, struct vfs_dent *dent)
{
	int r = 0;

	memset(dent, 0, sizeof(*dent));

	if (node != msc_dev_node)
		return DE_UNEXPECTED;

	if (offset == 0) {
		strcpy(&dent->name[0], ".");
		return 0;
	}

	if (offset == 1) {
		strcpy(&dent->name[0], "..");
		return 0;
	}

	if (offset < 6) {
		int i = (int)offset - 2;
		char c = (char)('0' + i);

		spin_lock_yield(&msc_dev_lock);
		check_media_changed_locked();

		if (msc_dev_array[i] != NULL) {
			dent->name[0] = c;
			dent->name[1] = '\0';
		}

		spin_unlock(&msc_dev_lock);
	}

	return r;
}

static int n_stat(struct vfs_node *node, struct vfs_stat *stat)
{
	struct bulk_only *state = get_state(node);

	memset(stat, 0, sizeof(*stat));

	spin_lock_yield(&state->lock);

	stat->size = state->disk_size;
	stat->block_size = (size_t)state->disk_block_size;

	spin_unlock(&state->lock);

	return 0;
}

static int msc_dev_init(void)
{
	int r = 0;

	spin_lock_yield(&msc_dev_lock);

	while (msc_dev_node == NULL) {
		int mode = vfs_mode_create | vfs_mode_exclusive;
		struct vfs_node *node;

		r = vfs_open("/dev/dancy-usb-storage/", &node, 0, mode);

		if (r != 0)
			break;

		node->n_release(&node);

		if ((node = malloc(sizeof(*node))) == NULL) {
			r = DE_MEMORY;
			break;
		}

		vfs_init_node(node, 0);
		node->count = 1;
		node->type = vfs_type_directory;

		node->n_open = n_open;
		node->n_readdir = n_readdir;

		r = vfs_mount("/dev/dancy-usb-storage/", node);

		msc_dev_node = node;
	}

	spin_unlock(&msc_dev_lock);

	return r;
}

static void bulk_only_driver(struct bulk_only *state)
{
	int i;

	if (msc_dev_init())
		return;

	bulk_only_reset(state);

	if (scsi_inquiry(state))
		return;

	if (scsi_read_capacity(state))
		return;

	printk("[USB] Mass Storage, Vendor \"%.8s\", "
		"Product \"%.8s\", %llu MiB\n",
		&state->inquiry_data[8], &state->inquiry_data[16],
		((unsigned long long)state->disk_size / 1024) / 1024);

	spin_lock_yield(&msc_dev_lock);
	check_media_changed_locked();

	for (i = 0; i < 6; i++) {
		struct vfs_node *msc_node;

		if (msc_dev_array[i] != NULL)
			continue;

		if ((msc_node = malloc(sizeof(*msc_node))) == NULL) {
			printk("[USB] Out of Memory\n");
			break;
		}

		vfs_init_node(msc_node, 0);
		msc_node->count = 1;
		msc_node->type = vfs_type_block;

		msc_node->n_release = n_release;
		msc_node->n_read = n_read;
		msc_node->n_write = n_write;
		msc_node->n_stat = n_stat;

		vfs_increment_count(state->usb_node);
		msc_node->internal_data = state->usb_node;

		spin_lock_yield(&state->lock);
		msc_dev_array[i] = msc_node;
		spin_unlock(&state->lock);

		break;
	}

	spin_unlock(&msc_dev_lock);
	usb_mnt_update();
}

void usb_msc_driver(struct vfs_node *node, struct dancy_usb_driver *driver)
{
	struct dancy_usb_node *data = node->internal_data;
	struct dancy_usb_device *dev = data->dev;

	uint8_t iSubClass = driver->descriptor.interface->bInterfaceSubClass;
	uint8_t iProtocol = driver->descriptor.interface->bInterfaceProtocol;

	/*
	 * The SCSI with Bulk-Only (BBB).
	 */
	if (iSubClass == 0x06 && iProtocol == 0x50) {
		struct usb_device_request request;
		int i = (int)driver->descriptor.interface->bInterfaceNumber;
		struct bulk_only *state;
		uint8_t max_lun = 0;

		if (usb_configure_endpoints(node, driver))
			return;

		spin_lock_yield(&dev->lock);

		if (data->port != dev->port || data->device != dev->device) {
			spin_unlock(&dev->lock);
			return;
		}

		memset(&request, 0, sizeof(request));

		/*
		 * Get the Max LUN.
		 */
		request.bmRequestType = 0xA1;
		request.bRequest      = 0xFE;
		request.wValue        = 0x0000;
		request.wIndex        = (uint16_t)i;
		request.wLength       = 1;

		i = dev->u_write_request(dev, &request, &max_lun);
		spin_unlock(&dev->lock);

		if ((state = malloc(sizeof(*state))) == NULL)
			return;

		memset(state, 0, sizeof(*state));

		state->usb_node = node;
		state->driver = driver;
		state->data = data;
		state->dev = dev;
		state->max_lun = (int)max_lun;

		driver->mass_storage_class = state;

		printk("[USB] Mass Storage, Bulk-Only Driver, %sMax LUN %d\n",
			(i == 0) ? "" : "Default ", state->max_lun);

		for (i = 0; i < 32; i++) {
			struct usb_endpoint_descriptor *e;

			if ((e = driver->descriptor.endpoints[i]) == NULL)
				continue;

			if ((e->bmAttributes & 3) != 2)
				continue;

			if ((e->bEndpointAddress & 0x80) != 0) {
				if (state->in == NULL)
					state->in = e;
			} else {
				if (state->out == NULL)
					state->out = e;
			}
		}

		if (state->in != NULL && state->out != NULL)
			bulk_only_driver(state);
	}
}
