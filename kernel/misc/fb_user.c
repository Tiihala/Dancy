/*
 * Copyright (c) 2024 Antti Tiihala
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
 * misc/fb_user.c
 *      Device /dev/dancy-framebuffer
 */

#include <dancy.h>

static struct vfs_node fb_user_node;

static int fb_user_size;
static struct __dancy_winsize fb_user_winsize;

static int n_read(struct vfs_node *node,
	uint64_t offset, size_t *size, void *buffer)
{
	uint32_t *b = buffer;
	const uint32_t *fb;

	size_t transfer_size = *size;
	int i, pixels;

	(void)node;
	*size = 0;

	if ((offset & 3) != 0 || (transfer_size & 3) != 0)
		return DE_ALIGNMENT;

	if (offset >= (uint64_t)fb_user_size || transfer_size == 0)
		return 0;

	{
		const size_t overflow_limit = 0x10000000;

		if (transfer_size > overflow_limit)
			transfer_size = overflow_limit;

		if ((int)transfer_size > fb_user_size - (int)offset)
			transfer_size = (size_t)(fb_user_size - (int)offset);
	}

	fb = (const uint32_t *)(kernel->fb_standard_addr + (addr_t)offset);
	pixels = (int)transfer_size / 4;

	fb_enter();

	for (i = 0; i < pixels; i++)
		*b++ = *fb++;

	fb_leave();

	return (*size = transfer_size), 0;
}

static int n_write(struct vfs_node *node,
	uint64_t offset, size_t *size, const void *buffer)
{
	const uint32_t *b = buffer;
	volatile uint32_t *fb;

	size_t transfer_size = *size;
	int i, pixels;

	(void)node;
	*size = 0;

	if ((offset & 3) != 0 || (transfer_size & 3) != 0)
		return DE_ALIGNMENT;

	if (offset >= (uint64_t)fb_user_size || transfer_size == 0)
		return 0;

	{
		const size_t overflow_limit = 0x10000000;

		if (transfer_size > overflow_limit)
			transfer_size = overflow_limit;

		if ((int)transfer_size > fb_user_size - (int)offset)
			transfer_size = (size_t)(fb_user_size - (int)offset);
	}

	fb = (volatile uint32_t *)(kernel->fb_standard_addr + (addr_t)offset);
	pixels = (int)transfer_size / 4;

	fb_enter();

	if (kernel->keyboard.console_switch_data != 0x102)
		return fb_leave(), DE_READ_ONLY;

	for (i = 0; i < pixels; i++) {
		uint32_t val = *b++;

		/*
		 * Avoid writing to the standard framebuffer,
		 * which is in normal memory, if the values
		 * do not change. Paging tricks are used.
		 *
		 * The fb pointer is volatile.
		 */
		if (*fb != val)
			*fb = val;
		fb += 1;
	}

	fb_leave();

	return (*size = transfer_size), 0;
}

static int n_ioctl(struct vfs_node *node,
	int request, long long arg)
{
	(void)node;

	if (request == __DANCY_IOCTL_TIOCGWINSZ) {
		void *p = (void *)((addr_t)arg);
		memcpy(p, &fb_user_winsize, sizeof(fb_user_winsize));
		return 0;
	}

	return DE_UNSUPPORTED;
}

static int n_stat(struct vfs_node *node, struct vfs_stat *stat)
{
	(void)node;

	memset(stat, 0, sizeof(*stat));
	stat->size = (uint64_t)fb_user_size;

	return 0;
}

int fb_user_init(void)
{
	static int run_once;
	const char *name = "/dev/dancy-framebuffer";
	struct vfs_node *node;
	int r;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	fb_user_size = 4;
	fb_user_size *= (int)kernel->fb_width;
	fb_user_size *= (int)kernel->fb_height;

	if (fb_user_size == 0 || kernel->fb_standard_size == 0)
		return 0;

	if (fb_user_size > (int)kernel->fb_standard_size)
		return DE_UNEXPECTED;

	fb_user_winsize.ws_xpixel = (unsigned short)kernel->fb_width;
	fb_user_winsize.ws_ypixel = (unsigned short)kernel->fb_height;

	if ((r = vfs_open(name, &node, 0, vfs_mode_create)) != 0)
		return r;

	node->n_release(&node);

	vfs_init_node(&fb_user_node, 0);
	fb_user_node.type = vfs_type_block;
	fb_user_node.n_read = n_read;
	fb_user_node.n_write = n_write;
	fb_user_node.n_ioctl = n_ioctl;
	fb_user_node.n_stat = n_stat;

	if ((r = vfs_mount(name, &fb_user_node)) != 0)
		return r;

	return 0;
}
