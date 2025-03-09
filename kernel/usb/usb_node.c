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
 * usb/usb_node.c
 *      Devices /dev/dancy-usb-*
 */

#include <dancy.h>

static void n_release(struct vfs_node **node)
{
	struct vfs_node *n = *node;
	struct dancy_usb_node *data = n->internal_data;

	*node = NULL;

	if (vfs_decrement_count(n) > 0)
		return;

	for (;;) {
		struct dancy_usb_driver *driver = data->_driver;

		if (driver == NULL)
			break;

		data->_driver = driver->next;

		if (driver->descriptor.hid_report != NULL)
			free(driver->descriptor.hid_report);

		memset(driver, 0, sizeof(*driver));
		free(driver);
	}

	if (data->_driver_buffer != NULL)
		mm_free_pages((phys_addr_t)data->_driver_buffer, 0);

	n->internal_data = NULL;
	memset(n, 0, sizeof(*n));

	free(n);
}

static int n_open(struct vfs_node *node, const char *name,
	struct vfs_node **new_node, int type, int mode)
{
	struct dancy_usb_node *data = node->internal_data;
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

static int descriptor(struct dancy_usb_device *dev_locked, size_t buffer_size,
	struct usb_device_request *request, size_t *size, uint8_t *buffer)
{
	int r;

	if (*size + request->wLength > buffer_size)
		return DE_BUFFER;

	r = dev_locked->u_write_request(dev_locked, request, buffer);

	if (r != 0)
		return r;

	if (buffer[0] > request->wLength) {
		request->wLength = (uint16_t)buffer[0];

		if (*size + request->wLength > buffer_size)
			return DE_BUFFER;

		r = dev_locked->u_write_request(dev_locked, request, buffer);

		if (r != 0)
			return r;

		if (buffer[0] != request->wLength)
			return DE_UNEXPECTED;
	}

	*size += (size_t)request->wLength;

