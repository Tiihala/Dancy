/*
 * Copyright (c) 2019 Antti Tiihala
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
 * core/start.c
 *      Kernel of Dancy Operating System
 */

#include <dancy.h>

struct core_information core;

static int core_error(int err)
{
	return err;
}

int core_start(struct core_information *info, uint32_t cpu)
{
	if (info != NULL)
		memcpy(&core, info, sizeof(core));
	if (memcmp(&core.magic[0], CORE_MAGIC, strlen(CORE_MAGIC)))
		return core_error(ERROR_SIGNATURE);
	if (core.cpu_max > cpu)
		return core_error(ERROR_INVALID_PARAMETER);

	return 0;
}
