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
 * usb/usb_mnt.c
 *      Mount mass storage file systems
 */

#include <dancy.h>

static int usb_mnt_lock;
struct vfs_node *usb_mnt_nodes[6];

struct usb_mnt_internal_data {
	struct vfs_node *msc_node;
	int lock;
	int media_changed;
};

static int mount_drive(const char *name, struct vfs_node *mnt_node)
{
	struct vfs_node *node;
	int mode = vfs_mode_create;
	int r;

	if ((r = vfs_open("/mnt/", &node, 0, mode)) != 0)
		return r;

	node->n_release(&node);

	if ((r = vfs_open(name, &node, vfs_type_directory, mode)) != 0)
		return r;

	node->n_release(&node);

	if ((r = fat_io_create(&node, mnt_node)) != 0) {
		vfs_unlink(name);
		return r;
	}

	if ((r = vfs_mount(name, node)) != 0)
		vfs_unlink(name);

	node->n_release(&node);

	return r;
}

static int n_read(struct vfs_node *node,
	uint64_t offset, size_t *size, void *buffer)
{
	struct usb_mnt_internal_data *data = node->internal_data;

	spin_lock_yield(&data->lock);

	if (data->msc_node != NULL) {
		struct vfs_node *msc_node = data->msc_node;
		int r = msc_node->n_read(msc_node, offset, size, buffer);

		if (r == DE_MEDIA_CHANGED)
			data->media_changed = 1;

		return spin_unlock(&data->lock), r;
	}

	return *size = 0, spin_unlock(&data->lock), 0;
}

static int n_write(struct vfs_node *node,
	uint64_t offset, size_t *size, const void *buffer)
{
	struct usb_mnt_internal_data *data = node->internal_data;

	spin_lock_yield(&data->lock);

	if (data->msc_node != NULL) {
		struct vfs_node *msc_node = data->msc_node;
		int r = msc_node->n_write(msc_node, offset, size, buffer);

		if (r == DE_MEDIA_CHANGED)
			data->media_changed = 1;

		return spin_unlock(&data->lock), r;
	}

	return *size = 0, spin_unlock(&data->lock), 0;
}

static int n_sync(struct vfs_node *node)
{
	struct usb_mnt_internal_data *data = node->internal_data;

	spin_lock_yield(&data->lock);

	if (data->msc_node != NULL) {
		struct vfs_node *msc_node = data->msc_node;
		int r = msc_node->n_sync(msc_node);

		if (r == DE_MEDIA_CHANGED)
			data->media_changed = 1;

		return spin_unlock(&data->lock), r;
	}

	return spin_unlock(&data->lock), 0;
}

static int n_stat(struct vfs_node *node, struct vfs_stat *stat)
{
	struct usb_mnt_internal_data *data = node->internal_data;

	spin_lock_yield(&data->lock);

	if (data->msc_node != NULL) {
		struct vfs_node *msc_node = data->msc_node;
		int r = msc_node->n_stat(msc_node, stat);

		if (r == DE_MEDIA_CHANGED)
			data->media_changed = 1;

		return spin_unlock(&data->lock), r;
	}

	return spin_unlock(&data->lock), 0;
}

static struct vfs_node *alloc_node(void)
{
	struct vfs_node *node;
	const size_t F = 0x0F;
	size_t size = sizeof(*node);
	size_t data_offset;

	size = (size + F) & (~F);
	data_offset = size;

	size += sizeof(struct usb_mnt_internal_data);

	if ((node = malloc(size)) != NULL) {
		addr_t a = (addr_t)node + (addr_t)data_offset;

		vfs_init_node(node, size);

		node->count = 1;
		node->type = vfs_type_block;
		node->internal_data = (void *)a;

		node->n_read = n_read;
		node->n_write = n_write;
		node->n_sync = n_sync;
		node->n_stat = n_stat;
	}

	return node;
}

static void sync_fat_io(void)
{
	struct vfs_node *node;
	int i;

	for (i = 0; i < 6; i++) {
		char name[] = "/mnt/?";
		name[5] = (char)('u' + i);

		if (vfs_open(&name[0], &node, vfs_type_directory, 0))
			continue;

		node->n_sync(node);
		node->n_release(&node);
	}
}

void usb_mnt_update(void)
{
	struct usb_mnt_internal_data *data;
	struct vfs_node *msc_node;
	int i;

	spin_lock_yield(&usb_mnt_lock);

	sync_fat_io();

	for (i = 0; i < 6; i++) {
		if (usb_mnt_nodes[i] == NULL)
			continue;

		data = usb_mnt_nodes[i]->internal_data;
		msc_node = data->msc_node;

		spin_lock_yield(&data->lock);

		if (data->media_changed && msc_node != NULL) {
			msc_node->n_release(&msc_node);
			data->msc_node = NULL;
			data->media_changed = 0;
		}

		spin_unlock(&data->lock);
	}

	for (i = 0; i < 6; i++) {
		char name[] = "/dev/dancy-usb-storage/?";

		name[23] = (char)('0' + i);

		if (vfs_open(&name[0], &msc_node, 0, 0))
			continue;

		if (usb_mnt_nodes[i] == NULL)
			usb_mnt_nodes[i] = alloc_node();

		if (usb_mnt_nodes[i] != NULL) {
			data = usb_mnt_nodes[i]->internal_data;

			spin_lock_yield(&data->lock);

			if (data->msc_node == NULL) {
				vfs_increment_count(msc_node);
				data->msc_node = msc_node;
			}

			spin_unlock(&data->lock);

			strcpy(&name[0], "/mnt/?");
			name[5] = (char)('u' + i);

			mount_drive(&name[0], usb_mnt_nodes[i]);
		}

		msc_node->n_release(&msc_node);
	}

	spin_unlock(&usb_mnt_lock);
}
