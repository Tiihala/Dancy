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

#define DANCY_USB_CONTROLLER_XHCI 1
#define DANCY_USB_CONTROLLER_EHCI 2
#define DANCY_USB_CONTROLLER_OHCI 3
#define DANCY_USB_CONTROLLER_UHCI 4

struct dancy_usb_controller {
	int type;
	struct pci_id *pci;
	void *hci;
};

/*
 * Declarations of usbfs.c
 */
int usbfs_create(struct dancy_usb_controller *hci);

#endif
