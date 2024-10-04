/*
 * Copyright (c) 2020-2024 Antti Tiihala
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
 * kernel/error.h
 *      Header of Dancy Operating System
 */

#ifdef DANCY_X

/*
 * Value of DE_SUCCESS must be zero.
 */
DANCY_X(DE_SUCCESS                   , "success"                         )

DANCY_X(DE_UNEXPECTED                , "unexpected behavior"             )
DANCY_X(DE_MEMORY                    , "not enough memory"               )
DANCY_X(DE_ACCESS                    , "permission denied"               )
DANCY_X(DE_CHECKSUM                  , "checksum does not match"         )
DANCY_X(DE_ARGUMENT                  , "function argument"               )
DANCY_X(DE_RETRY                     , "temporarily unavailable"         )
DANCY_X(DE_EMPTY                     , "no data available"               )
DANCY_X(DE_NOT_EMPTY                 , "resource not empty"              )
DANCY_X(DE_PIPE                      , "pipe failure"                    )
DANCY_X(DE_UNINITIALIZED             , "uninitialized"                   )
DANCY_X(DE_UNSUPPORTED               , "unsupported"                     )
DANCY_X(DE_PLACEHOLDER               , "placeholder data"                )
DANCY_X(DE_SEARCH                    , "resource not found"              )
DANCY_X(DE_INTERRUPT                 , "function interrupted"            )
DANCY_X(DE_BUFFER                    , "buffer size too small"           )
DANCY_X(DE_NAME                      , "name not found"                  )
DANCY_X(DE_PATH                      , "erroneous path"                  )
DANCY_X(DE_OVERFLOW                  , "resource overflowed"             )
DANCY_X(DE_SEQUENCE                  , "nonsequential access"            )
DANCY_X(DE_DIRECTORY                 , "directory not expected"          )
DANCY_X(DE_FILE                      , "file not expected"               )
DANCY_X(DE_BLOCK_READ                , "block read failure"              )
DANCY_X(DE_BLOCK_WRITE               , "block write failure"             )
DANCY_X(DE_READ                      , "read failure"                    )
DANCY_X(DE_WRITE                     , "write failure"                   )
DANCY_X(DE_SEEK                      , "seek failure"                    )
DANCY_X(DE_FULL                      , "resource is full"                )
DANCY_X(DE_READ_ONLY                 , "read-only"                       )
DANCY_X(DE_BUSY                      , "resource is in use"              )
DANCY_X(DE_TYPE                      , "unexpected resource type"        )
DANCY_X(DE_NULL                      , "null address"                    )
DANCY_X(DE_ADDRESS                   , "address not available"           )
DANCY_X(DE_ALIGNMENT                 , "unsupported alignment"           )
DANCY_X(DE_MEDIA_CHANGED             , "media changed"                   )
DANCY_X(DE_ADDRESS_MARK              , "missing address mark"            )
DANCY_X(DE_OVERRUN                   , "data overrun or underrun"        )
DANCY_X(DE_COFF_SIGNATURE            , "unsupported signature"           )
DANCY_X(DE_COFF_SIZE                 , "erroneous or unsupported size"   )
DANCY_X(DE_COFF_HEADER               , "incompatible object header"      )
DANCY_X(DE_COFF_SECTION              , "incompatible object sections"    )
DANCY_X(DE_COFF_FLAG                 , "incompatible object flags"       )
DANCY_X(DE_COFF_SYMBOL               , "incompatible object symbols"     )
DANCY_X(DE_COFF_EXTERNAL             , "unresolved external symbols"     )
DANCY_X(DE_COFF_RELOCATION           , "incompatible object relocations" )
DANCY_X(DE_COFF_START                , "missing valid start symbol"      )
DANCY_X(DE_COFF_FORMAT               , "unsupported or corrupted format" )

#endif

/*
 * The maximum size of an error message is 31 chars plus the terminating NUL.
 */
#ifndef DANCY_ERROR_MESSAGE_SIZE
#define DANCY_ERROR_MESSAGE_SIZE 32
#endif
