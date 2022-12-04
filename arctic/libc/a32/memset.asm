;;
;; Copyright (c) 2022 Antti Tiihala
;;
;; Permission to use, copy, modify, and/or distribute this software for any
;; purpose with or without fee is hereby granted, provided that the above
;; copyright notice and this permission notice appear in all copies.
;;
;; THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
;; WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
;; MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
;; ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
;; WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
;; ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
;; OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
;;
;; libc/a32/memset.asm
;;      The C Standard Library
;;

        bits 32

section .text

        global _memset

align 16
        ; void *memset(void *s, int c, size_t n)
_memset:
        push edi                        ; save register edi
        mov edi, [esp+8]                ; edi = s
        mov eax, [esp+12]               ; eax = c (low byte)
        mov ecx, [esp+16]               ; ecx = n
        cld                             ; (extra safety)
        rep stosb                       ; copy the low byte
        mov eax, [esp+8]                ; eax = s
        pop edi                         ; restore register edi
        ret
