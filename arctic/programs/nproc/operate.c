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
 * nproc/operate.c
 *      Print the number of processing units
 */

#include "main.h"

int operate(struct options *opt)
{
	int smp_ap_count = 0;
	ssize_t r;

	if (opt->operands[0] != NULL)
		return opt->error = "operands are not allowed", 1;

	r = __dancy_procinfo(getpid(), __DANCY_PROCINFO_SMP_AP_COUNT,
		&smp_ap_count, sizeof(smp_ap_count));

	if (r != (ssize_t)(sizeof(smp_ap_count)))
		smp_ap_count = 0;

	printf("%d\n", smp_ap_count + 1);

	return 0;
}
