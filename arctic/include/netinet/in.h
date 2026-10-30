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
 * netinet/in.h
 *      Internet address family
 */

#ifndef __DANCY_NETINET_IN_H
#define __DANCY_NETINET_IN_H

#include <__dancy/core.h>
#include <inttypes.h>

__Dancy_Header_Begin

typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;

#ifndef __DANCY_TYPEDEF_SA_FAMILY_T
#define __DANCY_TYPEDEF_SA_FAMILY_T
typedef unsigned short sa_family_t;
#endif

struct in_addr {
	in_addr_t s_addr;
};

struct in6_addr {
	uint8_t s6_addr[16];
};

struct sockaddr_in {
	sa_family_t sin_family;
	in_port_t sin_port;
	struct in_addr sin_addr;
};

struct sockaddr_in6 {
	sa_family_t sin6_family;
	in_port_t sin6_port;
	uint32_t sin6_flowinfo;
	struct in6_addr sin6_addr;
	uint32_t sin6_scope_id;
};

__Dancy_Header_End

#endif
