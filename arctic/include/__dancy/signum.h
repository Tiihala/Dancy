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
 * include/__dancy/signum.h
 *      The Arctic Dancy Header
 */

#ifndef __DANCY_INTERNAL_SIGNUM_H
#define __DANCY_INTERNAL_SIGNUM_H

#include <__dancy/core.h>

__Dancy_Header_Begin

#define SIGABRT     6
#define SIGALRM     14
#define SIGBUS      7
#define SIGCHLD     17
#define SIGCONT     18
#define SIGFPE      8
#define SIGHUP      1
#define SIGILL      4
#define SIGINT      2
#define SIGKILL     9
#define SIGPIPE     13
#define SIGPOLL     29
#define SIGPROF     27
#define SIGQUIT     3
#define SIGSEGV     11
#define SIGSTOP     19
#define SIGSYS      31
#define SIGTERM     15
#define SIGTRAP     5
#define SIGTSTP     20
#define SIGTTIN     21
#define SIGTTOU     22
#define SIGURG      23
#define SIGUSR1     10
#define SIGUSR2     12
#define SIGVTALRM   26
#define SIGWINCH    28
#define SIGXCPU     24
#define SIGXFSZ     25

/*
 * #define __DANCY_SIGNAL_MIN    1
 * #define __DANCY_SIGNAL_MAX    31
 * #define __DANCY_SIGNAL_COUNT  29
 */

__Dancy_Header_End

#endif
