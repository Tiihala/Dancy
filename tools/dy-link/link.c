/*
 * Copyright (c) 2018 Antti Tiihala
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
 * dy-link/link.c
 *      Linker for Dancy operating system
 */

#include "program.h"

static int link_end(struct options *opt, unsigned char *buf, size_t size)
{
	FILE *fp = stdout;
	int is_stdout = 1;

	if (!buf || !size)
		return 1;

	if (strcmp(opt->arg_o, "-")) {
		fp = (errno = 0, fopen(opt->arg_o, "wb"));
		if (!fp) {
			const char *fmt = "Error: output \"%s\" (%s)\n";
			fprintf(stderr, fmt, opt->arg_o, strerror(errno));
			return free(buf), 1;
		}
		is_stdout = 0;
	}
	if ((errno = 0, fwrite(buf, 1u, size, fp)) != size) {
		perror("Error");
		if (!is_stdout)
			(void)fclose(fp);
		return free(buf), 1;
	}
	free(buf);
	errno = 0;

	if (is_stdout)
		return fflush(fp) ? perror("Error"), 1 : 0;
	return fclose(fp) ? perror("Error"), 1 : 0;
}

int link_main(struct options *opt)
{
	unsigned char *buf = NULL;
	size_t size = 0;

	if (!opt->arg_o)
		return 0;

	if (opt->verbose) {
		printf("reloc1: %i\n", section_sizeof_text_reloc(opt));
		printf("reloc2: %i\n", section_sizeof_rdata_reloc(opt));
		printf("reloc3: %i\n", section_sizeof_text_reloc(opt));
	}

	return link_end(opt, buf, size);
}
