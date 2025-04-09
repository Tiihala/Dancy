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
 * dsh/prompt.c
 *      An interactive input editor
 */

#include "prompt.h"

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define DSH_PROMPT_BUFFER_SIZE 4096

static void get_winsize(struct dsh_prompt *state)
{
	struct winsize ws = { 0, 0, 0, 0 };
	const unsigned int max = 0x7FFF;

	state->ws_col = 80;
	state->ws_row = 25;

	ioctl(1, TIOCGWINSZ, &ws);

	if (ws.ws_col == 0 || ws.ws_col > max)
		return;

	if (ws.ws_row == 0 || ws.ws_row > max)
		return;

	state->ws_col = ws.ws_col;
	state->ws_row = ws.ws_row;
}

static void set_unbuffered_mode(struct dsh_prompt *state)
{
	struct termios new_termios;

	size_t size = sizeof(*state->default_termios);
	memcpy(&new_termios, state->default_termios, size);

	new_termios.c_lflag &= ~((tcflag_t)ECHO);
	new_termios.c_lflag &= ~((tcflag_t)ECHONL);
	new_termios.c_lflag &= ~((tcflag_t)ICANON);
	new_termios.c_lflag &= ~((tcflag_t)IEXTEN);

	if ((errno = 0, tcsetattr(1, TCSAFLUSH, &new_termios)) != 0)
		perror("tcsetattr");
}

static void restore_termios(struct dsh_prompt *state)
{
	if (!state->default_termios)
		return;

	state->default_termios->c_lflag |= (tcflag_t)ECHO;
	state->default_termios->c_lflag |= (tcflag_t)ECHONL;
	state->default_termios->c_lflag |= (tcflag_t)ICANON;
	state->default_termios->c_lflag |= (tcflag_t)IEXTEN;

	if ((errno = 0, tcsetattr(1, TCSAFLUSH, state->default_termios)) != 0)
		perror("tcsetattr");

	memset(state->default_termios, 0, sizeof(*state->default_termios));
	state->default_termios = NULL;
}

static int save_termios(struct dsh_prompt *state)
{
	if (state->default_termios)
		restore_termios(state);

	if ((errno = 0, tcgetattr(1, &state->_default_termios)) != 0) {
		if (errno == ENOTTY)
			perror("tcgetattr");
		return EXIT_FAILURE;
	}

	state->default_termios = &state->_default_termios;
	set_unbuffered_mode(state);

	return 0;
}

static int count_columns(const char *input)
{
	int i, r = 0, utf8 = 0;

	for (i = 0; input[i] != '\0'; i++) {
		int c = (int)((unsigned char)input[i]);

		if ((c & 0x80) == 0) {
			utf8 = 0;
			r += 1;
			continue;
		}

		if (utf8 > 0) {
			if (--utf8 == 0)
				r += 1;
			continue;
		}

		if ((c & 0xE0) == 0xC0) {
			utf8 = 1;
			continue;
		}

		if ((c & 0xF0) == 0xE0) {
			utf8 = 2;
			continue;
		}

		if ((c & 0xF8) == 0xF0) {
			utf8 = 8;
			continue;
		}
	}

	return r;
}

static int line_add(char *line, size_t size[2], const char *input, int count)
{
	int i = 0;

	for (;;) {
		int c = (int)((unsigned char)*input);

		if (c == '\0')
			break;

		if (size[0] >= size[1])
			return EXIT_FAILURE;

		i += ((c & 0xC0) != 0x80);

		if (i > count)
			break;

		line[size[0]++] = *input++;
	}

	return 0;
}

