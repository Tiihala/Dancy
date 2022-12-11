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
 * misc/hdd_fat.c
 *      Mount FAT12/16/32 hard disk drive partitions
 */

#include <dancy.h>

static int mount_drive(const char *name, struct vfs_node *dev_node)
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

	if ((r = fat_io_create(&node, dev_node)) != 0) {
		vfs_unlink(name);
		return r;
	}

	if ((r = vfs_mount(name, node)) != 0)
		vfs_unlink(name);

	node->n_release(&node);

	return r;
}

static int inspect_bpb(const unsigned char *buf)
{
	unsigned int val;

	/*
	 * Check the jump instruction.
	 */
	if (buf[0] == 0xEB) {
		val = buf[1];
		if (val < 0x3C || (val >= 0x80 && val != 0xFE))
			return 1;
	} else if (buf[0] == 0xE9) {
		val = LE16(&buf[1]);
		if (val < 0x3C || (val >= 0x1FD && val != 0xFFFD))
			return 1;
	} else {
		return 1;
	}

	/*
	 * Check the boot signature.
	 */
	if (LE16(&buf[510]) != 0xAA55)
		return 1;

	/*
	 * Sector sizes 512, 1024, 2048, and 4096 are supported.
	 */
	val = LE16(&buf[11]);
	if (val < 512 || val > 4096 || (val & (val - 1)) != 0)
		return 1;

	/*
	 * Sectors per cluster.
	 */
	val = buf[13];
	if (val < 1 || val > 128 || (val & (val - 1)) != 0)
		return 1;

	/*
	 * Reserved sectors.
	 */
	val = LE16(&buf[14]);
	if (val == 0)
		return 1;

	/*
	 * Number of FATs.
	 */
	val = buf[16];
	if (val != 1 && val != 2)
		return 1;

	return 0;
}

static int detect_fs(struct vfs_node *dev_node)
{
	unsigned char *buf;
	size_t size;
	int r;

	if ((buf = malloc(4096)) == NULL)
		return DE_MEMORY;

	size = 512;
	r = dev_node->n_read(dev_node, 0, &size, buf);

	if (r == DE_ALIGNMENT) {
		size = 4096;
		r = dev_node->n_read(dev_node, 0, &size, buf);
	}

	if (r != 0 || size < 512)
		return free(buf), DE_READ;

	if (inspect_bpb(buf))
		r = DE_UNSUPPORTED;

	return free(buf), r;
}

static int qsort_func(const void *v1, const void *v2)
{
	const char *n1 = v1;
	const char *n2 = v2;
	size_t l1 = strlen(n1);
	size_t l2 = strlen(n2);
	int p1 = INT_MAX, p2 = INT_MAX;
	int r;

	if (n1[4] == '\0' && n1[3] >= '1' && n1[3] <= '4')
		p1 = (int)n1[3];

	if (n2[4] == '\0' && n2[3] >= '1' && n2[3] <= '4')
		p2 = (int)n2[3];

	if (p1 < p2)
		return -1;
	if (p1 > p2)
		return 1;

	if ((r = strncmp(n1, n2, 3)) != 0)
		return r;

	if (l1 < l2)
		return -1;
	if (l1 > l2)
		return 1;

	return strcmp(n1, n2);
}

static int find_drives(void)
{
	struct { char name[16]; } *partitions;
	struct vfs_node *dev_node;
	struct vfs_dent dent;
	int letter = 'c', count = 0;
	char buf[16];
	int i, r;

	if ((partitions = calloc(128, sizeof(*partitions))) == NULL)
		return DE_MEMORY;

	if ((r = vfs_open("/dev/", &dev_node, 0, 0)) != 0)
		return free(partitions), r;

	for (i = 0; i < INT_MAX && count < 128; i++) {
		int add_partition = 0;

		r = dev_node->n_readdir(dev_node, (uint32_t)i, &dent);

		if (r != 0) {
			dev_node->n_release(&dev_node);
			return free(partitions), r;
		}

		if (dent.name[0] == '\0')
			break;

		if (dent.name[0] == 'h' && dent.name[1] == 'd')
			add_partition = 1;
		if (dent.name[0] == 's' && dent.name[1] == 'd')
			add_partition = 1;

		if (!add_partition)
			continue;

		memcpy(&partitions[count++].name[0], &dent.name[0], 16);
	}

	dev_node->n_release(&dev_node);

	qsort(partitions, (size_t)count, sizeof(*partitions), qsort_func);

	for (i = 0; i < count && letter <= 'z'; i++) {
		char *name = &buf[0];

		snprintf(name, 16, "/dev/%s", &partitions[i].name[0]);

		if ((r = vfs_open(name, &dev_node, 0, 0)) != 0)
			return free(partitions), r;

		if ((r = detect_fs(dev_node)) != 0) {
			dev_node->n_release(&dev_node);
			if (r != DE_UNSUPPORTED)
				return free(partitions), r;
			continue;
		}

		snprintf(name, 16, "/mnt/drive_%c", letter++);

		r = mount_drive(name, dev_node);
		dev_node->n_release(&dev_node);

		if (r != 0)
			return free(partitions), r;
	}

	return free(partitions), 0;
}

int hdd_fat_init(void)
{
	static int run_once;
	int r;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	if ((r = find_drives()) != 0)
		return r;

	return 0;
}
