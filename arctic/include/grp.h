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
 * grp.h
 *      Group structure
 */

#ifndef __DANCY_GRP_H
#define __DANCY_GRP_H

#include <__dancy/core.h>

__Dancy_Header_Begin

#ifndef __DANCY_TYPEDEF_GID_T
#define __DANCY_TYPEDEF_GID_T
typedef __dancy_gid_t gid_t;
#endif

struct group {
	char *gr_name;
	gid_t gr_gid;
	char **gr_mem;
};

void endgrent(void);
void setgrent(void);

struct group * getgrent (void);

struct group *getgrnam(const char *name);
struct group *getgrgid(gid_t gid);

int getgrnam_r(const char *name, struct group *grp, char *buffer,
	size_t buffer_size, struct group **result);

int getgrgid_r(gid_t gid, struct group *grp, char *buffer,
	size_t buffer_size, struct group **result);

__Dancy_Header_End

#endif
