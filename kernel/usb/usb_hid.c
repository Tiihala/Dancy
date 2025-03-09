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
 * usb/usb_hid.c
 *      Human Interface Devices
 */

#include <dancy.h>

static int read_report_descriptor(struct vfs_node *node,
	struct dancy_usb_driver *driver)
{
	struct dancy_usb_node *data = node->internal_data;
	struct dancy_usb_device *dev = data->dev;
	struct usb_hid_descriptor *hid = driver->descriptor.hid;

	int i, wDescriptorLength = 0;

	if (hid == NULL)
		return DE_UNSUPPORTED;

	for (i = 0; i < (int)hid->bNumDescriptors; i++) {
		const uint8_t *p = (const void *)((addr_t)hid);
		int bDescriptorType;

		if (((3 * i) + 9) > (int)hid->bLength)
			return DE_UNEXPECTED;

		p += ((3 * i) + 6);
		bDescriptorType = (int)p[0];

		if (bDescriptorType == 0x22) {
			wDescriptorLength = (int)p[1] | ((int)p[2] << 8);
			break;
		}
	}

	if (wDescriptorLength == 0)
		return DE_UNSUPPORTED;

	/*
	 * Allocate the data buffer.
	 */
	{
		size_t size = (size_t)wDescriptorLength;

		if (driver->descriptor.hid_report != NULL)
			free(driver->descriptor.hid_report);

		driver->descriptor.hid_report_size = size;
		driver->descriptor.hid_report = malloc(size);

		if (driver->descriptor.hid_report == NULL) {
			driver->descriptor.hid_report_size = 0;
			return DE_MEMORY;
		}

		memset(driver->descriptor.hid_report, 0, size);
	}

	spin_lock_yield(&dev->lock);

	if (data->port != dev->port || data->device != dev->device)
		return spin_unlock(&dev->lock), DE_MEDIA_CHANGED;

	/*
	 * Set the report protocol (only for boot devices).
	 */
	if (driver->descriptor.interface->bInterfaceSubClass == 1) {
		struct usb_device_request request;

		i = (int)driver->descriptor.interface->bInterfaceNumber;
		memset(&request, 0, sizeof(request));

		request.bmRequestType = 0x21;
		request.bRequest      = 0x0B;
		request.wValue        = 0x0001;
		request.wIndex        = (uint16_t)i;
		request.wLength       = 0;

		if (dev->u_write_request(dev, &request, NULL))
			return spin_unlock(&dev->lock), DE_UNSUPPORTED;
	}

	/*
	 * Read the first report descriptor.
	 */
	{
		struct usb_device_request request;
		void *buffer = driver->descriptor.hid_report;

		i = (int)driver->descriptor.interface->bInterfaceNumber;
		memset(&request, 0, sizeof(request));

		request.bmRequestType = 0x81;
		request.bRequest      = 0x06;
		request.wValue        = 0x2200;
		request.wIndex        = (uint16_t)i;
		request.wLength       = (uint16_t)wDescriptorLength;

		if (dev->u_write_request(dev, &request, buffer))
			return spin_unlock(&dev->lock), DE_UNSUPPORTED;
	}

	printk("[USB] HID Report Descriptor, Length %d, Port %d, Device %d\n",
		wDescriptorLength, dev->port, dev->device);

	spin_unlock(&dev->lock);

	return 0;
}

void usb_hid_driver(struct vfs_node *node, struct dancy_usb_driver *driver)
{
	printk("[USB] Driver Started, Human Interface Devices (HID)\n");

	if (read_report_descriptor(node, driver))
		return;

	(void)node, (void)driver, task_sleep(10000);
}
