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
 * errno.h
 *      System error numbers
 */

#ifndef ARCTIC_DANCY_ERRNO_H
#define ARCTIC_DANCY_ERRNO_H

#include <__dancy/core.h>

#undef errno
#define errno (*__dancy_errno())

#define __DANCY_ERRNO_C_STANDARD 256

#define EDOM                    (__DANCY_ERRNO_C_STANDARD + 1)
#define EILSEQ                  (__DANCY_ERRNO_C_STANDARD + 2)
#define ERANGE                  (__DANCY_ERRNO_C_STANDARD + 3)

#define __DANCY_ERRNO_OPEN_GROUP 512

#define E2BIG                   (__DANCY_ERRNO_OPEN_GROUP + 1)
#define EACCES                  (__DANCY_ERRNO_OPEN_GROUP + 2)
#define EADDRINUSE              (__DANCY_ERRNO_OPEN_GROUP + 3)
#define EADDRNOTAVAIL           (__DANCY_ERRNO_OPEN_GROUP + 4)
#define EAFNOSUPPORT            (__DANCY_ERRNO_OPEN_GROUP + 5)
#define EAGAIN                  (__DANCY_ERRNO_OPEN_GROUP + 6)
#define EALREADY                (__DANCY_ERRNO_OPEN_GROUP + 7)
#define EBADF                   (__DANCY_ERRNO_OPEN_GROUP + 8)
#define EBADMSG                 (__DANCY_ERRNO_OPEN_GROUP + 9)
#define EBUSY                   (__DANCY_ERRNO_OPEN_GROUP + 10)
#define ECANCELED               (__DANCY_ERRNO_OPEN_GROUP + 11)
#define ECHILD                  (__DANCY_ERRNO_OPEN_GROUP + 12)
#define ECONNABORTED            (__DANCY_ERRNO_OPEN_GROUP + 13)
#define ECONNREFUSED            (__DANCY_ERRNO_OPEN_GROUP + 14)
#define ECONNRESET              (__DANCY_ERRNO_OPEN_GROUP + 15)
#define EDEADLK                 (__DANCY_ERRNO_OPEN_GROUP + 16)
#define EDESTADDRREQ            (__DANCY_ERRNO_OPEN_GROUP + 17)
#define EDQUOT                  (__DANCY_ERRNO_OPEN_GROUP + 18)
#define EEXIST                  (__DANCY_ERRNO_OPEN_GROUP + 19)
#define EFAULT                  (__DANCY_ERRNO_OPEN_GROUP + 20)
#define EFBIG                   (__DANCY_ERRNO_OPEN_GROUP + 21)
#define EHOSTUNREACH            (__DANCY_ERRNO_OPEN_GROUP + 22)
#define EIDRM                   (__DANCY_ERRNO_OPEN_GROUP + 23)
#define EINPROGRESS             (__DANCY_ERRNO_OPEN_GROUP + 24)
#define EINTR                   (__DANCY_ERRNO_OPEN_GROUP + 25)
#define EINVAL                  (__DANCY_ERRNO_OPEN_GROUP + 26)
#define EIO                     (__DANCY_ERRNO_OPEN_GROUP + 27)
#define EISCONN                 (__DANCY_ERRNO_OPEN_GROUP + 28)
#define EISDIR                  (__DANCY_ERRNO_OPEN_GROUP + 29)
#define ELOOP                   (__DANCY_ERRNO_OPEN_GROUP + 30)
#define EMFILE                  (__DANCY_ERRNO_OPEN_GROUP + 31)
#define EMLINK                  (__DANCY_ERRNO_OPEN_GROUP + 32)
#define EMSGSIZE                (__DANCY_ERRNO_OPEN_GROUP + 33)
#define EMULTIHOP               (__DANCY_ERRNO_OPEN_GROUP + 34)
#define ENAMETOOLONG            (__DANCY_ERRNO_OPEN_GROUP + 35)
#define ENETDOWN                (__DANCY_ERRNO_OPEN_GROUP + 36)
#define ENETRESET               (__DANCY_ERRNO_OPEN_GROUP + 37)
#define ENETUNREACH             (__DANCY_ERRNO_OPEN_GROUP + 38)
#define ENFILE                  (__DANCY_ERRNO_OPEN_GROUP + 39)
#define ENOBUFS                 (__DANCY_ERRNO_OPEN_GROUP + 40)
#define ENODATA                 (__DANCY_ERRNO_OPEN_GROUP + 41)
#define ENODEV                  (__DANCY_ERRNO_OPEN_GROUP + 42)
#define ENOENT                  (__DANCY_ERRNO_OPEN_GROUP + 43)
#define ENOEXEC                 (__DANCY_ERRNO_OPEN_GROUP + 44)
#define ENOLCK                  (__DANCY_ERRNO_OPEN_GROUP + 45)
#define ENOLINK                 (__DANCY_ERRNO_OPEN_GROUP + 46)
#define ENOMEM                  (__DANCY_ERRNO_OPEN_GROUP + 47)
#define ENOMSG                  (__DANCY_ERRNO_OPEN_GROUP + 48)
#define ENOPROTOOPT             (__DANCY_ERRNO_OPEN_GROUP + 49)
#define ENOSPC                  (__DANCY_ERRNO_OPEN_GROUP + 50)
#define ENOSR                   (__DANCY_ERRNO_OPEN_GROUP + 51)
#define ENOSTR                  (__DANCY_ERRNO_OPEN_GROUP + 52)
#define ENOSYS                  (__DANCY_ERRNO_OPEN_GROUP + 53)
#define ENOTCONN                (__DANCY_ERRNO_OPEN_GROUP + 54)
#define ENOTDIR                 (__DANCY_ERRNO_OPEN_GROUP + 55)
#define ENOTEMPTY               (__DANCY_ERRNO_OPEN_GROUP + 56)
#define ENOTRECOVERABLE         (__DANCY_ERRNO_OPEN_GROUP + 57)
#define ENOTSOCK                (__DANCY_ERRNO_OPEN_GROUP + 58)
#define ENOTSUP                 (__DANCY_ERRNO_OPEN_GROUP + 59)
#define ENOTTY                  (__DANCY_ERRNO_OPEN_GROUP + 60)
#define ENXIO                   (__DANCY_ERRNO_OPEN_GROUP + 61)
#define EOPNOTSUPP              (__DANCY_ERRNO_OPEN_GROUP + 62)
#define EOVERFLOW               (__DANCY_ERRNO_OPEN_GROUP + 63)
#define EOWNERDEAD              (__DANCY_ERRNO_OPEN_GROUP + 64)
#define EPERM                   (__DANCY_ERRNO_OPEN_GROUP + 65)
#define EPIPE                   (__DANCY_ERRNO_OPEN_GROUP + 66)
#define EPROTO                  (__DANCY_ERRNO_OPEN_GROUP + 67)
#define EPROTONOSUPPORT         (__DANCY_ERRNO_OPEN_GROUP + 68)
#define EPROTOTYPE              (__DANCY_ERRNO_OPEN_GROUP + 69)
#define EROFS                   (__DANCY_ERRNO_OPEN_GROUP + 70)
#define ESPIPE                  (__DANCY_ERRNO_OPEN_GROUP + 71)
#define ESRCH                   (__DANCY_ERRNO_OPEN_GROUP + 72)
#define ESTALE                  (__DANCY_ERRNO_OPEN_GROUP + 73)
#define ETIME                   (__DANCY_ERRNO_OPEN_GROUP + 74)
#define ETIMEDOUT               (__DANCY_ERRNO_OPEN_GROUP + 75)
#define ETXTBSY                 (__DANCY_ERRNO_OPEN_GROUP + 76)
#define EWOULDBLOCK             (__DANCY_ERRNO_OPEN_GROUP + 77)
#define EXDEV                   (__DANCY_ERRNO_OPEN_GROUP + 78)

#endif
