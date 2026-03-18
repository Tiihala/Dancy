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
 * sys/resource.h
 *      Definitions for resource operations
 */

#ifndef __DANCY_SYS_RESOURCE_H
#define __DANCY_SYS_RESOURCE_H

#include <__dancy/core.h>
#include <__dancy/timeval.h>

__Dancy_Header_Begin

#ifndef __DANCY_TYPEDEF_ID_T
#define __DANCY_TYPEDEF_ID_T
typedef __dancy_id_t id_t;
#endif

typedef unsigned long long rlim_t;

struct rlimit {
	rlim_t rlim_cur;
	rlim_t rlim_max;
};

struct rusage {
	struct timeval ru_utime;
	struct timeval ru_stime;
};

#define PRIO_PROCESS    (0x00010)
#define PRIO_PGRP       (0x00011)
#define PRIO_USER       (0x00012)

#define RLIM_INFINITY   (~(0ull))
#define RLIM_SAVED_MAX  (~(0ull))
#define RLIM_SAVED_CUR  (~(0ull))

#define RLIMIT_CORE     (0x00020)
#define RLIMIT_CPU      (0x00021)
#define RLIMIT_DATA     (0x00022)
#define RLIMIT_FSIZE    (0x00023)
#define RLIMIT_NOFILE   (0x00024)
#define RLIMIT_STACK    (0x00025)
#define RLIMIT_AS       (0x00026)

#define RUSAGE_SELF     (0x00030)
#define RUSAGE_CHILDREN (0x00031)

int getpriority(int which, id_t who);
int setpriority(int which, id_t who, int value);

int getrlimit(int resource, struct rlimit *rlp);
int setrlimit(int resource, const struct rlimit *rlp);

int getrusage(int who, struct rusage *r_usage);

__Dancy_Header_End

#endif
