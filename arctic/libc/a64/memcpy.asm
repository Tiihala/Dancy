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
;; libc/a64/memcpy.asm
;;      The C Standard Library
;;

        bits 64

section .text

        global memcpy

align 16
        ; void *memcpy(void *s1, const void *s2, size_t n)
memcpy:
        push rsi                        ; save register rsi
        push rdi                        ; save register rdi
        mov rdi, rcx                    ; rdi = s1
        mov rsi, rdx                    ; rsi = s2
        mov rdx, rcx                    ; rdx = s1 (return value)
        mov rcx, r8                     ; rcx = n
        cld                             ; (extra safety)
        rep movsb                       ; copy bytes
        mov rax, rdx                    ; rax = s1
        pop rdi                         ; restore register rdi
        pop rsi                         ; restore register rsi
        ret