	return 0;
}

static int n_read(struct vfs_node *node,
	uint64_t offset, size_t *size, void *buffer)
{
	struct dancy_usb_node *data = node->internal_data;
	struct dancy_usb_device *dev = data->dev;
	size_t buffer_size = *size;
	int r = 0;

	*size = 0;

	spin_lock_yield(&dev->lock);

	if (data->port != dev->port || data->device != dev->device) {
		spin_unlock(&dev->lock);
		return DE_MEDIA_CHANGED;
	}

	if (offset == 0) {
		struct usb_device_request request;
		unsigned int i, max_string_index = 0, lang_id = 0;
		uint8_t *b = buffer;

		memset(&request, 0, sizeof(request));

		/*
		 * Get the DEVICE descriptor.
		 */
		request.bmRequestType = 0x80;
		request.bRequest      = 6;
		request.wValue        = 0x0100;
		request.wIndex        = 0;
		request.wLength       = 18;

		r = descriptor(dev, buffer_size, &request, size, b + *size);

		if (r != 0)
			return spin_unlock(&dev->lock), r;

		/*
		 * Check the maximum string index.
		 */
		{
			/*
			 * The iManufacturer string.
			 */
			if (max_string_index < b[14])
				max_string_index = b[14];

			/*
			 * The iProduct string.
			 */
			if (max_string_index < b[15])
				max_string_index = b[15];

			/*
			 * The iSerialNumber string.
			 */
			if (max_string_index < b[16])
				max_string_index = b[16];
		}

		/*
		 * Get all the bNumConfigurations.
		 */
		for (i = 0; i < b[17]; i++) {
			size_t original_size = *size;
			uint8_t *sb = b + *size;

			/*
			 * Get the CONFIGURATION descriptor.
			 */
			request.bmRequestType = 0x80;
			request.bRequest      = 6;
			request.wValue        = (uint16_t)(0x0200 + i);
			request.wIndex        = 0;
			request.wLength       = 9;

			r = descriptor(dev, buffer_size, &request, size, sb);

			if (r != 0)
				return spin_unlock(&dev->lock), r;

			*size = original_size;

			/*
			 * Get the CONFIGURATION descriptor (wTotalLength).
			 */
			request.bmRequestType = 0x80;
			request.bRequest      = 6;
			request.wValue        = (uint16_t)(0x0200 + i);
			request.wIndex        = 0;

			request.wLength       = 0;
			request.wLength      |= (uint16_t)((int)sb[2] << 0);
			request.wLength      |= (uint16_t)((int)sb[3] << 8);

			r = descriptor(dev, buffer_size, &request, size, sb);

			if (r != 0)
				return spin_unlock(&dev->lock), r;
		}

		if (max_string_index != 0) {
			uint8_t *sb = b + *size;

			/*
			 * Get the STRING descriptor (Zero).
			 */
			request.bmRequestType = 0x80;
			request.bRequest      = 6;
			request.wValue        = (uint16_t)(0x0300 + 0);
			request.wIndex        = 0;
			request.wLength       = 2;

			r = descriptor(dev, buffer_size, &request, size, sb);

			if (r == DE_BUFFER)
				return spin_unlock(&dev->lock), r;

			if (r != 0) {
				sb[0] = 0x02, sb[1] = 0x03;

				request.wLength = 2;
				*size += 2;
				r = 0;
			}

			for (i = 2; i + 1 < request.wLength; i += 2) {
				/*
				 * Language: English (United States).
				 */
				if (sb[i + 0] == 0x09 && sb[i + 1] == 0x04) {
					lang_id = 0x0409;
					break;
				}
			}
		}

		for (i = 1; i <= max_string_index; i++) {
			uint8_t *sb = b + *size;
			int question_mark = 0;

			if (*size + 4 > buffer_size)
				return spin_unlock(&dev->lock), DE_BUFFER;

			/*
			 * Use a question mark if the language ID is not set.
			 */
			if (lang_id == 0)
				question_mark = 1;

			/*
			 * Use a question mark if indices are not sequential.
			 */
			if (i != b[14] && i != b[15] && i != b[16])
				question_mark = 1;

			/*
			 * The UTF-16LE encoding.
			 */
			if (question_mark) {
				sb[0] = 0x04, sb[1] = 0x03;
				sb[2] = 0x3F, sb[3] = 0x00;

				*size += 4;
				continue;
			}

			/*
			 * Get the STRING descriptor (Index).
			 */
			request.bmRequestType = 0x80;
			request.bRequest      = 6;
			request.wValue        = (uint16_t)(0x0300 + i);
			request.wIndex        = (uint16_t)lang_id;
			request.wLength       = 2;

			r = descriptor(dev, buffer_size, &request, size, sb);

			if (r != 0)
				return spin_unlock(&dev->lock), r;
		}
	}

	spin_unlock(&dev->lock);

	return r;
}

static int n_readdir(struct vfs_node *node,
	uint32_t offset, struct vfs_dent *dent)
{
	struct dancy_usb_node *data = node->internal_data;
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
		struct dancy_usb_node *n = hci->ports[port]->internal_data;
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

int usb_register_controller(struct dancy_usb_controller *hci)
{
	struct dancy_usb_node *data;
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

static int handle_device_locked(struct dancy_usb_device *dev, int attached)
{
	struct dancy_usb_controller *hci = dev->hci;
	struct dancy_usb_node *data;
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

int usb_attach_device(struct dancy_usb_device *dev)
{
	int r;

	spin_lock_yield(&dev->lock);
	spin_lock_yield(&dev->hci->lock);

	r = handle_device_locked(dev, 1);

	spin_unlock(&dev->hci->lock);
	spin_unlock(&dev->lock);

	if (!r && !task_create(usb_attach_device_task, dev, task_detached))
		r = DE_MEMORY;

	return r;
}

int usb_remove_device(struct dancy_usb_device *dev)
{
	int r;

	spin_lock_yield(&dev->lock);
	spin_lock_yield(&dev->hci->lock);

	r = handle_device_locked(dev, 0);

	spin_unlock(&dev->hci->lock);
	spin_unlock(&dev->lock);

	return r;
}
