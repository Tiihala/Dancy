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
;; libc/a64/syscall4.asm
;;      The system call function
;;

        bits 64

section .text

        global __dancy_syscall4

align 16
        ; long long __dancy_syscall4(int arg0, ...)
__dancy_syscall4:
        push rbx                        ; save register rbx
        push rsi                        ; save register rsi

        mov rax, rcx                    ; rax = arg0
        mov rcx, rdx                    ; rcx = arg1
        mov rdx, r8                     ; rdx = arg2
        mov rbx, r9                     ; rbx = arg3
        mov rsi, [rsp+56]               ; rsi = arg4
        int 0x64                        ; call the kernel

        pop rsi                         ; restore register rsi
        pop rbx                         ; restore register rbx
        ret
