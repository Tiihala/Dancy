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
 * dsh/prompt.h
 *      An interactive input editor
 */

#ifndef __DANCY_DSH_PROMPT_H
#define __DANCY_DSH_PROMPT_H

#include <termios.h>

struct dsh_prompt {
	struct termios _default_termios;
	struct termios *default_termios;

	int ws_col;
	int ws_row;

	const char *prompt;
	int prompt_offset;

	char *buffer;
	int buffer_state[2];

	char escape[8];
	int escape_state;

	int skip_render;
};

int dsh_prompt_available(void);
void dsh_prompt_init(struct dsh_prompt *state);
void dsh_prompt_free(struct dsh_prompt *state);

char *dsh_prompt_read(struct dsh_prompt *state,
	const char *prompt, int prompt_offset);

#endif
