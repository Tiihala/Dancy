/*
 * Copyright (c) 2024 Antti Tiihala
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
 * include/__dancy/proc.h
 *      The Arctic Dancy Header
 */

#ifndef __DANCY_INTERNAL_PROC_H
#define __DANCY_INTERNAL_PROC_H

#include <__dancy/core.h>
#include <__dancy/ssize.h>

__Dancy_Header_Begin

#ifndef __DANCY_TYPEDEF_PID_T
#define __DANCY_TYPEDEF_PID_T
typedef __dancy_pid_t pid_t;
#endif

#define __DANCY_PROCINFO_OWNER_ID       (0x1000)
#define __DANCY_PROCINFO_GROUP_ID       (0x1001)
#define __DANCY_PROCINFO_SESSION_ID     (0x1002)
#define __DANCY_PROCINFO_CMDLINE        (0x1003)

ssize_t __dancy_proclist(pid_t *buffer, size_t size);
ssize_t __dancy_procinfo(pid_t pid, int request, void *buffer, size_t size);

__Dancy_Header_End

#endif
