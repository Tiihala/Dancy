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
 * usb/usbfs.c
 *      Devices /dev/dancy-usb-*
 */

#include <dancy.h>

struct usbfs_node_data {
	struct dancy_usb_controller *hci;
	struct dancy_usb_device *dev;

	int port;
	int device;
};

static void n_release(struct vfs_node **node)
{
	struct vfs_node *n = *node;

	*node = NULL;

	if (vfs_decrement_count(n) > 0)
		return;

	n->internal_data = NULL;
	memset(n, 0, sizeof(*n));

	free(n);
}

static int n_open(struct vfs_node *node, const char *name,
	struct vfs_node **new_node, int type, int mode)
{
	struct usbfs_node_data *data = node->internal_data;
	struct dancy_usb_controller *hci = data->hci;

	int r = DE_NAME, port = 0, device = 0;
	size_t i;

	*new_node = NULL;

	if ((mode & vfs_mode_create) != 0)
		r = DE_FULL;

	if (type == vfs_type_directory)
		return r;

	/*
	 * Example: port-001-device-000001
	 */
	{
		if (strlen(name) != 22)
			return r;

		if (strncmp(&name[0], "port-", 5))
			return r;

		if (strncmp(&name[8], "-device-", 8))
			return r;

		for (i = 0; i < 3; i++) {
			char c = name[5 + i];

			if (c < '0' || c > '9')
				return r;
			port *= 10;
			port += (int)(c - '0');
		}

		for (i = 0; i < 6; i++) {
			char c = name[16 + i];

			if (c < '0' || c > '9')
				return r;
			device *= 10;
			device += (int)(c - '0');
		}
	}

	if (port < 1 || device < 0)
		return r;

	spin_lock_yield(&hci->lock);

	for (i = 0; i < sizeof(hci->ports) / sizeof(*hci->ports); i++) {
		if (hci->ports[i] == NULL)
			continue;

		if ((data = hci->ports[i]->internal_data) == NULL)
			continue;

		if (data->port != port || data->device != device)
			continue;

		vfs_increment_count(hci->ports[i]);
		*new_node = hci->ports[i];

		r = 0;
		break;
	}

	spin_unlock(&hci->lock);

	return r;
}

static int n_read(struct vfs_node *node,
	uint64_t offset, size_t *size, void *buffer)
{
	struct usbfs_node_data *data = node->internal_data;
	struct dancy_usb_device *dev = data->dev;
	size_t buffer_size = *size;
	int r = 0;

	*size = 0;

	if (buffer_size < 0x100)
		return DE_BUFFER;

	if ((offset & 0xFF) != 0 || (buffer_size & 0xFF) != 0)
		return DE_ALIGNMENT;

	memset(buffer, 0, 0x100);

	spin_lock_yield(&dev->lock);

	if (data->port != dev->port || data->device != dev->device) {
		spin_unlock(&dev->lock);
		return DE_MEDIA_CHANGED;
	}

	if (offset == 0) {
		struct usb_device_request request;
		size_t wLength = 18;

		memset(&request, 0, sizeof(request));

		/*
		 * Get the DEVICE descriptor.
		 */
		request.bmRequestType = 0x80;
		request.bRequest      = 6;
		request.wValue        = 0x0100;
		request.wIndex        = 0;
		request.wLength       = (uint16_t)wLength;

		r = dev->u_write_request(dev, &request, &wLength, buffer);
		*size = (size_t)((r == 0) ? 0x100 : 0);

	} else if (offset == 0x100) {
		struct usb_device_request request;
		size_t wLength = 9;

		memset(&request, 0, sizeof(request));

		/*
		 * Get the CONFIGURATION descriptor.
		 */
		request.bmRequestType = 0x80;
		request.bRequest      = 6;
		request.wValue        = 0x0200;
		request.wIndex        = 0;
		request.wLength       = (uint16_t)wLength;

		r = dev->u_write_request(dev, &request, &wLength, buffer);
		*size = (size_t)((r == 0) ? 0x100 : 0);
	}

	spin_unlock(&dev->lock);

	return r;
}

