;;
;; Copyright (c) 2021 Antti Tiihala
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
;; init/a64/kernel.asm
;;      Kernel initialization
;;

        bits 64

section .text

        global kernel_jump

align 16
        ; void kernel_jump(void (*func)(void), void *stack)
kernel_jump:
        mov rsp, rdx                    ; rsp = stack

        xor eax, eax                    ; rax = 0
        xor edx, edx                    ; rdx = 0
        xor ebx, ebx                    ; rbx = 0
        xor ebp, ebp                    ; rbp = 0
        xor esi, esi                    ; rsi = 0
        xor edi, edi                    ; rdi = 0

        xor r8, r8                      ; r8 = 0
        xor r9, r9                      ; r9 = 0
        xor r10, r10                    ; r10 = 0
        xor r11, r11                    ; r11 = 0
        xor r12, r12                    ; r12 = 0
        xor r13, r13                    ; r13 = 0
        xor r14, r14                    ; r14 = 0
        xor r15, r15                    ; r15 = 0

        jmp rcx                         ; jump to the function
