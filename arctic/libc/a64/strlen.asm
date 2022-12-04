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
;; libc/a64/strlen.asm
;;      The C Standard Library
;;

        bits 64

section .text

        global strlen


align 16
        ; size_t strlen(const char *s)
strlen:
        push rdi                        ; save register rdi
        mov rdi, rcx                    ; rdi = s
        xor eax, eax                    ; rax = 0
        lea rcx, [rax-1]                ; rcx = SIZE_MAX
        cld                             ; (extra safety)
        repne scasb                     ; find the zero byte
        not rcx                         ; reverse each bit of rcx
        lea rax, [rcx-1]                ; rax = length of the string
        pop rdi                         ; restore register rdi
        ret
