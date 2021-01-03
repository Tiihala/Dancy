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
;; init/a32/kernel.asm
;;      Kernel initialization
;;

        bits 32

section .text

        global _kernel_jump

align 16
        ; void kernel_jump(void (*func)(void), void *stack)
_kernel_jump:
        mov ecx, [esp+4]                ; ecx = func
        mov esp, [esp+8]                ; esp = stack

        xor eax, eax                    ; eax = 0
        xor edx, edx                    ; edx = 0
        xor ebx, ebx                    ; ebx = 0
        xor ebp, ebp                    ; ebp = 0
        xor esi, esi                    ; esi = 0
        xor edi, edi                    ; edi = 0

        jmp ecx                         ; jump to the function
