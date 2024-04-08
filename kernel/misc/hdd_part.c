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
 * misc/hdd_part.c
 *      Hard disk drive partitions
 */

#include <dancy.h>

struct hdd_partition {
	uint64_t offset;
	uint64_t size;
	size_t block_size;
	struct vfs_node *dev_node;
	void *pointer;
	int legacy_type;
};

struct hdd_legacy_entry {
	int status;
	int type;
	int chs[2][3];
	uint32_t lba;
	uint32_t sectors;
};

struct hdd_legacy_table {
	struct hdd_legacy_entry array[4];
};

#define HDD_PARTITION_COUNT 32

static int hdd_args(struct hdd_partition *hp, uint64_t *offset, size_t *size)
{
	uint64_t partition_offset = *offset;

	if (partition_offset >= hp->size)
		return *size = 0, 1;

	if (*size > hp->size - partition_offset)
		*size = (size_t)(hp->size - partition_offset);

	*offset = hp->offset + partition_offset;

	return 0;
}

static void n_release(struct vfs_node **node)
{
	struct hdd_partition *hp = (*node)->internal_data;
	struct vfs_node *n = *node;

	*node = NULL;

	if (vfs_decrement_count(n) == 0) {
		hp->dev_node->n_release(&hp->dev_node);
		memset(n, 0, sizeof(*n));
		free(n);
	}
}

static int n_read(struct vfs_node *node,
	uint64_t offset, size_t *size, void *buffer)
{
	struct hdd_partition *hp = node->internal_data;
	uint64_t new_offset = offset;

	if (hdd_args(hp, &new_offset, size))
		return 0;

	return hp->dev_node->n_read(hp->dev_node, new_offset, size, buffer);
}

static int n_write(struct vfs_node *node,
	uint64_t offset, size_t *size, const void *buffer)
{
	struct hdd_partition *hp = node->internal_data;
	uint64_t new_offset = offset;

	if (hdd_args(hp, &new_offset, size))
		return 0;

	return hp->dev_node->n_write(hp->dev_node, new_offset, size, buffer);
}

static int n_sync(struct vfs_node *node)
{
	struct hdd_partition *hp = node->internal_data;

	return hp->dev_node->n_sync(hp->dev_node);
}

static int n_stat(struct vfs_node *node, struct vfs_stat *stat)
{
	struct hdd_partition *hp = node->internal_data;
	int r;

	if ((r = hp->dev_node->n_stat(hp->dev_node, stat)) != 0)
		return r;

	stat->size = hp->size;

	return 0;
}

static struct vfs_node *alloc_node(void)
{
	struct vfs_node *node;
	const size_t F = 0x0F;
	size_t size = sizeof(*node);
	size_t data_offset;

	size = (size + F) & (~F);
	data_offset = size;

	size += sizeof(struct hdd_partition);

	if ((node = malloc(size)) != NULL) {
		addr_t a = (addr_t)node + (addr_t)data_offset;

		vfs_init_node(node, size);

		node->count = 1;
		node->type = vfs_type_block;
		node->internal_data = (void *)a;
		node->n_release = n_release;

		node->n_read  = n_read;
		node->n_write = n_write;
		node->n_sync  = n_sync;
		node->n_stat  = n_stat;
	}

	return node;
}

static int mount_partition(const char *name, struct hdd_partition *hp)
{
	struct vfs_node *node;
	int mode = vfs_mode_create | vfs_mode_exclusive;
	int r;

	if ((r = vfs_open(name, &node, 0, mode)) != 0)
		return r;

	node->n_release(&node);

	if ((node = alloc_node()) == NULL)
		return DE_MEMORY;

	vfs_increment_count(hp->dev_node);
	memcpy(node->internal_data, hp, sizeof(*hp));

	r = vfs_mount(name, node);
	node->n_release(&node);

	return r;
}

static int read_sector(struct hdd_partition *dev_partition, uint64_t lba)
{
	struct vfs_node *node = dev_partition->dev_node;
	uint64_t offset = lba * (uint64_t)dev_partition->block_size;
	size_t size = dev_partition->block_size;
	void *buffer = dev_partition->pointer;
	int r;

	if ((r = node->n_read(node, offset, &size, buffer)) == 0)
		return r;

	if (size != dev_partition->block_size)
		return  DE_BLOCK_READ;

	return 0;
}

