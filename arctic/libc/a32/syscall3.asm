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
;; libc/a32/syscall3.asm
;;      The system call function
;;

        bits 32

section .text

        global ___dancy_syscall3

align 16
        ; long long __dancy_syscall3(int arg0, ...)
___dancy_syscall3:
        push ebx                        ; save register ebx

        mov eax, [esp+8]                ; eax = arg0
        mov ecx, [esp+12]               ; ecx = arg1
        mov edx, [esp+16]               ; edx = arg2
        mov ebx, [esp+20]               ; ebx = arg3
        int 0x64                        ; call the kernel

        pop ebx                         ; restore register ebx
        ret
