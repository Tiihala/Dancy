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
;; libc/a32/strlen.asm
;;      The C Standard Library
;;

        bits 32

section .text

        global _strlen

align 16
        ; size_t strlen(const char *s)
_strlen:
        push edi                        ; save register edi
        mov edi, [esp+8]                ; edi = s
        xor eax, eax                    ; eax = 0
        lea ecx, [eax-1]                ; ecx = SIZE_MAX
        cld                             ; (extra safety)
        repne scasb                     ; find the zero byte
        not ecx                         ; reverse each bit of ecx
        lea eax, [ecx-1]                ; eax = length of the string
        pop edi                         ; restore register edi
        ret
