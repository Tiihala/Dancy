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
 * sys/socket.h
 *      The main sockets header
 */

#ifndef __DANCY_SYS_SOCKET_H
#define __DANCY_SYS_SOCKET_H

#include <__dancy/core.h>

__Dancy_Header_Begin

#ifndef __DANCY_TYPEDEF_SOCKLEN_T
#define __DANCY_TYPEDEF_SOCKLEN_T
typedef size_t socklen_t;
#endif

#ifndef __DANCY_TYPEDEF_SA_FAMILY_T
#define __DANCY_TYPEDEF_SA_FAMILY_T
typedef unsigned short sa_family_t;
#endif

#define SOCK_DGRAM     (1)
#define SOCK_RAW       (2)
#define SOCK_SEQPACKET (3)
#define SOCK_STREAM    (4)

#define SOCK_NONBLOCK  (1 << 10)
#define SOCK_CLOEXEC   (1 << 11)
#define SOCK_CLOFORK   (1 << 12)

#define AF_UNSPEC      (0)
#define AF_INET        (1)
#define AF_INET6       (2)
#define AF_UNIX        (3)

struct sockaddr {
	sa_family_t sa_family;
	char sa_data[14];
};

int socket(int domain, int type, int protocol);
int connect(int socket, const struct sockaddr *addr, socklen_t addr_len);

int bind(int socket, const struct sockaddr *addr, socklen_t addr_len);
int listen(int socket, int backlog);

__Dancy_Header_End

#endif