static int n_readdir(struct vfs_node *node,
	uint32_t offset, struct vfs_dent *dent)
{
	struct usbfs_node_data *data = node->internal_data;
	struct dancy_usb_controller *hci = data->hci;
	int port, r = DE_PLACEHOLDER;

	memset(dent, 0, sizeof(*dent));

	if (offset == 0) {
		strcpy(&dent->name[0], ".");
		return 0;
	}

	if (offset == 1) {
		strcpy(&dent->name[0], "..");
		return 0;
	}

	if (offset - 1 >= sizeof(hci->ports) / sizeof(*hci->ports))
		return 0;

	port = (int)(offset - 1);

	spin_lock_yield(&hci->lock);

	if (hci->ports[port] != NULL) {
		struct usbfs_node_data *n = hci->ports[port]->internal_data;
		char buffer[32];

		int t = snprintf(&buffer[0], sizeof(buffer),
			"port-%03d-device-%06d", n->port, n->device);

		if (t == 22) {
			strcpy(&dent->name[0], &buffer[0]);
			r = 0;
		}
	}

	spin_unlock(&hci->lock);

	return r;
}

static int n_remove(struct vfs_node *node,
	const char *name, int dir)
{
	struct vfs_node *n;
	int r;

	if (dir)
		return DE_NAME;

	if ((r = node->n_open(node, name, &n, 0, 0)) != 0)
		return r;

	n->n_release(&n);

	return DE_BUSY;
}

static struct vfs_node *alloc_node(size_t data_size)
{
	struct vfs_node *node;
	const size_t F = 0x0F;
	size_t size = sizeof(*node);
	size_t data_offset;

	size = (size + F) & (~F);
	data_offset = size;

	size += data_size;

	if ((node = malloc(size)) != NULL) {
		addr_t a = (addr_t)node + (addr_t)data_offset;

		vfs_init_node(node, size);
		node->count = 1;
		node->n_release = n_release;
		node->internal_data = (void *)a;
	}

	return node;
}

static int controller_monotonic;
static int controller_monotonic_lock;

int usbfs_create(struct dancy_usb_controller *hci)
{
	struct usbfs_node_data *data;
	struct vfs_node *node;
	char buffer[32];
	int r = 0;

	spin_lock_yield(&controller_monotonic_lock);

	if (controller_monotonic >= 0 && controller_monotonic < 99)
		r = (++controller_monotonic);

	spin_unlock(&controller_monotonic_lock);

	if (r == 0)
		return DE_OVERFLOW;

	r = snprintf(&buffer[0], sizeof(buffer),
		"/dev/dancy-usb-host-%02d/", r);

	if (r != 23)
		return DE_UNEXPECTED;

	{
		int mode = vfs_mode_create | vfs_mode_exclusive;

		if ((r = vfs_open(&buffer[0], &node, 0, mode)) != 0)
			return r;
	}

	node->n_release(&node);

	if ((node = alloc_node(sizeof(*data))) == NULL)
		return DE_MEMORY;

	node->type = vfs_type_directory;
	node->n_open = n_open;
	node->n_readdir = n_readdir;
	node->n_remove = n_remove;

	data = node->internal_data;
	data->hci = hci;

	r = vfs_mount(&buffer[0], node);
	node->n_release(&node);

	return r;
}

static int usbfs_device_locked(struct dancy_usb_device *dev, int attached)
{
	struct dancy_usb_controller *hci = dev->hci;
	struct usbfs_node_data *data;
	struct vfs_node *node;

	if (dev->port <= 0)
		return DE_UNEXPECTED;

	if (dev->port >= (int)(sizeof(hci->ports) / sizeof(*hci->ports)))
		return DE_UNEXPECTED;

	if ((node = hci->ports[dev->port]) != NULL) {
		data = hci->ports[dev->port]->internal_data;
		data->dev->device = 0;
		node->n_release(&node);
		hci->ports[dev->port] = NULL;
	}

	if (attached) {
		if (hci->monotonic < 0 || hci->monotonic >= 999999)
			return DE_OVERFLOW;

		dev->device = (++hci->monotonic);

		if ((node = alloc_node(sizeof(*data))) == NULL)
			return DE_MEMORY;

		node->type = vfs_type_block;
		node->n_read = n_read;

		data = node->internal_data;
		data->hci = hci;
		data->dev = dev;

		data->port = dev->port;
		data->device = dev->device;

		hci->ports[dev->port] = node;
	}

	return 0;
}

int usbfs_device(struct dancy_usb_device *dev, int attached)
{
	int r;

	spin_lock_yield(&dev->lock);
	spin_lock_yield(&dev->hci->lock);

	r = usbfs_device_locked(dev, attached);

	spin_unlock(&dev->hci->lock);
	spin_unlock(&dev->lock);

	return r;
}