static int read_legacy_table(struct hdd_legacy_table *table,
	unsigned char *buffer, int ebr_mode)
{
	int extended_partition_found = 0;
	int i, j;

	memset(table, 0, sizeof(*table));

	if (buffer[510] != 0x55 || buffer[511] != 0xAA)
		return DE_UNSUPPORTED;

	for (i = 0; i < 4; i++) {
		struct hdd_legacy_entry *e = &table->array[i];
		unsigned char *p = &buffer[446 + (i * 16)];

		e->status = (int)p[0];

		if (e->status >= 0x01 && e->status <= 0x7F)
			return DE_UNSUPPORTED;

		e->type = (int)p[4];
		e->lba = (uint32_t)LE32(&p[8]);
		e->sectors = (uint32_t)LE32(&p[12]);

		if (e->type == 0x00) {
			if (e->lba || e->sectors)
				return DE_UNSUPPORTED;
			continue;
		}

		if (i >= 2 && ebr_mode)
			return DE_UNSUPPORTED;

		if (e->type == 0x05 || e->type == 0x0F) {
			if (extended_partition_found++)
				return DE_UNSUPPORTED;
		}

		if (!e->lba || !e->sectors)
			return DE_UNSUPPORTED;

		for (j = 0; j < 2; j++) {
			int a = (j * 4) + 1;
			int b0 = (int)p[a + 0];
			int b1 = (int)p[a + 1];
			int b2 = (int)p[a + 2];

			e->chs[j][0] = b2 | ((b1 & 0xC0) << 2);
			e->chs[j][1] = b0;

			if ((e->chs[j][2] = b1 & 0x3F) == 0)
				return DE_UNSUPPORTED;
		}

		for (j = 0; j < 3; j++) {
			if (e->chs[0][j] < e->chs[1][j])
				break;

			if (e->chs[0][j] > e->chs[1][j])
				return DE_UNSUPPORTED;
		}
	}

	return 0;
}

static int handle_legacy(struct hdd_partition *partitions)
{
	struct hdd_partition *dev_partition = &partitions[0];
	unsigned char *buffer = dev_partition->pointer;
	struct hdd_legacy_entry *ebr = NULL;
	struct hdd_legacy_table table;
	uint64_t ebr_base_lba = 0;
	int ebr_count = 0;
	int i, r;

	if ((r = read_legacy_table(&table, buffer, 0)) != 0)
		return r;

	/*
	 * Primary partitions.
	 */
	for (i = 0; i < 4; i++) {
		struct hdd_legacy_entry *e = &table.array[i];
		uint64_t offset, size;

		if (e->type == 0)
			continue;

		offset = (uint64_t)e->lba * 512;
		size = (uint64_t)e->sectors * 512;

		if (offset + size > dev_partition->size)
			return DE_UNSUPPORTED;

		if (e->type == 0x05 || e->type == 0x0F) {
			ebr_base_lba = (ebr = e)->lba;
			continue;
		}

		partitions[i + 1].offset = offset;
		partitions[i + 1].size = size;
		partitions[i + 1].legacy_type = e->type;
	}

	/*
	 * Extended partitions.
	 */
	while (ebr != NULL) {
		uint64_t lba = (uint64_t)ebr->lba;
		struct hdd_legacy_entry *e;

		if ((r = read_sector(dev_partition, lba)) != 0)
			return r;

		if ((r = read_legacy_table(&table, buffer, 1)) != 0)
			return r;

		e = &table.array[0];
		ebr = &table.array[1];

		if (e->type != 0x00) {
			uint64_t offset, size;

			offset = (lba + (uint64_t)e->lba) * 512;
			size = (uint64_t)e->sectors * 512;

			if (offset + size > dev_partition->size)
				return DE_UNSUPPORTED;

			if ((++ebr_count) + 4 >= HDD_PARTITION_COUNT)
				break;

			partitions[ebr_count + 4].offset = offset;
			partitions[ebr_count + 4].size = size;
			partitions[ebr_count + 4].legacy_type = e->type;
		}

		if (ebr->type != 0x05 && ebr->type != 0x0F)
			break;

		if (ebr_base_lba + (uint64_t)ebr->lba > 0xFFFFFFFF)
			return DE_UNSUPPORTED;

		ebr->lba += (uint32_t)ebr_base_lba;
	}

	return 0;
}

