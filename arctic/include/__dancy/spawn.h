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

#define __DANCY_SPAWN_ADD_CLOSE (1)
#define __DANCY_SPAWN_ADD_DUP2  (2)
#define __DANCY_SPAWN_ADD_OPEN  (3)

struct __dancy_spawn_file_actions {
	unsigned int __state;
	unsigned int __count;
	struct {
		int __type;
		int __args[3];
		const void *__path;
	} __actions[128];
};

#define __DANCY_SPAWN_RESETIDS        (0x0001)
#define __DANCY_SPAWN_SETPGROUP       (0x0002)
#define __DANCY_SPAWN_SETSIGDEF       (0x0004)
#define __DANCY_SPAWN_SETSIGMASK      (0x0008)
#define __DANCY_SPAWN_SETSCHEDPARAM   (0x0010)
#define __DANCY_SPAWN_SETSCHEDULER    (0x0020)
#define __DANCY_SPAWN_USEVFORK        (0x0040)
#define __DANCY_SPAWN_SETSID          (0x0080)

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
	const struct __dancy_spawn_file_actions *actions;
	const struct __dancy_spawn_attributes *attrp;
};

__Dancy_Header_End

#endif
