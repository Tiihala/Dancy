/*
 * Copyright (c) 2022, 2023 Antti Tiihala
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
 * syscall/arg.c
 *      User space arguments and environment
 */

#include <dancy.h>

struct arg_header {
	cpu_native_t argc;
	cpu_native_t argv;
	cpu_native_t envp;
	cpu_native_t size;
};

static void *alloc_arg_state(size_t *size)
{
	const int retry_count = 4096;
	void *arg_state = NULL;
	int i;

	*size = (*size + 0x1F) & 0xFFFFFFE0;

	for (i = 0; i < retry_count; i++) {
		if ((arg_state = aligned_alloc(0x20, *size)) != NULL)
			break;
		task_yield();
	}

	if (arg_state)
		memset(arg_state, 0, *size);

	return arg_state;
}

int arg_create(void **arg_state,
	const char *path, const void *argv, const void *envp)
{
	size_t size = sizeof(struct arg_header);
	int argv_count = 0, envp_count = 0;
	addr_t base, *argv_pointer, *envp_pointer;
	struct arg_header *ah;
	int i, r;
	char *p;

	*arg_state = NULL;

	if (argv && (r = pg_check_user_vector(argv, &argv_count)) != 0)
		return r;

	if (envp && (r = pg_check_user_vector(envp, &envp_count)) != 0)
		return r;

	if (argv_count > 0xFFFF || envp_count > 0xFFFF)
		return DE_OVERFLOW;

	size += ((size_t)(argv_count + 2) * sizeof(void *));
	size += ((size_t)(envp_count + 1) * sizeof(void *));

	size += (strlen(path) + 1);

	for (i = 0; i < argv_count; i++) {
		char *s = *(((char **)((addr_t)argv)) + i);
		int count;

		if ((r = pg_check_user_string(s, &count)) != 0)
			return r;

		if (!((int)size < INT_MAX - count))
			return DE_OVERFLOW;

		size += ((size_t)(count + 1));
	}

	for (i = 0; i < envp_count; i++) {
		char *s = *(((char **)((addr_t)envp)) + i);
		int count;

		if ((r = pg_check_user_string(s, &count)) != 0)
			return r;

		if (!((int)size < INT_MAX - count))
			return DE_OVERFLOW;

		size += ((size_t)(count + 1));
	}

	if (size > 0x04000000)
		return DE_OVERFLOW;

	{
		size_t total_size = size;

		if ((*arg_state = alloc_arg_state(&total_size)) == NULL)
			return DE_MEMORY;

		ah = (struct arg_header *)(*arg_state);
		base = (addr_t)(0x80000000 - total_size);

		ah->argc = (cpu_native_t)argv_count;
		ah->size = (cpu_native_t)total_size;
	}

	p = *arg_state;
	p += (sizeof(struct arg_header));
	argv_pointer = (addr_t *)((addr_t)p);
	ah->argv = (cpu_native_t)(base + ((addr_t)p - (addr_t)(*arg_state)));

	p = *arg_state;
	p += (sizeof(struct arg_header));
	p += ((size_t)(argv_count + 2) * sizeof(void *));
	envp_pointer = (addr_t *)((addr_t)p);
	ah->envp = (cpu_native_t)(base + ((addr_t)p - (addr_t)(*arg_state)));

	p = *arg_state;
	p += (sizeof(struct arg_header));
	p += ((size_t)(argv_count + 2) * sizeof(void *));
	p += ((size_t)(envp_count + 1) * sizeof(void *));

	{
		const char *s = path;

		*argv_pointer++ = base + ((addr_t)p - (addr_t)(*arg_state));

		while ((*p++ = *s++) != '\0')
			/* void */;
	}

	for (i = 0; i < argv_count; i++) {
		char *s = *(((char **)((addr_t)argv)) + i);

		*argv_pointer++ = base + ((addr_t)p - (addr_t)(*arg_state));

		while ((*p++ = *s++) != '\0')
			/* void */;
	}

	for (i = 0; i < envp_count; i++) {
		char *s = *(((char **)((addr_t)envp)) + i);

		*envp_pointer++ = base + ((addr_t)p - (addr_t)(*arg_state));

		while ((*p++ = *s++) != '\0')
			/* void */;
	}

	if (((addr_t)p - (addr_t)(*arg_state)) != size)
		kernel->panic("arg_create: unexpected behavior");

	ah->argv += ((cpu_native_t)sizeof(cpu_native_t));

	return 0;
}

int arg_copy(void *arg_state, addr_t *user_sp)
{
	struct arg_header *ah = (struct arg_header *)arg_state;
	addr_t base = (addr_t)(0x80000000 - ah->size);
	size_t size = (size_t)(ah->size);

	*user_sp = base - (addr_t)sizeof(addr_t);

	if ((base & 0x1F) != 0 || base < 0x7C000000 || base >= 0x80000000)
		return DE_UNEXPECTED;

	ah->size = 0;

	{
		addr_t map_base = (addr_t)(base - 0x2000);
		size_t map_size = (size_t)(size + 0x2000);

		if (!pg_map_user(map_base, map_size))
			return DE_MEMORY;
	}

	memcpy((void *)base, arg_state, size);

	return 0;
}

int arg_set_cmdline(struct vfs_node *node, addr_t user_sp, addr_t user_ld)
{
	struct task *current = task_current();
	uint8_t *buffer = &current->cmd._line[0];
	size_t buffer_size = 1;
	size_t size = 0;

	addr_t ah_user_addr = user_sp + (addr_t)sizeof(addr_t);
	struct arg_header *ah = (struct arg_header *)((void *)ah_user_addr);
	int argc = (int)ah->argc;
	char **argv = (char **)ah->argv;

	char path[256];
	int i, r;

	if (user_ld != 0)
		argc -= 1, argv += 1;

	if ((r = vfs_realpath(node, &path[0], sizeof(path))) != 0)
		return r;

	buffer_size += (strlen(&path[0]) + 1);

	if (buffer_size <= 2 || buffer_size >= sizeof(path))
		return DE_UNEXPECTED;

	for (i = 1; i < argc; i++) {
		size_t add = strlen(argv[i]);

		if (add != 0)
			buffer_size += (add + 1);
	}

	if (buffer_size > TASK_CMD_STATIC_SIZE) {
		const int retry_count = 4096;

		for (i = 0; i < retry_count; i++) {
			if ((buffer = malloc(buffer_size)) != NULL)
				break;
			task_yield();
		}

		if (buffer == NULL)
			return DE_MEMORY;

	} else if (buffer[0] != 0) {
		/*
		 * Remove all previous content from the static
		 * task-specific buffer. This is not necessary.
		 */
		memset(buffer, 0, TASK_CMD_STATIC_SIZE);
	}

	for (i = 0; /* void */; i++) {
		if ((buffer[size++] = (uint8_t)path[i]) == 0)
			break;
	}

	for (i = 1; i < argc; i++) {
		const uint8_t *p = (const uint8_t *)argv[i];

		if (p[0] == 0)
			continue;

		while ((buffer[size++] = *p++) != 0)
			/* void */;
	}

	buffer[size++] = 0;

	if (size != buffer_size)
		kernel->panic("arg_set_cmdline: unexpected behavior");

	task_set_cmdline(current, buffer, NULL);

	return 0;
}

void arg_enable_path(void *arg_state)
{
	struct arg_header *ah = (struct arg_header *)arg_state;

	ah->argc += 1;
	ah->argv -= ((cpu_native_t)sizeof(cpu_native_t));
}

void arg_delete(void *arg_state)
{
	free(arg_state);
}