static int handle_guid(struct hdd_partition *partitions)
{
	(void)partitions;

	return DE_UNSUPPORTED;
}

static int handle_device(char *dev_name, struct hdd_partition *partitions)
{
	struct hdd_partition *dev_partition = &partitions[0];
	size_t dev_name_length = strlen(dev_name);
	char nr[4];
	int i, j, r;

	if ((r = read_sector(dev_partition, 1)) != 0)
		return r;

	if (!memcmp(dev_partition->pointer, "EFI PART", 8)) {
		if ((r = handle_guid(partitions)) != 0)
			return r;
	} else {
		if ((r = read_sector(dev_partition, 0)) != 0)
			return r;

		if ((r = handle_legacy(partitions)) != 0)
			return r;
	}

	for (i = 1; i < HDD_PARTITION_COUNT; i++) {
		if (partitions[i].offset == 0)
			continue;

		for (j = 1; j < HDD_PARTITION_COUNT; j++) {
			uint64_t t0 = partitions[j].offset;
			uint64_t t1 = partitions[i].offset;

			if (t0 == t1 && j != i)
				return DE_UNSUPPORTED;

			if (t0 < t1 && t0 + partitions[j].size > t1)
				return DE_UNSUPPORTED;
		}
	}

	for (i = 1; i < HDD_PARTITION_COUNT; i++) {
		if (partitions[i].offset == 0)
			continue;

		snprintf(&nr[0], sizeof(nr), "%d", i);
		strcat(dev_name, &nr[0]);

		partitions[i].block_size = partitions[0].block_size;
		partitions[i].dev_node = partitions[0].dev_node;
		partitions[i].pointer = NULL;

		r = mount_partition(dev_name, &partitions[i]);

		dev_name[dev_name_length] = '\0';

		if (r && r != DE_BUSY)
			return r;
	}

	return 0;
}

static int get_device_name(char *dev_name, int *state)
{
	int i = *state;

	if (i < 4) {
		snprintf(dev_name, 16, "/dev/hd%c", ('a' + (i + 0)));
		*state = i + 1;
	} else if (i < 16) {
		snprintf(dev_name, 16, "/dev/sd%c", ('a' + (i - 4)));
		*state = i + 1;
	} else {
		*state = 0;
		return DE_OVERFLOW;
	}

	return 0;
}

static int find_partitions(void)
{
	size_t size = HDD_PARTITION_COUNT * sizeof(struct hdd_partition);
	struct hdd_partition *partitions;
	struct vfs_stat stat;
	char dev_name[16];
	void *buffer;
	int state = 0;
	int r;

	if ((partitions = malloc(size)) == NULL)
		return DE_MEMORY;

	if ((buffer = malloc(0x1000)) == NULL)
		return free(partitions), DE_MEMORY;

	while (!get_device_name(&dev_name[0], &state)) {
		struct vfs_node *dev_node;

		if ((r = vfs_open(&dev_name[0], &dev_node, 0, 0)) != 0) {
			if (r == DE_NAME)
				continue;
			return free(buffer), free(partitions), r;
		}

		if ((r = dev_node->n_stat(dev_node, &stat)) != 0) {
			dev_node->n_release(&dev_node);
			return free(buffer), free(partitions), r;
		}

		if (stat.size < 0x10000 || stat.block_size < 512) {
			dev_node->n_release(&dev_node);
			continue;
		}

		memset(partitions, 0, size);
		memset(buffer, 0, 0x1000);

		/*
		 * The first one is the special "device partition".
		 */
		partitions[0].size = stat.size;
		partitions[0].block_size = stat.block_size;
		partitions[0].dev_node = dev_node;
		partitions[0].pointer = buffer;

		r = handle_device(&dev_name[0], partitions);
		dev_node->n_release(&dev_node);

		if (r && r != DE_UNSUPPORTED)
			return free(buffer), free(partitions), r;
	}

	return free(buffer), free(partitions), 0;
}

int hdd_part_init(void)
{
	static int run_once;
	int r;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	if ((r = find_partitions()) != 0)
		return r;

	return 0;
}
