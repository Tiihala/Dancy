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
 * include/__dancy/spawn.h
 *      The Arctic Dancy Header
 */

#ifndef __DANCY_INTERNAL_SPAWN_H
#define __DANCY_INTERNAL_SPAWN_H

#include <__dancy/core.h>

__Dancy_Header_Begin

struct __dancy_spawn_file_actions {
	unsigned int __state;
};

struct __dancy_spawn_attributes {
	unsigned int __state;
	int __flags;
	unsigned long long __pgroup;
	unsigned long long __sigdef;
	unsigned long long __sigmask;
	int __sched[2];
	int __other[2];
};

struct __dancy_spawn_options {
	struct __dancy_spawn_file_actions *actions;
	struct __dancy_spawn_attributes *attrp;
};

__Dancy_Header_End

#endif