static int render(struct dsh_prompt *state)
{
	const char *input_left = &state->buffer[0];
	const char *input_right = input_left + state->buffer_state[0];

	int columns_max = state->ws_col - (state->prompt_offset + 1);
	int trim_left = 0, trim_right = 0;
	int columns_left, columns_right;

	char line[1024];
	size_t size[2] = { 0, sizeof(line) };

	if (state->skip_render > 0) {
		state->skip_render -= 1;
		return 0;
	}

	if (columns_max < 8)
		return EXIT_FAILURE;

	columns_right = count_columns(input_right);
	columns_left = count_columns(input_left) - columns_right;

	while (columns_left + columns_right > columns_max) {
		if (columns_left > (columns_max / 2)) {
			columns_left -= 1;
			trim_left += 1;
			continue;
		}
		if (columns_right > 0) {
			columns_right -= 1;
			trim_right += 1;
			continue;
		}
	}

	if (line_add(&line[0], size, "\033[m\r\033[K", INT_MAX))
		return EXIT_FAILURE;

	if (line_add(&line[0], size, &state->prompt[0], INT_MAX))
		return EXIT_FAILURE;

	if (trim_left) {
		int count = trim_left;

		while (count > 0) {
			int c = (int)((unsigned char)(*(++input_left)));

			if ((c & 0xC0) != 0x80)
				count -= 1;
		}

		if (line_add(&line[0], size, "\010\033[36m<\033[m", INT_MAX))
			return EXIT_FAILURE;
	}

	if (line_add(&line[0], size, input_left, columns_left))
		return EXIT_FAILURE;

	if (line_add(&line[0], size, input_right, columns_right))
		return EXIT_FAILURE;

	if (trim_right) {
		if (line_add(&line[0], size, "\033[36m>\033[m", INT_MAX))
			return EXIT_FAILURE;
	}

	{
		char b[32];
		int g = state->prompt_offset + 1 + columns_left;

		int r = snprintf(&b[0], sizeof(b), "\033[%dG", g);

		if (r <= 0 || line_add(&line[0], size, &b[0], INT_MAX))
			return EXIT_FAILURE;

		r = (int)write(1, &line[0], size[0]);

		if (r != (int)size[0])
			return EXIT_FAILURE;
	}

	return 0;
}

static void add_char(struct dsh_prompt *state, int c)
{
	char *b = state->buffer;
	int offset = state->buffer_state[0];
	int i;

	if (offset + 8 >= DSH_PROMPT_BUFFER_SIZE)
		return;

	if (c <= 0x1F) {
		if (c == 0x1B)
			add_char(state, 0xC2), add_char(state, 0xB7);
		return;
	}

	for (i = state->buffer_state[1] - offset - 1; i >= 0; i--)
		b[offset + i + 1] = b[offset + i];

	b[state->buffer_state[0]++] = (char)c;
	b[++state->buffer_state[1]] = '\0';
}

static void del_char(struct dsh_prompt *state)
{
	char *b = state->buffer;
	int offset = state->buffer_state[0];
	int i, size;

	while ((size = state->buffer_state[1] - offset) > 0) {
		for (i = 0; i < size; i++)
			b[offset + i] = b[offset + i + 1];

		b[--state->buffer_state[1]] = '\0';

		if (((unsigned char)b[offset] & 0xC0) != 0x80)
			break;
	}

	if (state->buffer_state[0] > state->buffer_state[1])
		state->buffer_state[0] = state->buffer_state[1];
}

static int move_cursor(struct dsh_prompt *state, int columns)
{
	const char *b = state->buffer;
	int i, r = 0;

	if (columns < 0) {
		for (i = 0; i > columns; i--) {
			int j = state->buffer_state[0];

			if ((j -= 1) < 0)
				break;

			while (j > 0 && ((unsigned char)b[j] & 0xC0) == 0x80)
				j -= 1;

			state->buffer_state[0] = j;
			r -= 1;
		}

	} else {
		for (i = 0; i < columns; i++) {
			int j = state->buffer_state[0];
			int k = state->buffer_state[1];

			if ((j += 1) > k)
				break;

			while (j < k && ((unsigned char)b[j] & 0xC0) == 0x80)
				j += 1;

			state->buffer_state[0] = j;
			r += 1;
		}
	}

	return r;
}

