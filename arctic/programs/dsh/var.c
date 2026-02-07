/*
 * Copyright (c) 2026 Antti Tiihala
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
 * dsh/var.c
 *      The Dancy Shell
 */

#include "main.h"

struct dsh_var {
	char *data[2];
	size_t flags;
};

static struct dsh_var *dsh_var_array;
static size_t dsh_var_end;

extern char **environ;
char **dsh_var_environ;

static void update_environ(void)
{
	size_t i, n = 0;

	for (i = 0; i + 1 < dsh_var_end; i++) {
		if (dsh_var_array[i].data[0] == NULL)
			break;
		if ((dsh_var_array[i].flags & 1) == 0)
			continue;
		dsh_var_environ[n++] = dsh_var_array[i].data[0];
	}

	dsh_var_environ[n] = NULL;
}

int dsh_var_init(void)
{
	size_t i;

	if (environ != NULL) {
		for (i = 0; environ[i] != NULL; i++)
			dsh_var_end += 1;
	}

	dsh_var_end += 1024;

	dsh_var_array = malloc(dsh_var_end * sizeof(dsh_var_array[0]));
	dsh_var_environ = malloc(dsh_var_end * sizeof(dsh_var_environ[0]));

	if (dsh_var_array == NULL || dsh_var_environ == NULL)
		return dsh_var_free(), EXIT_FAILURE;

	for (i = 0; i < dsh_var_end; i++) {
		dsh_var_array[i].data[0] = NULL;
		dsh_var_array[i].data[1] = NULL;
		dsh_var_array[i].flags = 0;
	}

	for (i = 0; i < dsh_var_end - 1; i++) {
		if (environ[i] == NULL)
			break;
		if ((dsh_var_array[i].data[0] = strdup(environ[i])) == NULL)
			return dsh_var_free(), EXIT_FAILURE;
		dsh_var_array[i].flags = 1;
	}

	update_environ();
	return 0;
}

void dsh_var_free(void)
{
	size_t i;

	if (dsh_var_array != NULL) {
		for (i = 0; i < dsh_var_end; i++) {
			if (dsh_var_array[i].data[0] == NULL)
				break;
			free(dsh_var_array[i].data[0]);
			free(dsh_var_array[i].data[1]);
		}
	}

	free(dsh_var_array);
	dsh_var_array = NULL;

	free(dsh_var_environ);
	dsh_var_environ = NULL;
}

const char *dsh_var_read(const char *name)
{
	size_t length = 0;
	size_t i = 0;
	char c;

	while ((c = name[i++]) != '\0') {
		if (c == '=')
			return NULL;
		length += 1;
	}

	for (i = 0; dsh_var_array[i].data[0] != NULL; i++) {
		char *e = dsh_var_array[i].data[0];

		if (dsh_var_array[i].data[1] != NULL)
			e = dsh_var_array[i].data[1];

		if (strncmp(e, name, length))
			continue;

		if (e[length] == '=')
			return &e[length + 1];
	}

	return NULL;
}

void *dsh_var_write(const char *name, const char *value, size_t flags)
{
	size_t i, size = 4;
	struct dsh_var *v = NULL;
	char *data;

	if (name[0] == '\0' || strchr(name, '='))
		return NULL;

	size += strlen(name);

	if (value != NULL)
		size += strlen(value);

	if ((data = malloc(size)) == NULL)
		return NULL;

	strcpy(data, name);
	strcat(data, "=");

	size = strlen(data);

	for (i = 0; i + 1 < dsh_var_end; i++) {
		const char *d = dsh_var_array[i].data[0];

		if (d == NULL || !strncmp(d, data, size)) {
			v = &dsh_var_array[i];
			break;
		}
	}

	if (v == NULL)
		return free(data), NULL;

	if (value == NULL) {
		if (v->data[0] == NULL)
			return free(data), v;

		v->flags &= (~((size_t)3));
		v->flags |= (flags & 3);

		if ((v->flags & 1) != 0 && v->data[1] != NULL) {
			free(v->data[0]);
			v->data[0] = v->data[1];
			v->data[1] = NULL;
		}

		if ((v->flags & 2) != 0) {
			free(v->data[0]), v->data[0] = NULL;
			free(v->data[1]), v->data[1] = NULL;
			v->flags = 0;

			i = 0, size = sizeof(dsh_var_array[0]);

			while ((v + (++i))->data[0] != NULL)
				size += sizeof(dsh_var_array[0]);

			memmove(v + 0, v + 1, size);
		}

		update_environ();
		return free(data), v;
	}

	strcat(data, value);
	free(v->data[1]), v->data[1] = NULL;

	v->flags |= (flags & 1);

	if ((v->flags & 1) != 0)
		free(v->data[0]), v->data[0] = data;
	else
		v->data[(v->data[0] == NULL) ? 0 : 1] = data;

	update_environ();
	return v;
}
