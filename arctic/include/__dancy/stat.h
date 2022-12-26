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
 * include/__dancy/stat.h
 *      The Arctic Dancy Header
 */

#ifndef __DANCY_INTERNAL_STAT_H
#define __DANCY_INTERNAL_STAT_H

#include <__dancy/core.h>
#include <__dancy/timespec.h>

__Dancy_Header_Begin

#ifndef __DANCY_TYPEDEF_BLKCNT_T
#define __DANCY_TYPEDEF_BLKCNT_T
typedef __dancy_blkcnt_t blkcnt_t;
#endif

#ifndef __DANCY_TYPEDEF_BLKSIZE_T
#define __DANCY_TYPEDEF_BLKSIZE_T
typedef __dancy_blksize_t blksize_t;
#endif

#ifndef __DANCY_TYPEDEF_DEV_T
#define __DANCY_TYPEDEF_DEV_T
typedef __dancy_dev_t dev_t;
#endif

#ifndef __DANCY_TYPEDEF_GID_T
#define __DANCY_TYPEDEF_GID_T
typedef __dancy_gid_t gid_t;
#endif

#ifndef __DANCY_TYPEDEF_INO_T
#define __DANCY_TYPEDEF_INO_T
typedef __dancy_ino_t ino_t;
#endif

#ifndef __DANCY_TYPEDEF_MODE_T
#define __DANCY_TYPEDEF_MODE_T
typedef __dancy_mode_t mode_t;
#endif

#ifndef __DANCY_TYPEDEF_NLINK_T
#define __DANCY_TYPEDEF_NLINK_T
typedef __dancy_nlink_t nlink_t;
#endif

#ifndef __DANCY_TYPEDEF_OFF_T
#define __DANCY_TYPEDEF_OFF_T
typedef __dancy_off_t off_t;
#endif

#ifndef __DANCY_TYPEDEF_UID_T
#define __DANCY_TYPEDEF_UID_T
typedef __dancy_uid_t uid_t;
#endif

struct stat {
	dev_t st_dev;
	ino_t st_ino;

	mode_t st_mode;
	nlink_t st_nlink;

	uid_t st_uid;
	gid_t st_gid;
	dev_t st_rdev;
	off_t st_size;

	struct timespec st_atim;
	struct timespec st_mtim;
	struct timespec st_ctim;

	blksize_t st_blksize;
	blkcnt_t st_blocks;
};

__Dancy_Header_End

#endif
