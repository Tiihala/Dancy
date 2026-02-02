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
 * dsh/tab.c
 *      The Dancy Shell
 */

#include "main.h"

static void tab_list(struct dsh_prompt *state, const char *array[], int count)
{
	int i, s = -1;

	for (i = 0; i < count; i++) {
		size_t size = strlen(array[i]);

		if (size > 0 && size < 128) {
			const char *prefix = "  ";

			if (s < 0)
				prefix = "\n", s = 0;

			if (s + ((int)size + 2) >= state->ws_col)
				prefix = "\n", s = 0;

			write(1, prefix, strlen(prefix));
			write(1, array[i], size);

			s += ((int)size + 2);
		}
	}

	if (s >= 0)
		write(1, "\n", 1);
}

static int tab_qsort(const void *p1, const void *p2)
{
	char **s1 = (void *)p1;
	char **s2 = (void *)p2;

	return strcmp(*s1, *s2);
}

static void tab_command(struct dsh_prompt *state, const char *command)
{
	size_t command_length = strlen(command);
	DIR *dir = opendir("/bin");
	int i;

	const char *array[1024];
	int array_end = (int)(sizeof(array) / sizeof(array[0]));
	int array_i = 0;

	char buffer[0x10000];
	int buffer_end = (int)(sizeof(buffer) / sizeof(buffer[0]));
	int buffer_i = 0;

	while (dir != NULL) {
		struct dirent *d = readdir(dir);
		const char *p1, *p2;

		if (d == NULL) {
			closedir(dir), dir = NULL;
			break;
		}

		p1 = &buffer[buffer_i];
		p2 = &d->d_name[0];

		if (p2[0] == '.' && (!strcmp(p2, ".") || !strcmp(p2, "..")))
			continue;

		if (strchr(p2, ' '))
			continue;

		if (command_length && strncmp(p2, command, command_length))
			continue;

		for (;;) {
			if (buffer_i + 1 >= buffer_end) {
				p1 = NULL;
				break;
			}

			if ((buffer[buffer_i++] = *p2++) == '\0')
				break;
		}

		if (p1 != NULL && array_i < array_end)
			array[array_i++] = p1;
	}

	for (i = 0; dsh_builtin_array[i].name != NULL; i++) {
		const char *name = dsh_builtin_array[i].name;

		if (command_length && strncmp(name, command, command_length))
			continue;

		if (array_i < array_end)
			array[array_i++] = dsh_builtin_array[i].name;
	}

	qsort(&array[0], (size_t)array_i, sizeof(array[0]), tab_qsort);

	for (i = 1; i < array_i; i++) {
		if (!strcmp(array[i - 1], array[i]))
			array[i - 1] = "";
	}

	{
		int offset = -1;

		for (i = 0; i < array_i; i++) {
			if (array[i][0] != '\0')
				offset = (int)((offset < 0) ? i : INT_MAX);
		}

		if (offset >= 0 && offset < array_i) {
			const char *p1 = command;
			const char *p2 = array[offset];

			while (p1[0] != '\0' && p1[0] == p2[0])
				p1 += 1, p2 += 1;

			while (p2[0] != '\0') {
				unsigned char c = *((unsigned char *)(p2++));
				state->add_char(state, (int)c);
			}

			state->add_char(state, ' ');
			return;
		}
	}

	for (i = 0; /* void */; i++) {
		int c = 0, offset = 0;

		while (c >= 0 && offset < array_i) {
			if (array[offset++][0] != '\0') {
				char a = array[offset - 1][i];

				if (c == 0)
					c = (int)((unsigned char)a);

				if (c == 0 || c != (int)((unsigned char)a))
					c = -1;
			}
		}

		if (c <= 0)
			break;

		if ((size_t)i >= command_length) {
			state->add_char(state, c);
			state->c[0] = '\0', state->c[1] = '\0';
		}
	}

	if (state->c[0] == '\t')
		tab_list(state, &array[0], array_i);
}

