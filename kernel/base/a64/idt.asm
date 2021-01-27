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
;; base/a64/idt.asm
;;      Interrupt Descriptor Table
;;

        bits 64

section .text

        extern idt_handler
        global idt_asm_array
        global idt_asm_handler
        global idt_load

align 64
        ; uint8_t idt_asm_array[256][16]
idt_asm_array:
        times 4096 db 0xCC

        ; const uint8_t idt_asm_handler[]
idt_asm_handler:
        push rdx                        ; save register rdx
        push rbx                        ; save register rbx
        push r8                         ; save register r8
        push r9                         ; save register r9
        push r10                        ; save register r10
        push r11                        ; save register r11
        mov rbx, rsp                    ; save stack
        cld                             ; clear direction flag

        ;   [rsp+72]   instruction pointer
        ;   [rsp+64]   error code
        ;   [rsp+56]   register rax
        ;   [rsp+48]   register rcx
        ;   [rsp+40]   register rdx
        ;   [rsp+32]   register rbx
        ;   [rsp+24]   register r8
        ;   [rsp+16]   register r9
        ;   [rsp+8]    register r10
        ;   [rsp+0]    register r11

        and rsp, -16                    ; align stack
        sub rsp, 32                     ; shadow space

        ; void idt_handler(int num, void *stack)
        ;
        ;   num        interrupt number (register rcx)
        ;   stack      stack at "instruction pointer"

        lea rdx, [rbx+72]               ; rdx = stack at "instruction pointer"
        call idt_handler                ; call idt_handler

        mov rsp, rbx                    ; restore stack
        pop r11                         ; restore register r11
        pop r10                         ; restore register r10
        pop r9                          ; restore register r9
        pop r8                          ; restore register r8
        pop rbx                         ; restore register rbx
        pop rdx                         ; restore register rdx
        pop rcx                         ; restore register rcx
        pop rax                         ; restore register rax
        add rsp, 8                      ; remove error code
        iretq

align 16
        ; void idt_load(const void *idt_ptr)
idt_load:
        lidt [rcx]                      ; load interrupt descriptor table
        ret
