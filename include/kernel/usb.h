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
 * kernel/usb.h
 *      Header of Dancy Operating System
 */

#ifndef KERNEL_USB_H
#define KERNEL_USB_H

#include <common/types.h>

struct usb_device_request {
	uint8_t bmRequestType;
	uint8_t bRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
};

struct usb_device_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdUSB;
	uint8_t bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t bDeviceProtocol;
	uint8_t bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	uint8_t bcdDevice;
	uint8_t iManufacturer;
	uint8_t iProduct;
	uint8_t iSerialNumber;
	uint8_t bNumConfigurations;
};

struct usb_configuration_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wTotalLength;
	uint8_t bNumInterfaces;
	uint8_t bConfigurationValue;
	uint8_t iConfiguration;
	uint8_t bmAttributes;
	uint8_t bMaxPower;
};

struct usb_interface_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;
	uint8_t bNumEndpoints;
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubClass;
	uint8_t bInterfaceProtocol;
	uint8_t iInterface;
};

struct usb_hid_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdHID;
	uint8_t bCountryCode;
	uint8_t bNumDescriptors;
};

struct usb_endpoint_descriptor {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bEndpointAddress;
	uint8_t bmAttributes;
	uint16_t wMaxPacketSize;
	uint8_t bInterval;
};

#define DANCY_USB_CONTROLLER_XHCI 1
#define DANCY_USB_CONTROLLER_EHCI 2
#define DANCY_USB_CONTROLLER_OHCI 3
#define DANCY_USB_CONTROLLER_UHCI 4

struct dancy_usb_controller {
	int type;
	struct pci_id *pci;
	void *hci;

	int lock;
	int monotonic;

	struct vfs_node *ports[256];
};

struct dancy_usb_device {
	struct dancy_usb_controller *hci;
	struct dancy_usb_device *owner;

	int lock;
	int port;
	int device;

	int (*u_write_request)(struct dancy_usb_device *dev_locked,
		const struct usb_device_request *request, void *buffer);

	int (*u_write_endpoint)(struct dancy_usb_device *dev_locked,
		const struct usb_endpoint_descriptor *endpoint,
		size_t *size, void *buffer);

	int (*u_configure_endpoint)(struct dancy_usb_device *dev_locked,
		const struct usb_endpoint_descriptor *endpoint);
};

struct dancy_usb_driver {
	struct dancy_usb_driver *next;

	struct {
		struct usb_device_descriptor *device;
		struct usb_configuration_descriptor *configuration;
		struct usb_interface_descriptor *interface;
		struct usb_hid_descriptor *hid;
		struct usb_endpoint_descriptor *endpoints[32];

		size_t hid_report_size;
		uint8_t *hid_report;
	} descriptor;
};

struct dancy_usb_node {
	struct dancy_usb_controller *hci;
	struct dancy_usb_device *dev;

	struct dancy_usb_driver *_driver;
	void *_driver_buffer;

	int port;
	int device;
};

/*
 * Declarations of boot_kbd.c
 */
void usb_boot_keyboard_driver(struct vfs_node *node,
	struct dancy_usb_driver *driver);

/*
 * Declarations of boot_mse.c
 */
void usb_boot_mouse_driver(struct vfs_node *node,
	struct dancy_usb_driver *driver);

/*
 * Declarations of usb_ep.c
 */
int usb_configure_endpoints(struct vfs_node *node,
	struct dancy_usb_driver *driver);

/*
 * Declarations of usb_hid.c
 */
void usb_hid_driver(struct vfs_node *node, struct dancy_usb_driver *driver);

/*
 * Declarations of usb_node.c
 */
int usb_register_controller(struct dancy_usb_controller *hci);
int usb_attach_device(struct dancy_usb_device *dev);
int usb_remove_device(struct dancy_usb_device *dev);

/*
 * Declarations of usb_task.c
 */
int usb_attach_device_task(void *arg);

#endif
