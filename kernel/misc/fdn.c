/*
 * Copyright (c) 2022 Antti Tiihala
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
 * misc/fdn.c
 *      Device nodes for floppy disks
 */

#include <dancy.h>

static int fdn_lock;

struct fdn_internal {
	struct vfs_node *node;
	mtx_t fdn_mtx;
	int dsel;
};

#define FDN_COUNT 2
static struct fdn_internal fdn_internal_data[FDN_COUNT];

static int read_locked(struct fdn_internal *data,
	uint64_t offset, size_t *size, void *buffer)
{
	/*
	 * Caching has not been implemented at this point.
	 */
	return floppy_read(data->dsel, offset, size, buffer);
}

static int write_locked(struct fdn_internal *data,
	uint64_t offset, size_t *size, const void *buffer)
{
	/*
	 * Caching has not been implemented at this point.
	 */
	return floppy_write(data->dsel, offset, size, buffer);
}

static int n_read(struct vfs_node *node,
	uint64_t offset, size_t *size, void *buffer)
{
	struct fdn_internal *data = node->internal_data;
	int r;

	if (mtx_lock(&data->fdn_mtx) != thrd_success)
		return *size = 0, DE_UNEXPECTED;

	r = read_locked(data, offset, size, buffer);
	mtx_unlock(&data->fdn_mtx);

	return r;
}

static int n_write(struct vfs_node *node,
	uint64_t offset, size_t *size, const void *buffer)
{
	struct fdn_internal *data = node->internal_data;
	int r;

	if (mtx_lock(&data->fdn_mtx) != thrd_success)
		return *size = 0, DE_UNEXPECTED;

	r = write_locked(data, offset, size, buffer);
	mtx_unlock(&data->fdn_mtx);

	return r;
}

static int n_stat(struct vfs_node *node, struct vfs_stat *stat)
{
	struct fdn_internal *data = node->internal_data;
	int r;

	if (mtx_lock(&data->fdn_mtx) != thrd_success)
		return memset(stat, 0, sizeof(*stat)), DE_UNEXPECTED;

	r = floppy_stat(data->dsel, stat);
	mtx_unlock(&data->fdn_mtx);

	return r;
}

int fdn_open_node(int dsel, struct vfs_node **new_node)
{
	void *lock_local = &fdn_lock;
	struct fdn_internal *data;
	struct vfs_node *node;
	int r = DE_BUSY;

	*new_node = NULL;

	if (dsel < 0 || dsel >= FDN_COUNT)
		return DE_ARGUMENT;

	if ((node = malloc(sizeof(*node))) == NULL)
		return DE_MEMORY;

	vfs_init_node(node, 0);
	node->count = -1;

	node->n_read  = n_read;
	node->n_write = n_write;
	node->n_stat  = n_stat;

	data = &fdn_internal_data[dsel];
	node->internal_data = data;

	spin_enter(&lock_local);

	if (!data->node) {
		if (mtx_init(&data->fdn_mtx, mtx_plain) != thrd_success) {
			r = DE_UNEXPECTED;
		} else {
			data->node = node;
			data->dsel = dsel;
			r = 0;
		}
	}

	spin_leave(&lock_local);

	if (r)
		free(node);

	return *new_node = data->node, r;
}

int fdn_free_cache(void *arg)
{
	void *lock_local = &fdn_lock;
	int i, r = DE_MEMORY;

	if (arg)
		return DE_ARGUMENT;

	for (i = 0; i < FDN_COUNT; i++) {
		struct fdn_internal *data = &fdn_internal_data[i];

		spin_enter(&lock_local);
		if (!data->node)
			data = NULL;
		spin_leave(&lock_local);

		if (!data)
			continue;

		if (mtx_lock(&data->fdn_mtx) != thrd_success)
			return DE_UNEXPECTED;

		/*
		 * Caching has not been implemented at this point.
		 */

		mtx_unlock(&data->fdn_mtx);
	}

	return r;
}