static void tab_path(struct dsh_prompt *state, char *path, int chdir_mode)
{
	size_t name_length = 0;
	char *name = strrchr(path, '/');
	char wd[2] = { '.', '\0' };

	DIR *dir;
	int i;

	const char *array[1024];
	int array_end = (int)(sizeof(array) / sizeof(array[0]));
	int array_i = 0;

	char buffer[0x10000];
	int buffer_end = (int)(sizeof(buffer) / sizeof(buffer[0]));
	int buffer_i = 0;

	if (path[0] == '/')
		wd[0] = '/';

	if (name == NULL)
		name = path, path = &wd[0];
	else
		(++name)[-1] = '\0';

	if (path[0] == '\0')
		path = &wd[0];

	if (strlen(path) > 256)
		return;

	name_length = strlen(name);

	dir = opendir(path);

	while (dir != NULL) {
		struct dirent *d = readdir(dir);
		const char *p1, *p2;

		struct stat sb;
		char sb_path[512];

		if (d == NULL) {
			closedir(dir), dir = NULL;
			break;
		}

		p1 = &buffer[buffer_i];
		p2 = &d->d_name[0];

		if (p2[0] == '.' && (!strcmp(p2, ".") || !strcmp(p2, "..")))
			continue;

		if (strchr(p2, ' ') || strlen(p2) > 128)
			continue;

		if (name_length && strncmp(p2, name, name_length))
			continue;

		strcpy(&sb_path[0], path);
		strcat(&sb_path[0], "/");
		strcat(&sb_path[0], p2);

		if (lstat(&sb_path[0], &sb) == -1)
			continue;

		for (;;) {
			if (buffer_i + 2 >= buffer_end) {
				p1 = NULL;
				break;
			}

			if (chdir_mode && !S_ISDIR(sb.st_mode)) {
				p1 = NULL;
				break;
			}

			if ((buffer[buffer_i++] = *p2++) == '\0') {
				if (S_ISDIR(sb.st_mode)) {
					buffer[buffer_i - 1] = '/';
					buffer[buffer_i++] = '\0';
				}
				break;
			}
		}

		if (p1 != NULL && array_i < array_end)
			array[array_i++] = p1;
	}

	qsort(&array[0], (size_t)array_i, sizeof(array[0]), tab_qsort);

	for (i = 1; i < array_i; i++) {
		if (!strcmp(array[i - 1], array[i]))
			array[i - 1] = "";
	}

	{
		int offset = -1;

		for (i = 0; i < array_i; i++) {
			if (array[i][0] != '\0')
				offset = (int)((offset < 0) ? i : INT_MAX);
		}

		if (offset >= 0 && offset < array_i) {
			const char *p1 = name;
			const char *p2 = array[offset];

			while (p1[0] != '\0' && p1[0] == p2[0])
				p1 += 1, p2 += 1;

			while (p2[0] != '\0') {
				unsigned char c = *((unsigned char *)(p2++));
				state->add_char(state, (int)c);
			}

			if (!strchr(array[offset], '/'))
				state->add_char(state, ' ');
			return;
		}
	}

	for (i = 0; /* void */; i++) {
		int c = 0, offset = 0;

		while (c >= 0 && offset < array_i) {
			if (array[offset++][0] != '\0') {
				char a = array[offset - 1][i];

				if (c == 0)
					c = (int)((unsigned char)a);

				if (c == 0 || c != (int)((unsigned char)a))
					c = -1;
			}
		}

		if (c <= 0)
			break;

		if ((size_t)i >= name_length) {
			state->add_char(state, c);
			state->c[0] = '\0', state->c[1] = '\0';
		}
	}

	if (state->c[0] == '\t')
		tab_list(state, &array[0], array_i);
}

static void tab(struct dsh_prompt *state, char *line)
{
	int i, offset = state->buffer_state[0];
	int chdir_mode = 0;

	if (offset < 0 || (size_t)offset > strlen(line))
		return;

	if (line[offset] != ' ' && line[offset] != '\0')
		return;

	{
		int quote_state = 0;

		while (offset >= 2) {
			if (line[offset - 1] != ' ')
				break;
			if (line[offset - 2] != ' ')
				break;
			offset -= 1;
		}

		line[offset] = '\0';

		for (i = 0; i < offset; i++) {
			char c = line[i];

			if (quote_state == 0 && (c == '"' || c == '\'')) {
				if (i < 1 || line[i - 1] != '\\')
					quote_state = (int)c;
				continue;
			}

			if (quote_state == '"' && c == '"') {
				if (i < 1 || line[i - 1] != '\\')
					quote_state = 0;
				continue;
			}

			if (quote_state == '\'' && c == '\'') {
				quote_state = 0;
				continue;
			}
		}

		if (quote_state != 0)
			return;

		for (i = offset - 1; i >= 0; i--) {
			if (line[i] == '&' || line[i] == '|') {
				line = &line[i + 1];
				break;
			}
		}

		while (line[0] == ' ')
			line += 1;

		offset = (int)strlen(line);
	}

	if (!strchr(line, ' ') && !strchr(line, '/')) {
		tab_command(state, line);
		return;
	}

	if (!strncmp(line, "cd ", 3) || !strncmp(line, "chdir ", 6))
		chdir_mode = 1;

	for (i = offset - 1; i >= 0; i--) {
		if (line[i] == ' ' || line[i] == '=') {
			line = &line[i + 1];
			break;
		}
	}

	tab_path(state, line, chdir_mode);
}

void dsh_tab_completion(struct dsh_prompt *state)
{
	char *line;

	if (state->buffer == NULL || (line = strdup(state->buffer)) == NULL)
		return;

	tab(state, line);
	free(line);
}
