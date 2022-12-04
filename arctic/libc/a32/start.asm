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
;; libc/a32/start.asm
;;      Start function of the C library
;;

        bits 32

section .text

        extern ___dancy_libc_start
        global ___start

align 16

___start:
        ; nop                           ; [esp + 4]  = argc
        ; nop                           ; [esp + 8]  = argv
        ; nop                           ; [esp + 12] = envp
        mov eax, _environ               ; eax = address of _environ
        mov edx, [esp+12]               ; edx = envp
        mov [eax], edx                  ; set the _environ variable
        jmp near ___dancy_libc_start    ; jump to the libc start function

section .data

        global _environ

align 4
        ; char **environ
_environ: dd 0
