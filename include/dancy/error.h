/*
 * Copyright (c) 2020, 2021, 2022 Antti Tiihala
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
 * dancy/error.h
 *      Header of Dancy Operating System
 */

#ifdef DANCY_X

/*
 * Value of DE_SUCCESS must be zero.
 */
DANCY_X(DE_SUCCESS                   , "success"                         )

DANCY_X(DE_UNEXPECTED                , "unexpected behavior"             )
DANCY_X(DE_MEMORY                    , "not enough memory"               )
DANCY_X(DE_CHECKSUM                  , "checksum does not match"         )
DANCY_X(DE_ARGUMENT                  , "function argument"               )
DANCY_X(DE_RETRY                     , "resource unavailable"            )
DANCY_X(DE_UNINITIALIZED             , "uninitialized"                   )
DANCY_X(DE_UNSUPPORTED               , "unsupported"                     )
DANCY_X(DE_PLACEHOLDER               , "placeholder data"                )
DANCY_X(DE_BUFFER                    , "buffer size too small"           )
DANCY_X(DE_NAME                      , "name not found"                  )
DANCY_X(DE_PATH                      , "erroneous path"                  )
DANCY_X(DE_OVERFLOW                  , "resource overflowed"             )
DANCY_X(DE_BLOCK_READ                , "block read failure"              )
DANCY_X(DE_BLOCK_WRITE               , "block write failure"             )
DANCY_X(DE_READ                      , "read failure"                    )
DANCY_X(DE_WRITE                     , "write failure"                   )
DANCY_X(DE_SEEK                      , "seek failure"                    )
DANCY_X(DE_FULL                      , "resource is full"                )
DANCY_X(DE_READ_ONLY                 , "read-only"                       )
DANCY_X(DE_BUSY                      , "resource is in use"              )
DANCY_X(DE_TYPE                      , "unexpected resource type"        )

#endif

/*
 * The maximum size of an error message is 31 chars plus the terminating NUL.
 */
#ifndef DANCY_ERROR_MESSAGE_SIZE
#define DANCY_ERROR_MESSAGE_SIZE 32
#endif
