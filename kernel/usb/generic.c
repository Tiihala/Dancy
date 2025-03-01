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
 * usb/generic.c
 *      Generic USB code for Dancy Operating System
 */

#include <dancy.h>

static int configure_endpoints(struct vfs_node *node)
{
	struct dancy_usb_node *data = node->internal_data;
	struct dancy_usb_device *dev = data->dev;
	struct dancy_usb_driver *driver = data->_driver;

	int i, r = DE_UNEXPECTED, bNumEndpoints = 0;

	for (i = 0; i < 32; i++) {
		if (driver->descriptor.endpoints[i] == NULL)
			continue;
		bNumEndpoints += 1;
	}

	if (bNumEndpoints != driver->descriptor.interface->bNumEndpoints)
		return r;

	spin_lock_yield(&dev->lock);

	if (data->port != dev->port || data->device != dev->device) {
		spin_unlock(&dev->lock);
		return DE_MEDIA_CHANGED;
	}

	if (bNumEndpoints != 0) {
		struct usb_configuration_descriptor *configuration;
		struct usb_device_request request;

		configuration = driver->descriptor.configuration;
		memset(&request, 0, sizeof(request));

		/*
		 * Write the SET_CONFIGURATION request.
		 */
		request.bmRequestType = 0x00;
		request.bRequest      = 9;
		request.wValue        = configuration->bConfigurationValue;
		request.wIndex        = 0;
		request.wLength       = 0;

		r = dev->u_write_request(dev, &request, (void *)((addr_t)16));

		if (r != 0) {
			printk("[USB] SET_CONFIGURATION Error\n");
			spin_unlock(&dev->lock);
			return r;
		}

		for (i = 0; i < 32 && r == 0; i++) {
			struct usb_endpoint_descriptor *endpoint;
			endpoint = driver->descriptor.endpoints[i];

			if (endpoint != NULL)
				r = dev->u_configure_endpoint(dev, endpoint);
		}

		if (r != 0) {
			printk("[USB] Configure Endpoint Error\n");
			spin_unlock(&dev->lock);
			return r;
		}
	}

	spin_unlock(&dev->lock);

	return 0;
}

static void start_driver(struct vfs_node *node,
	struct dancy_usb_driver *driver)
{
	struct dancy_usb_node *data = node->internal_data;
	uint8_t bInterfaceClass, bInterfaceProtocol;

	/*
	 * Create a copy of the driver structure.
	 */
	{
		struct dancy_usb_driver *new_driver;

		if ((new_driver = malloc(sizeof(*new_driver))) == NULL) {
			printk("[USB] Out of Memory\n");
			return;
		}

		memcpy(new_driver, driver, sizeof(*new_driver));

		new_driver->next = data->_driver;
		data->_driver = new_driver;
	}

	bInterfaceClass = driver->descriptor.interface->bInterfaceClass;
	bInterfaceProtocol = driver->descriptor.interface->bInterfaceProtocol;

	/*
	 * Check the keyboard interface.
	 */
	if (bInterfaceClass == 3 && bInterfaceProtocol == 1) {
		if (configure_endpoints(node))
			return;

		printk("[USB] Human Interface Device (Keyboard) Found\n");
	}
}

static void reset_interface(struct dancy_usb_driver *driver)
{
	int i;

	driver->descriptor.interface = NULL;

	for (i = 0; i < 32; i++)
		driver->descriptor.endpoints[i] = NULL;
}