static int prompt_read(struct dsh_prompt *state)
{
	size_t histories = sizeof(state->history) / sizeof(state->history[0]);
	char *buffer = malloc(DSH_PROMPT_BUFFER_SIZE);

	if ((state->buffer = buffer) == NULL)
		return EXIT_FAILURE;

	if (state->prompt_offset >= state->ws_col)
		return EXIT_FAILURE;

	state->buffer[0] = '\0';
	state->buffer_state[0] = 0;
	state->buffer_state[1] = 0;

	state->escape[0] = '\0';
	state->escape_state = 0;

	state->skip_render = 0;
	state->history_state = 0;

	while (!render(state)) {
		int c = getchar();

		if ((c == EOF || c == 0x04) && state->buffer[0] == '\0')
			return EXIT_FAILURE;

		if (c == EOF || c == '\n' || (char)c == '\0')
			return 0;

		if ((c & 0x80) != 0) {
			if ((c & 0xE0) == 0xC0)
				state->skip_render = 1;

			else if ((c & 0xF0) == 0xE0)
				state->skip_render = 2;

			else if ((c & 0xF8) == 0xF0)
				state->skip_render = 3;
		} else {
			state->skip_render = 0;
		}

		if (c == '\r') {
			state->escape_state = 0;
			continue;
		}

		if (state->escape_state) {
			int size = (int)(sizeof(state->escape));
			char *p = &state->escape[0];

			if (state->escape_state + 1 < size) {
				p[state->escape_state++] = (char)c;
				p[state->escape_state] = '\0';
			}

			if (c >= 0x20 && c <= 0x3F)
				continue;

			if (!strcmp(p, "\033[") || !strcmp(p, "\033[[")) {
				continue;

			} else if (!strcmp(p, "\033[3~")) {
				del_char(state);

			} else if (!strcmp(p, "\033[A")) {
				int i = state->history_state;
				char *b = NULL;

				if (i + 1 < (int)histories) {
					if (state->history[i + 1])
						b = strdup(state->buffer);
				}

				if (b) {
					if (state->history[i])
						free(state->history[i]);

					state->history[i] = b;

					strcpy(state->buffer,
						state->history[i + 1]);

					i = (int)strlen(state->buffer);
					state->buffer_state[0] = i;
					state->buffer_state[1] = i;

					state->history_state += 1;
				}

			} else if (!strcmp(p, "\033[B")) {
				int i = state->history_state;
				char *b = NULL;

				if (i > 0) {
					if (state->history[i - 1])
						b = strdup(state->buffer);
				}

				if (b) {
					if (state->history[i])
						free(state->history[i]);

					state->history[i] = b;

					strcpy(state->buffer,
						state->history[i - 1]);

					i = (int)strlen(state->buffer);
					state->buffer_state[0] = i;
					state->buffer_state[1] = i;

					state->history_state -= 1;
				}

			} else if (!strcmp(p, "\033[C")) {
				move_cursor(state, 1);

			} else if (!strcmp(p, "\033[D")) {
				move_cursor(state, -1);

			} else if (!strcmp(p, "\033[F")) {
				move_cursor(state, INT_MAX);

			} else if (!strcmp(p, "\033[H")) {
				move_cursor(state, INT_MIN);

			}

			while ((c & 0x80) != 0 && state->escape_state > 0) {
				add_char(state, (int)((unsigned char)*p++));
				state->escape_state -= 1;
			}

			state->escape_state = 0;
			continue;
		}

		if (c == 0x1B) {
			char *p = &state->escape[0];
			p[state->escape_state++] = (char)c;
			p[state->escape_state] = '\0';
			continue;
		}

		if (c == 0x7F) {
			if (move_cursor(state, -1) == -1)
				del_char(state);
			continue;
		}

		add_char(state, c);
	}

	return EXIT_FAILURE;
}

int dsh_prompt_available(void)
{
	struct dsh_prompt state;
	int r;

	if (!isatty(0))
		return 0;

	dsh_prompt_init(&state);
	r = save_termios(&state);
	restore_termios(&state);

	return !r;
}

void dsh_prompt_init(struct dsh_prompt *state)
{
	static struct dsh_prompt null_state;
	memcpy(state, &null_state, sizeof(*state));
}

void dsh_prompt_free(struct dsh_prompt *state)
{
	size_t histories = sizeof(state->history) / sizeof(state->history[0]);
	size_t i;

	for (i = 0; i < histories; i++) {
		if (state->history[i])
			free(state->history[i]), state->history[i] = NULL;
	}
}

char *dsh_prompt_read(struct dsh_prompt *state,
	const char *prompt, int prompt_offset)
{
	size_t histories = sizeof(state->history) / sizeof(state->history[0]);
	size_t i;

	state->buffer = NULL;

	if (save_termios(state))
		return NULL;

	get_winsize(state);

	state->prompt = (prompt != NULL) ? prompt : "";
	state->prompt_offset = (prompt_offset >= 0) ? prompt_offset : 0;

	if (prompt_read(state)) {
		free(state->buffer);
		state->buffer = NULL;
	}

	restore_termios(state);

	state->prompt = NULL;
	state->prompt_offset = 0;

	if (state->history[0])
		free(state->history[0]), state->history[0] = NULL;

	if (state->buffer) {
		char *buffer = state->buffer;

		while (*buffer != '\0') {
			if (*buffer++ != ' ') {
				buffer = NULL;
				break;
			}
		}

		if (buffer)
			return state->buffer;

		if (state->history[1]) {
			if (!strcmp(state->history[1], state->buffer))
				return state->buffer;
		}

		if ((buffer = strdup(state->buffer)) == NULL)
			return state->buffer;

		if (state->history[histories - 1])
			free(state->history[histories - 1]);

		for (i = histories - 1; i > 1; i--)
			state->history[i] = state->history[i - 1];

		state->history[1] = buffer;
	}

	return state->buffer;
}
