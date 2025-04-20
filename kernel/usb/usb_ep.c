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
 * usb/usb_ep.c
 *      Configure endpoints
 */

#include <dancy.h>

int usb_configure_endpoints(struct vfs_node *node,
	struct dancy_usb_driver *driver)
{
	struct dancy_usb_node *data = node->internal_data;
	struct dancy_usb_device *dev = data->dev;

	struct usb_configuration_descriptor *configuration;
	struct usb_interface_descriptor *interface;

	int bNumEndpoints = 0;
	int i, r = 0;

	spin_lock_yield(&dev->lock);

	if (data->port != dev->port || data->device != dev->device)
		return spin_unlock(&dev->lock), DE_MEDIA_CHANGED;

	configuration = driver->descriptor.configuration;
	interface = driver->descriptor.interface;

	{
		struct usb_device_request request;

		for (i = 0; r == 0 && i < 32; i++) {
			struct usb_endpoint_descriptor *endpoint;
			endpoint = driver->descriptor.endpoints[i];

			if (endpoint == NULL)
				continue;

			r = dev->u_configure_endpoint(dev, endpoint);
			bNumEndpoints += (r == 0);
		}

		if (interface->bNumEndpoints != bNumEndpoints) {
			printk("[USB] Configure Endpoint Error, "
				"Port %d, Device %d\n",
				dev->port, dev->device);

			return spin_unlock(&dev->lock), DE_UNEXPECTED;
		}

		memset(&request, 0, sizeof(request));

		/*
		 * Write the SET_CONFIGURATION request.
		 */
		request.bmRequestType = 0x00;
		request.bRequest      = 9;
		request.wValue        = configuration->bConfigurationValue;
		request.wIndex        = 0;
		request.wLength       = 0;

		r = dev->u_write_request(dev, &request, NULL);

		if (r == DE_USB_STALL) {
			printk("[USB] Stall Error, Port %d, Device %d\n",
				dev->port, dev->device);
		}

		if (r != 0) {
			printk("[USB] SET_CONFIGURATION Error, "
				"Port %d, Device %d\n",
				dev->port, dev->device);

			return spin_unlock(&dev->lock), DE_UNEXPECTED;
		}
	}

	spin_unlock(&dev->lock);

	return 0;
}
