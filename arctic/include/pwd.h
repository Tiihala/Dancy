/*
 * Copyright (c) 2025, 2026 Antti Tiihala
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
 * pwd.h
 *      Password structure
 */

#ifndef __DANCY_PWD_H
#define __DANCY_PWD_H

#include <__dancy/core.h>

__Dancy_Header_Begin

#ifndef __DANCY_TYPEDEF_GID_T
#define __DANCY_TYPEDEF_GID_T
typedef __dancy_gid_t gid_t;
#endif

#ifndef __DANCY_TYPEDEF_UID_T
#define __DANCY_TYPEDEF_UID_T
typedef __dancy_uid_t uid_t;
#endif

struct passwd {
	char *pw_name;
	uid_t pw_uid;
	gid_t pw_gid;
	char *pw_dir;
	char *pw_shell;
};

void endpwent(void);
void setpwent(void);

struct passwd * getpwent (void);

struct passwd *getpwnam(const char *name);
struct passwd *getpwuid(uid_t uid);

int getpwnam_r(const char *name, struct passwd *pwd, char *buffer,
	size_t buffer_size, struct passwd **result);

int getpwuid_r(uid_t uid, struct passwd *pwd, char *buffer,
	size_t buffer_size, struct passwd **result);

__Dancy_Header_End

#endif
