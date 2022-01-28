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
 * vfs/path.c
 *      Path normalization
 */

#include <dancy.h>

#define VFS_PATH_COUNT  32
#define VFS_PATH_BUFFER 256

struct vfs_path {
	char *working_directory[VFS_PATH_COUNT];
	char *absolute_path[VFS_PATH_COUNT];
	char buffer[2 * VFS_PATH_BUFFER];
};

static char vfs_path_default[2]   = { '0', '\0' };
static char vfs_path_dot[2]       = { '.', '\0' };
static char vfs_path_directory[2] = { 'D', '\0' };

static struct vfs_path *get_vfs_path(void)
{
	addr_t a = (addr_t)task_current();

	/*
	 * Every task has a vfs_path structure. There is no
	 * need for any kind of locking. If changing the size
	 * of the structure, this offset must be carefully
	 * checked so that there is enough space.
	 */
	return (struct vfs_path *)((void *)(a + 0x0800));
}

static int copy_working_directory(struct vfs_path *path, int *offset)
{
	const size_t size = VFS_PATH_BUFFER;
	int count = 1, new_offset = -1, state = 0;
	int i;

	memcpy(&path->buffer[size], &path->buffer[0], size);

	path->absolute_path[0] = &vfs_path_default[0];
	path->absolute_path[VFS_PATH_COUNT - 2] = NULL;
	path->absolute_path[VFS_PATH_COUNT - 1] = NULL;

	for (i = 1; i < VFS_PATH_COUNT - 2; i++) {
		char *p;

		if ((p = path->working_directory[i]) == NULL)
			state = 1;

		if (state == 0)
			path->absolute_path[i] = p + size, count += 1;
		else
			path->absolute_path[i] = NULL;
	}

	path->absolute_path[count++] = &vfs_path_dot[0];

	for (i = VFS_PATH_BUFFER; i < (2 * VFS_PATH_BUFFER) - 2; i++) {
		if (path->buffer[i] == '\0') {
			if (path->buffer[i + 1] == '\0') {
				if (i > 0)
					new_offset = i + 1;
				else
					new_offset = VFS_PATH_BUFFER;
				break;
			}
		}
	}

	if (new_offset < 0 || state != 1)
		kernel->panic("vfs_path: inconsistent state");

	*offset = new_offset;

	return count;
}

static void path_error(struct vfs_path *path)
{
	const size_t size = VFS_PATH_BUFFER;
	int i;

	memset(&path->buffer[size], 0, size);

	for (i = 1; i < VFS_PATH_COUNT; i++)
		path->absolute_path[i] = NULL;
}

static int set_vname(struct vfs_path *path, struct vfs_name *vname, int val)
{
	if (val)
		path_error(path);

	memset(vname, 0, sizeof(*vname));

	vname->components = &path->absolute_path[1];
	vname->offset = 0;

	if (path->absolute_path[0][0] == 'D')
		vname->type = vfs_type_directory;
	else
		vname->type = vfs_type_unknown;

	return val;
}

