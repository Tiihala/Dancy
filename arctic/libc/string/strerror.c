/*
 * Copyright (c) 2023 Antti Tiihala
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
 * libc/string/strerror.c
 *      The C Standard Library
 */

#include <errno.h>
#include <string.h>

#define errnum_switch_entry(errnum) case errnum : r = #errnum ; break

char *strerror(int errnum)
{
	const char *r;

	if (errnum == EDOM)
		return (char *)("EDOM");
	if (errnum == EILSEQ)
		return (char *)("EILSEQ");
	if (errnum == ERANGE)
		return (char *)("ERANGE");

	switch (errnum) {
		errnum_switch_entry(E2BIG);
		errnum_switch_entry(EACCES);
		errnum_switch_entry(EADDRINUSE);
		errnum_switch_entry(EADDRNOTAVAIL);
		errnum_switch_entry(EAFNOSUPPORT);
		errnum_switch_entry(EAGAIN);
		errnum_switch_entry(EALREADY);
		errnum_switch_entry(EBADF);
		errnum_switch_entry(EBADMSG);
		errnum_switch_entry(EBUSY);
		errnum_switch_entry(ECANCELED);
		errnum_switch_entry(ECHILD);
		errnum_switch_entry(ECONNABORTED);
		errnum_switch_entry(ECONNREFUSED);
		errnum_switch_entry(ECONNRESET);
		errnum_switch_entry(EDEADLK);
		errnum_switch_entry(EDESTADDRREQ);
		errnum_switch_entry(EDQUOT);
		errnum_switch_entry(EEXIST);
		errnum_switch_entry(EFAULT);
		errnum_switch_entry(EFBIG);
		errnum_switch_entry(EHOSTUNREACH);
		errnum_switch_entry(EIDRM);
		errnum_switch_entry(EINPROGRESS);
		errnum_switch_entry(EINTR);
		errnum_switch_entry(EINVAL);
		errnum_switch_entry(EIO);
		errnum_switch_entry(EISCONN);
		errnum_switch_entry(EISDIR);
		errnum_switch_entry(ELOOP);
		errnum_switch_entry(EMFILE);
		errnum_switch_entry(EMLINK);
		errnum_switch_entry(EMSGSIZE);
		errnum_switch_entry(EMULTIHOP);
		errnum_switch_entry(ENAMETOOLONG);
		errnum_switch_entry(ENETDOWN);
		errnum_switch_entry(ENETRESET);
		errnum_switch_entry(ENETUNREACH);
		errnum_switch_entry(ENFILE);
		errnum_switch_entry(ENOBUFS);
		errnum_switch_entry(ENODATA);
		errnum_switch_entry(ENODEV);
		errnum_switch_entry(ENOENT);
		errnum_switch_entry(ENOEXEC);
		errnum_switch_entry(ENOLCK);
		errnum_switch_entry(ENOLINK);
		errnum_switch_entry(ENOMEM);
		errnum_switch_entry(ENOMSG);
		errnum_switch_entry(ENOPROTOOPT);
		errnum_switch_entry(ENOSPC);
		errnum_switch_entry(ENOSR);
		errnum_switch_entry(ENOSTR);
		errnum_switch_entry(ENOSYS);
		errnum_switch_entry(ENOTCONN);
		errnum_switch_entry(ENOTDIR);
		errnum_switch_entry(ENOTEMPTY);
		errnum_switch_entry(ENOTRECOVERABLE);
		errnum_switch_entry(ENOTSOCK);
		errnum_switch_entry(ENOTSUP);
		errnum_switch_entry(ENOTTY);
		errnum_switch_entry(ENXIO);
		errnum_switch_entry(EOPNOTSUPP);
		errnum_switch_entry(EOVERFLOW);
		errnum_switch_entry(EOWNERDEAD);
		errnum_switch_entry(EPERM);
		errnum_switch_entry(EPIPE);
		errnum_switch_entry(EPROTO);
		errnum_switch_entry(EPROTONOSUPPORT);
		errnum_switch_entry(EPROTOTYPE);
		errnum_switch_entry(EROFS);
		errnum_switch_entry(ESPIPE);
		errnum_switch_entry(ESRCH);
		errnum_switch_entry(ESTALE);
		errnum_switch_entry(ETIME);
		errnum_switch_entry(ETIMEDOUT);
		errnum_switch_entry(ETXTBSY);
		errnum_switch_entry(EWOULDBLOCK);
		errnum_switch_entry(EXDEV);

		default:
			r = "**** unknown error ****";
			break;
	}

	return (char *)r;
}
