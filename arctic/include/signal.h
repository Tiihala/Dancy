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
 * signal.h
 *      The C Standard Library
 */

#ifndef __DANCY_SIGNAL_H
#define __DANCY_SIGNAL_H

#include <__dancy/core.h>
#include <__dancy/signum.h>

__Dancy_Header_Begin

#ifndef __DANCY_TYPEDEF_PID_T
#define __DANCY_TYPEDEF_PID_T
typedef __dancy_pid_t pid_t;
#endif

#ifndef __DANCY_TYPEDEF_SIG_ATOMIC_T
#define __DANCY_TYPEDEF_SIG_ATOMIC_T
typedef __dancy_sig_atomic_t sig_atomic_t;
#endif

#ifndef __DANCY_TYPEDEF_SIGSET_T
#define __DANCY_TYPEDEF_SIGSET_T
typedef __dancy_sigset_t sigset_t;
#endif

void (*signal(int sig, void (*func)(int)))(int);
int raise(int sig);

int kill(pid_t pid, int sig);

int sigemptyset(sigset_t *set);
int sigfillset(sigset_t *set);

int sigaddset(sigset_t *set, int sig);
int sigdelset(sigset_t *set, int sig);

int sigpending(sigset_t *out);
int sigprocmask(int how, const sigset_t *set, sigset_t *out);
int sigismember(const sigset_t *set, int sig);

__Dancy_Header_End

#endif
