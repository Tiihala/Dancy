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
 * usb/usb_task.c
 *      The attach device task function
 */

#include <dancy.h>

struct driver_arg {
	struct vfs_node *node;
	struct dancy_usb_driver *driver;
	void (*driver_task)(struct vfs_node *, struct dancy_usb_driver *);
};

static int start_driver_3(void *driver_arg)
{
	struct driver_arg arg;

	task_set_cmdline(task_current(), NULL, "[usb-driver]");

	memcpy(&arg, driver_arg, sizeof(arg));
	free(driver_arg);

	arg.driver_task(arg.node, arg.driver);
	arg.node->n_release(&arg.node);

	return 0;
}

static void start_driver_2(struct vfs_node *n, struct dancy_usb_driver *d,
	void (*driver_task)(struct vfs_node *, struct dancy_usb_driver *))
{
	struct driver_arg *arg;

	if ((arg = malloc(sizeof(*arg))) == NULL) {
		printk("[USB] Out of Memory\n");
		return;
	}

	arg->node = n;
	arg->driver = d;
	arg->driver_task = driver_task;

	vfs_increment_count(arg->node);

	if (!task_create(start_driver_3, arg, task_detached))
		arg->node->n_release(&arg->node), free(arg);
}

static void start_driver_1(struct vfs_node *node,
	struct dancy_usb_driver *driver)
{
	struct dancy_usb_node *data = node->internal_data;
	uint8_t iClass, iSubClass, iProtocol;

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

		driver = new_driver;
	}

	iClass = driver->descriptor.interface->bInterfaceClass;
	iSubClass = driver->descriptor.interface->bInterfaceSubClass;
	iProtocol = driver->descriptor.interface->bInterfaceProtocol;

	{
		int i, count = 0;

		for (i = 0; i < 32; i++)
			count += (driver->descriptor.endpoints[i] != NULL);

		if (count != driver->descriptor.interface->bNumEndpoints) {
			printk("[USB] Erroneous bNumEndpoints\n");
			return;
		}
	}

	printk("[USB] Interface Found, Class %d, SubClass %d, Protocol %d\n",
		(int)iClass, (int)iSubClass, (int)iProtocol);

	/*
	 * The driver for human interface devices.
	 */
	if (iClass == 3) {
		start_driver_2(node, driver, usb_hid_driver);
		return;
	}
}

static void start_driver_0(struct vfs_node *node,
	struct dancy_usb_driver *driver)
{
	struct dancy_usb_node *data = node->internal_data;
	uint8_t *buffer = data->_driver_buffer;

	int i, offset = 0;

	/*
	 * Check the descriptors and collect enough information.
	 */
	for (;;) {
		addr_t addr = (addr_t)((void *)(&buffer[offset + 0]));

		int bLength = (int)buffer[offset + 0];
		int bDescriptorType = (int)buffer[offset + 1];

		offset = ((offset + bLength) + 0x0F) & 0x1FF0;

		if (bLength < 2) {
			if (driver->descriptor.interface != NULL)
				start_driver_1(node, driver);

			memset(driver, 0, sizeof(*driver));
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
				start_driver_1(node, driver);

			driver->descriptor.interface = NULL;
			driver->descriptor.hid = NULL;

			for (i = 0; i < 32; i++)
				driver->descriptor.endpoints[i] = NULL;

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
				start_driver_1(node, driver);

			driver->descriptor.interface = NULL;
			driver->descriptor.hid = NULL;

			for (i = 0; i < 32; i++)
				driver->descriptor.endpoints[i] = NULL;

			interface = (void *)addr;

			if (interface->bAlternateSetting != 0)
				continue;

			if (interface->bNumEndpoints == 0)
				continue;

			driver->descriptor.interface = interface;
			continue;
		}

		/*
		 * The HID descriptor.
		 */
		if (bDescriptorType == 33 && bLength >= 6) {
			if (driver->descriptor.device == NULL)
				continue;

			if (driver->descriptor.configuration == NULL)
				continue;

			if (driver->descriptor.interface == NULL)
				continue;

			driver->descriptor.hid = (void *)addr;
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
				i = (ep_number << 1) | ep_in;
				driver->descriptor.endpoints[i] = ep;
			}
			continue;
		}
	}
}

int usb_attach_device_task(void *arg)
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
		struct dancy_usb_driver *driver;
		uint8_t *p[2];

		spin_lock_yield(&node->lock);

		if (data->_driver_buffer != NULL) {
			printk("[USB] Unexpected Node Access\n");
			spin_unlock(&node->lock);
			node->n_release(&node);
			return EXIT_FAILURE;
		}

		data->_driver_buffer = (void *)mm_alloc_pages(mm_kernel, 0);
		spin_unlock(&node->lock);

		if (data->_driver_buffer == NULL) {
			printk("[USB] Out of Memory\n");
			node->n_release(&node);
			return EXIT_FAILURE;
		}

		p[0] = data->_driver_buffer;
		p[1] = (void *)mm_alloc_pages(mm_kernel, 0);

		if (p[1] == NULL) {
			/*
			 * The n_release function releases _driver_buffer.
			 */
			printk("[USB] Out of Memory\n");
			node->n_release(&node);
			return EXIT_FAILURE;
		}

		memset(p[0], 0, 0x1000);
		memset(p[1], 0, 0x1000);

		/*
		 * Read all relevant descriptors and force 16-byte alignments.
		 */
		{
			size_t size = 0x1000;
			int i = 0, j = 0;

			node->n_read(node, 0, &size, p[1]);

			while (i + 2 < (int)size) {
				int bLength = (int)p[1][i];

				if (bLength < 2)
					break;

				j = (j + 0x0F) & 0x1FF0;

				if (j + bLength >= 0x0FF0)
					break;

				memcpy(p[0] + j, p[1] + i, (size_t)bLength);
				i += bLength, j += bLength;
			}
		}

		mm_free_pages((phys_addr_t)p[1], 0), p[1] = NULL;

		if ((driver = malloc(sizeof(*driver))) == NULL) {
			printk("[USB] Out of Memory\n");
			node->n_release(&node);
			return EXIT_FAILURE;
		}

		memset(driver, 0, sizeof(*driver));

		start_driver_0(node, driver);
		free(driver);

		node->n_release(&node);
	}

	return 0;
}