int vfs_build_path(const char *name, struct vfs_name *vname)
{
	const size_t size = VFS_PATH_BUFFER;
	struct vfs_path *path = get_vfs_path();
	int count = 1, offset = VFS_PATH_BUFFER, state = 0;
	int i, j;

	if (!name || name[0] != '/') {
		count = copy_working_directory(path, &offset);

		/*
		 * vfs_build_path(NULL) will return the current
		 * working directory.
		 */
		if (!name) {
			path->absolute_path[0] = &vfs_path_directory[0];
			path->absolute_path[count - 1] = NULL;
			return set_vname(path, vname, 0);
		}

	} else {
		memset(&path->buffer[size], 0, size);
		path->absolute_path[0] = &vfs_path_default[0];

		for (i = 1; i < VFS_PATH_COUNT; i++)
			path->absolute_path[i] = NULL;

		/*
		 * vfs_build_path("/") is a special case.
		 */
		if (name[1] == '\0') {
			path->absolute_path[0] = &vfs_path_directory[0];
			return set_vname(path, vname, 0);
		}
	}

	/*
	 * vfs_build_path("") is not valid.
	 */
	if (name[0] == '\0')
		return set_vname(path, vname, DE_PATH);

	for (i = 0; name[i] != '\0'; i++) {
		char c = name[i];
		char *p;

		if (count + 2 >= VFS_PATH_COUNT)
			return set_vname(path, vname, DE_PATH);

		if (offset + 2 >= 2 * VFS_PATH_BUFFER)
			return set_vname(path, vname, DE_PATH);

		if (state == 0) {
			p = &path->buffer[offset];
			path->absolute_path[count++] = p;

			state = 1;
		}

		if (state == 1 && c != '/')
			state = 2;

		if (state == 2) {
			if (c != '/') {
				path->buffer[offset++] = c;

			} else if (name[i + 1] == '\0') {
				path->buffer[offset++] = '\0';
				p = &vfs_path_dot[0];
				path->absolute_path[count++] = p;
				break;

			} else {
				path->buffer[offset++] = '\0';
				state = 0;
			}
		}
	}

	/*
	 * If the name ends with a "." or "..", the final
	 * component must be a directory. The above code
	 * added a "." if the name ended with a "/".
	 */
	if (path->absolute_path[count - 1][0] == '.') {
		int dot_count = 0;

		if (path->absolute_path[count - 1][1] == '\0') {
			dot_count = 1;

		} else if (path->absolute_path[count - 1][1] == '.') {
			if (path->absolute_path[count - 1][2] == '\0')
				dot_count = 2;
		}

		if (dot_count != 0)
			path->absolute_path[0] = &vfs_path_directory[0];
	}

	for (i = 1; i < count; i++) {
		int dot_count = 0;

		if (path->absolute_path[i][0] == '.') {
			if (path->absolute_path[i][1] == '\0') {
				dot_count = 1;

			} else if (path->absolute_path[i][1] == '.') {
				if (path->absolute_path[i][2] == '\0')
					dot_count = 2;
			}
		}

		if (dot_count == 1 || (dot_count == 2 && i == 1)) {
			for (j = i; j < count; j++) {
				char *p = path->absolute_path[j + 1];
				path->absolute_path[j] = p;
			}
			count -= 1;
			i -= 1;

		} else if (dot_count == 2 && i > 1) {
			for (j = i - 1; j < count - 1; j++) {
				char *p = path->absolute_path[j + 2];
				path->absolute_path[j] = p;
			}
			count -= 2;
			i -= 2;
		}
	}

	if (path->absolute_path[1] == NULL)
		path->absolute_path[0] = &vfs_path_directory[0];

	return set_vname(path, vname, 0);
}

int vfs_chdir(const char *name)
{
	const size_t size = VFS_PATH_BUFFER;
	struct vfs_path *path = get_vfs_path();
	int count = 0, offset = 0;
	int i;

	/*
	 * vfs_chdir(NULL) is valid, and will remove
	 * the current working directory.
	 */
	if (name) {
		struct vfs_name vname;

		if (vfs_build_path(name, &vname))
			return DE_ARGUMENT;

		while (path->absolute_path[count] != NULL)
			count += 1;

		if (count > VFS_PATH_COUNT - 4) {
			(void)path_error(path);
			return DE_ARGUMENT;
		}
	}

	memset(&path->buffer[0], 0, size);

	for (i = 0; i < VFS_PATH_COUNT; i++)
		path->working_directory[i] = NULL;

	for (i = 1; i < count; i++) {
		char *p = path->absolute_path[i];

		path->working_directory[i] = &path->buffer[offset];

		for (;;) {
			char c = *p++;

			if ((path->buffer[offset++] = c) == '\0')
				break;
		}
	}

	return 0;
}
