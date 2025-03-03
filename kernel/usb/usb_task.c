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

		data->_driver_buffer = (void *)mm_alloc_pages(mm_kernel, 0);

		if (data->_driver_buffer == NULL) {
			printk("[USB] Out of Memory\n");
			node->n_release(&node);
			return EXIT_FAILURE;
		}

		node->n_release(&node);
	}

	return 0;
}
