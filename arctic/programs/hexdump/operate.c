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
 * hexdump/operate.c
 *      Display file contents in hexadecimal
 */

#include "main.h"

static struct options *hd_opt;
static unsigned long long offset;
static unsigned char *input_buffer;

static void print_line(const unsigned char *buffer, int size)
{
	static unsigned char prev_buffer[16];
	static int state;
	int i;

	if (!hd_opt->verbose && size == 16) {
		if (offset != 0 && (offset & 15) == 0) {
			if (!memcmp(&prev_buffer[0], &buffer[0], 16)) {
				if (state == 0) {
					printf("*\n");
					state = 1;
				}
				offset += 16;
				return;
			}
			state = 0;
		}
		memcpy(&prev_buffer[0], &buffer[0], 16);
	}

	if (!hd_opt->canonical) {
		printf("%07llx", offset);
		offset += (unsigned long long)size;

		for (i = 0; i < 16; i += 2) {
			unsigned int b0 = buffer[i];
			unsigned int b1 = 0;

			if (i >= size) {
				printf("%5s", "");
				continue;
			}

			if (i + 1 < size)
				b1 = buffer[i + 1];

			printf(" %02x%02x", b1, b0);
		}

		printf("\n");
		return;
	}

	printf("%08llx  ", offset);
	offset += (unsigned long long)size;

	for (i = 0; i < 16; i++) {
		unsigned int b = buffer[i];

		if (i < size)
			printf("%02x ", b);
		else
			printf("%3s", "");

		if (i == 7 || i == 15)
			printf(" ");
	}

	printf("|");

	for (i = 0; i < size; i++) {
		unsigned int b = buffer[i];

		if (b >= 0x20 && b <= 0x7E)
			printf("%c", b);
		else
			printf(".");
	}

	printf("|\n");
}

static int read_input(int fd)
{
	size_t i = 0, j = 0;

	for (;;) {
		ssize_t size = read(fd, &input_buffer[i], 4096 - i);

		if (size < 0)
			return perror(MAIN_CMDNAME ": read"), EXIT_FAILURE;

		if (size > 0 && (i += (size_t)size) < 16)
			continue;

		while (i > 0) {
			int s = ((i < 16) ? (int)i : 16);
			print_line(&input_buffer[j], s);
			i -= (size_t)s;
			j += (size_t)s;
		}

		j = 0;

		if (size == 0)
			break;
	}

	if (offset > 0) {
		if (!hd_opt->canonical)
			printf("%07llx\n", offset);
		else
			printf("%08llx\n", offset);
	}

	return 0;
}

static int hd(const char *path)
{
	int fd, r;

	if (path[0] == '-' && path[1] == '\0') {
		return read_input(0);
	}

	if ((fd = open(path, O_RDONLY)) < 0)
		return perror(path), EXIT_FAILURE;

	r = read_input(fd);
	close(fd);

	return r;
}

int operate(struct options *opt)
{
	int i, r;

	hd_opt = opt;

	if ((input_buffer = aligned_alloc(4096, 4096)) == NULL) {
		fputs(MAIN_CMDNAME ": out of memory\n", stderr);
		return EXIT_FAILURE;
	}

	if (opt->operands[0] == NULL) {
		r = hd("-");
		return free(input_buffer), r;
	}

	for (i = 0; opt->operands[i] != NULL; i++) {
		if ((r = hd(opt->operands[i])) != 0)
			return free(input_buffer), r;
	}

	return free(input_buffer), 0;
}
