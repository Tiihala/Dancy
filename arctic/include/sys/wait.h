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
 * sys/wait.h
 *      Declarations for waiting
 */

#ifndef __DANCY_SYS_WAIT_H
#define __DANCY_SYS_WAIT_H

#include <__dancy/core.h>

__Dancy_Header_Begin

#define WNOHANG             0x0001
#define WUNTRACED           0x0002

#define WEXITSTATUS(status) (((status) >> 8) & 255)
#define WIFEXITED(status)   (((status) & 127) == 0)
#define WIFSIGNALED(status) (((status) & 127) > 0 && ((status) & 127) < 127)
#define WIFSTOPPED(status)  (((status) & 255) == 127)
#define WSTOPSIG(status)    (((status) >> 8) & 255)
#define WTERMSIG(status)    (((status) >> 0) & 127)

#ifndef __DANCY_TYPEDEF_ID_T
#define __DANCY_TYPEDEF_ID_T
typedef __dancy_id_t id_t;
#endif

#ifndef __DANCY_TYPEDEF_PID_T
#define __DANCY_TYPEDEF_PID_T
typedef __dancy_pid_t pid_t;
#endif

pid_t wait(int *status);
pid_t waitpid(pid_t pid, int *status, int options);

__Dancy_Header_End

#endif
