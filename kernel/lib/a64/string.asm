;;
;; Copyright (c) 2018 Antti Tiihala
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
;; lib/a64/string.asm
;;      C standard library functions
;;

        bits 64

section .text

        global memset

align 16
        ; void *memset(void *s, int c, size_t n)
memset:
        push rdi                        ; save register rdi
        mov rdx, rcx                    ; rdx = s (return value)
        mov rdi, rcx                    ; rdi = s
        mov eax, edx                    ; eax = c (low byte)
        mov rcx, r8                     ; rcx = n
        rep stosb                       ; copy the low byte
        mov rax, rdx                    ; rax = s
        pop rdi                         ; restore register rdi
        ret
