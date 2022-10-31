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
;; libc/a32/syscall4.asm
;;      The system call function
;;

        bits 32

section .text

        global ___dancy_syscall4

align 16
        ; long long __dancy_syscall4(int arg0, ...)
___dancy_syscall4:
        push ebx                        ; save register ebx
        push esi                        ; save register esi

        mov eax, [esp+12]               ; eax = arg0
        mov ecx, [esp+16]               ; ecx = arg1
        mov edx, [esp+20]               ; edx = arg2
        mov ebx, [esp+24]               ; ebx = arg3
        mov esi, [esp+28]               ; esi = arg4
        int 0x64                        ; call the kernel

        pop esi                         ; restore register esi
        pop ebx                         ; restore register ebx
        ret