static void prepare_driver(struct vfs_node *node)
{
	struct dancy_usb_node *data = node->internal_data;
	uint8_t *p = data->_driver_buffer;
	struct dancy_usb_driver *driver;
	int offset = 0;

	if ((driver = malloc(sizeof(*driver))) == NULL) {
		printk("[USB] Out of Memory\n");
		return;
	}

	memset(driver, 0, sizeof(*driver));
	data->_driver = driver;

	/*
	 * Read all relevant descriptors and force 16-byte alignments.
	 */
	{
		size_t size = 2048;
		uint8_t *buffer = calloc(1, (size + 32));
		int i = 0, j = 0;

		if (buffer == NULL)
			return;

		node->n_read(node, 0, &size, buffer);

		while (i + 2 < (int)size) {
			int bLength = (int)buffer[i];

			if (bLength < 2 || i + bLength > 2048)
				break;

			j = (j + 0x0F) & 0x1FF0;

			if (j + bLength >= 0x0FF0)
				break;

			memcpy(&p[j], &buffer[i], (size_t)bLength);
			i += bLength, j += bLength;
		}

		free(buffer);
	}

	/*
	 * Check the descriptors and collect enough information for
	 * the interface-specific driver.
	 */
	for (;;) {
		addr_t addr = (addr_t)((void *)(&p[offset + 0]));

		int bLength = (int)p[offset + 0];
		int bDescriptorType = (int)p[offset + 1];

		offset = ((offset + bLength) + 0x0F) & 0x1FF0;

		if (bLength < 2) {
			if (driver->descriptor.interface != NULL)
				start_driver(node, driver);

			reset_interface(driver);
			break;
		}

		/*
		 * The device descriptor.
		 */
		if (bDescriptorType == 1 && bLength >= 18) {
			if (driver->descriptor.device != NULL)
				break;

			driver->descriptor.device = (void *)addr;
			continue;
		}

		/*
		 * The configuration descriptor.
		 */
		if (bDescriptorType == 2 && bLength >= 9) {
			if (driver->descriptor.device == NULL)
				continue;

			if (driver->descriptor.interface != NULL)
				start_driver(node, driver);

			reset_interface(driver);

			/*
			 * Only the first configuration is used.
			 */
			if (driver->descriptor.configuration != NULL)
				break;

			driver->descriptor.configuration = (void *)addr;
			continue;
		}

		/*
		 * The interface descriptor.
		 */
		if (bDescriptorType == 4 && bLength >= 9) {
			struct usb_interface_descriptor *interface;

			if (driver->descriptor.device == NULL)
				continue;

			if (driver->descriptor.configuration == NULL)
				continue;

			if (driver->descriptor.interface != NULL)
				start_driver(node, driver);

			reset_interface(driver);
			interface = (void *)addr;

			if (interface->bAlternateSetting != 0)
				continue;

			if (interface->bNumEndpoints == 0)
				continue;

			driver->descriptor.interface = interface;
			continue;
		}

		/*
		 * The endpoint descriptor.
		 */
		if (bDescriptorType == 5 && bLength >= 7) {
			struct usb_endpoint_descriptor *ep = (void *)addr;

			int ep_number = (int)(ep->bEndpointAddress & 0x0F);
			int ep_in = ((ep->bEndpointAddress & 0x80) != 0);

			if (driver->descriptor.device == NULL)
				continue;

			if (driver->descriptor.configuration == NULL)
				continue;

			if (driver->descriptor.interface == NULL)
				continue;

			if (ep_number != 0) {
				int i = (ep_number << 1) | ep_in;
				driver->descriptor.endpoints[i] = ep;
			}
			continue;
		}
	}
}

static int attach_device_task(void *arg)
{
	struct dancy_usb_device *dev = arg;
	struct vfs_node *node;

	task_set_cmdline(task_current(), NULL, "[usb-attach]");

	spin_lock_yield(&dev->lock);
	spin_lock_yield(&dev->hci->lock);

	if ((node = dev->hci->ports[dev->port]) != NULL)
		vfs_increment_count(node);

	spin_unlock(&dev->hci->lock);
	spin_unlock(&dev->lock);

	if (node != NULL) {
		struct dancy_usb_node *data = node->internal_data;

		data->_driver_buffer = (void *)mm_alloc_pages(mm_kernel, 0);

		if (data->_driver_buffer != NULL) {
			memset(data->_driver_buffer, 0, 0x1000);
			prepare_driver(node);
		}

		node->n_release(&node);
	}

	return 0;
}

int usb_register_controller(struct dancy_usb_controller *hci)
{
	int r;

	if ((r = usbfs_create(hci)) != 0)
		return r;

	return 0;
}

int usb_attach_device(struct dancy_usb_device *dev)
{
	int r;

	if ((r = usbfs_device(dev, 1)) != 0)
		return r;

	if (!task_create(attach_device_task, dev, task_detached))
		return DE_MEMORY;

	return 0;
}

int usb_remove_device(struct dancy_usb_device *dev)
{
	int r;

	if ((r = usbfs_device(dev, 0)) != 0)
		return r;

	return 0;
}
