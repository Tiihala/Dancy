/*
 * Copyright (c) 2025 Antti Tiihala
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
 * syscall/errno.c
 *      Translate errno to message strings
 */

#include <dancy.h>
#include <errno.h>

static struct { int e; const char *name; const char *description; } data[] = {

{ EDOM             , "EDOM"            ,  NULL                              },
{ EILSEQ           , "EILSEQ"          ,  NULL                              },
{ ERANGE           , "ERANGE"          ,  NULL                              },
{ E2BIG            , "E2BIG"           ,  NULL                              },
{ EACCES           , "EACCES"          , "Permission denied"                },
{ EADDRINUSE       , "EADDRINUSE"      ,  NULL                              },
{ EADDRNOTAVAIL    , "EADDRNOTAVAIL"   ,  NULL                              },
{ EAFNOSUPPORT     , "EAFNOSUPPORT"    ,  NULL                              },
{ EAGAIN           , "EAGAIN"          , "Resource temporarily unavailable" },
{ EALREADY         , "EALREADY"        ,  NULL                              },
{ EBADF            , "EBADF"           , "Bad file descriptor"              },
{ EBADMSG          , "EBADMSG"         ,  NULL                              },
{ EBUSY            , "EBUSY"           , "Resource busy"                    },
{ ECANCELED        , "ECANCELED"       ,  NULL                              },
{ ECHILD           , "ECHILD"          , "No child processes"               },
{ ECONNABORTED     , "ECONNABORTED"    ,  NULL                              },
{ ECONNREFUSED     , "ECONNREFUSED"    ,  NULL                              },
{ ECONNRESET       , "ECONNRESET"      ,  NULL                              },
{ EDEADLK          , "EDEADLK"         ,  NULL                              },
{ EDESTADDRREQ     , "EDESTADDRREQ"    ,  NULL                              },
{ EDQUOT           , "EDQUOT"          ,  NULL                              },
{ EEXIST           , "EEXIST"          , "Resource exists"                  },
{ EFAULT           , "EFAULT"          , "Bad address"                      },
{ EFBIG            , "EFBIG"           ,  NULL                              },
{ EHOSTUNREACH     , "EHOSTUNREACH"    ,  NULL                              },
{ EIDRM            , "EIDRM"           ,  NULL                              },
{ EINPROGRESS      , "EINPROGRESS"     ,  NULL                              },
{ EINTR            , "EINTR"           , "Interrupted system call"          },
{ EINVAL           , "EINVAL"          , "Invalid argument"                 },
{ EIO              , "EIO"             , "Input/output error"               },
{ EISCONN          , "EISCONN"         ,  NULL                              },
{ EISDIR           , "EISDIR"          , "Is a directory"                   },
{ ELOOP            , "ELOOP"           ,  NULL                              },
{ EMFILE           , "EMFILE"          , "Too many open files"              },
{ EMLINK           , "EMLINK"          ,  NULL                              },
{ EMSGSIZE         , "EMSGSIZE"        ,  NULL                              },
{ EMULTIHOP        , "EMULTIHOP"       ,  NULL                              },
{ ENAMETOOLONG     , "ENAMETOOLONG"    , "Resource name too long"           },
{ ENETDOWN         , "ENETDOWN"        ,  NULL                              },
{ ENETRESET        , "ENETRESET"       ,  NULL                              },
{ ENETUNREACH      , "ENETUNREACH"     ,  NULL                              },
{ ENFILE           , "ENFILE"          ,  NULL                              },
{ ENOBUFS          , "ENOBUFS"         ,  NULL                              },
{ ENODATA          , "ENODATA"         , "No data available"                },
{ ENODEV           , "ENODEV"          , "No such device"                   },
{ ENOENT           , "ENOENT"          , "No such file or directory"        },
{ ENOEXEC          , "ENOEXEC"         , "Exec format not supported"        },
{ ENOLCK           , "ENOLCK"          ,  NULL                              },
{ ENOLINK          , "ENOLINK"         ,  NULL                              },
{ ENOMEM           , "ENOMEM"          , "Cannot allocate memory"           },
{ ENOMSG           , "ENOMSG"          ,  NULL                              },
{ ENOPROTOOPT      , "ENOPROTOOPT"     ,  NULL                              },
{ ENOSPC           , "ENOSPC"          , "No space left"                    },
{ ENOSR            , "ENOSR"           ,  NULL                              },
{ ENOSTR           , "ENOSTR"          ,  NULL                              },
{ ENOSYS           , "ENOSYS"          , "Feature not implemented"          },
{ ENOTCONN         , "ENOTCONN"        ,  NULL                              },
{ ENOTDIR          , "ENOTDIR"         , "Not a directory"                  },
{ ENOTEMPTY        , "ENOTEMPTY"       , "Directory not empty"              },
{ ENOTRECOVERABLE  , "ENOTRECOVERABLE" ,  NULL                              },
{ ENOTSOCK         , "ENOTSOCK"        ,  NULL                              },
{ ENOTSUP          , "ENOTSUP"         ,  NULL                              },
{ ENOTTY           , "ENOTTY"          , "Inappropriate ioctl"              },
{ ENXIO            , "ENXIO"           ,  NULL                              },
{ EOPNOTSUPP       , "EOPNOTSUPP"      ,  NULL                              },
{ EOVERFLOW        , "EOVERFLOW"       , "Overflow"                         },
{ EOWNERDEAD       , "EOWNERDEAD"      ,  NULL                              },
{ EPERM            , "EPERM"           , "Operation not permitted"          },
{ EPIPE            , "EPIPE"           , "Pipe error"                       },
{ EPROTO           , "EPROTO"          ,  NULL                              },
{ EPROTONOSUPPORT  , "EPROTONOSUPPORT" ,  NULL                              },
{ EPROTOTYPE       , "EPROTOTYPE"      ,  NULL                              },
{ EROFS            , "EROFS"           ,  NULL                              },
{ ESPIPE           , "ESPIPE"          ,  NULL                              },
{ ESRCH            , "ESRCH"           ,  NULL                              },
{ ESTALE           , "ESTALE"          ,  NULL                              },
{ ETIME            , "ETIME"           ,  NULL                              },
{ ETIMEDOUT        , "ETIMEDOUT"       ,  NULL                              },
{ ETXTBSY          , "ETXTBSY"         , "File busy"                        },
{ EWOULDBLOCK      , "EWOULDBLOCK"     , "Resource temporarily unavailable" },
{ EXDEV            , "EXDEV"           , "Invalid cross-device link"        },

{ 0                , "SUCCESS"         , "Success"                         }};

void errno_internal(int number, char buffer[64], int flags)
{
	int i, count = (int)(sizeof(data) / sizeof(data[0]));

	for (i = 0; i < count; i++) {
		if (data[i].e == number) {
			if (flags == 0) {
				strcpy(&buffer[0], data[i].name);
				return;
			}
			if (flags == 1 && data[i].description == NULL) {
				strcpy(&buffer[0], data[i].name);
				return;
			}
			if (flags == 1 && data[i].description != NULL) {
				strcpy(&buffer[0], data[i].description);
				return;
			}
		}
	}

	snprintf(&buffer[0], 64, "Unknown error %d", number);
}
