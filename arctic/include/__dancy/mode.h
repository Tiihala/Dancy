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
 * include/__dancy/mode.h
 *      The Arctic Dancy Header
 */

#ifndef __DANCY_INTERNAL_MODE_H
#define __DANCY_INTERNAL_MODE_H

#include <__dancy/core.h>

__Dancy_Header_Begin

#define __DANCY_S_IFMT    0xF000  /* 0170000 */
#define __DANCY_S_IFIFO   0x1000  /* 0010000 */
#define __DANCY_S_IFCHR   0x2000  /* 0020000 */
#define __DANCY_S_IFDIR   0x4000  /* 0040000 */
#define __DANCY_S_IFBLK   0x6000  /* 0060000 */
#define __DANCY_S_IFREG   0x8000  /* 0100000 */
#define __DANCY_S_IFLNK   0xA000  /* 0120000 */
#define __DANCY_S_IFSOCK  0xC000  /* 0140000 */

#define __DANCY_S_IXOTH   0x0001  /* 0000001 */
#define __DANCY_S_IWOTH   0x0002  /* 0000002 */
#define __DANCY_S_IROTH   0x0004  /* 0000004 */
#define __DANCY_S_IRWXO   0x0007  /* 0000007 */

#define __DANCY_S_IXGRP   0x0008  /* 0000010 */
#define __DANCY_S_IWGRP   0x0010  /* 0000020 */
#define __DANCY_S_IRGRP   0x0020  /* 0000040 */
#define __DANCY_S_IRWXG   0x0038  /* 0000070 */

#define __DANCY_S_IXUSR   0x0040  /* 0000100 */
#define __DANCY_S_IWUSR   0x0080  /* 0000200 */
#define __DANCY_S_IRUSR   0x0100  /* 0000400 */
#define __DANCY_S_IRWXU   0x01C0  /* 0000700 */

#define __DANCY_S_ISVTX   0x0200  /* 0001000 */
#define __DANCY_S_ISGID   0x0400  /* 0002000 */
#define __DANCY_S_ISUID   0x0800  /* 0004000 */

__Dancy_Header_End

#endif
