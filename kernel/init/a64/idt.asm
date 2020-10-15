;;
;; Copyright (c) 2020 Antti Tiihala
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
;; init/a64/idt.asm
;;      Interrupt Descriptor Table
;;

        bits 64

section .text

        extern idt_handler
        global idt_init
        global idt_load_null
        global idt_restore

align 16
        ; void idt_init(void)
idt_init:
        push rbx                        ; save register rbx
        push rdi                        ; save register rdi

        mov ebx, idt_handlers           ; ebx = address of idt_handlers

        ; Generate code for each interrupt handler
        ;
        ;   push 0                      ; push error code
        ;   push rax                    ; save register rax
        ;   mov eax, idt_common         ; eax = address of idt_common
        ;   push rcx                    ; save register rcx
        ;   mov ecx, <interrupt>        ; ecx = interrupt number
        ;   jmp rax                     ; jmp idt_common
        ;
        ; The first instruction "push 0" will be replaced with
        ; two "nop" instructions for exceptions that push the
        ; error code, e.g. General Protection Exception.
        ;
        ; Normally code in .bss section cannot be executed, but
        ; the pre-kernel environment allows that.

        mov eax, idt_common             ; eax = address of idt_common
        mov ecx, 256                    ; ecx = number of interrupts
        mov edx, 0x0000B951             ; edx = instructions (3/4)

.L1:    mov dword [rbx+0], 0xB850006A   ; write instructions (1/4)
        mov dword [rbx+4], eax          ; write instructions (2/4)
        mov dword [rbx+8], edx          ; write instructions (3/4)
        mov dword [rbx+12], 0xE0FF0000  ; write instructions (4/4)

        add ebx, 16                     ; next handler
        add edx, 0x00010000             ; modify "mov ecx, <interrupt>"
        dec ecx                         ; decrement counter
        jnz short .L1

        mov eax, 0xB8509090             ; eax = instructions (1/4, "nops")
        mov ebx, idt_handlers           ; ebx = address of idt_handlers
        mov [rbx+(8*16)], eax           ; double fault exception
        mov [rbx+(10*16)], eax          ; invalid tss exception
        mov [rbx+(11*16)], eax          ; segment not present exception
        mov [rbx+(12*16)], eax          ; stack exception
        mov [rbx+(13*16)], eax          ; general protection exception
        mov [rbx+(14*16)], eax          ; page fault exception
        mov [rbx+(17*16)], eax          ; alignment check exception
        mov [rbx+(30*16)], eax          ; security exception

        ; Install interrupt descriptors

        mov ecx, 256                    ; ecx = number of interrupts
        mov edi, idt_table              ; edi = address of idt_table

.L2:    mov eax, ebx                    ; eax = address of idt_handler
        and eax, 0x0000FFFF             ; eax = address bits 0-15
        or eax, 0x00080000              ; set target code segment selector
        mov edx, ebx                    ; edx = address of idt_handler
        and edx, 0xFFFF0000             ; edx = address bits 16-31
        or edx, 0x00008E00              ; set 64-bit interrupt gate

        mov [rdi+0], eax                ; write descriptor (1/2)
        mov [rdi+4], edx                ; write descriptor (2/2)

        add edi, 16                     ; next handler
        add ebx, 16                     ; next descriptor
        dec ecx                         ; decrement counter
        jnz short .L2

        ; Load the interrupt descriptor table register

        mov ebx, idt_pointer            ; ebx = address of idt_pointer
        lidt [rbx]                      ; load the register

        pop rdi                         ; restore register rdi
        pop rbx                         ; restore register rbx
        ret

align 16
idt_common:
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

        and rsp, 0xFFFFFFE0             ; align stack
        sub rsp, 32                     ; shadow space

        ; void idt_handler(unsigned num, unsigned err_code, const void *stack)
        ;
        ;   num        interrupt number (register rcx)
        ;   err_code   error code
        ;   stack      stack at "instruction pointer"

        mov rdx, [rbx+64]               ; rdx = error code
        lea r8, [rbx+72]                ; r8 = stack at "instruction pointer"
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
        ; void idt_load_null(void)
idt_load_null:
        mov eax, (idt_pointer_null + 6) ; eax = address of "null pointer"
        lidt [rax]                      ; load the register
        ret

align 16
        ; void idt_restore(void)
idt_restore:
        mov eax, idt_pointer            ; eax = address of idt_pointer
        lidt [rax]                      ; load the register
        ret


section .data

align 16
        dw 0, 0, 0
idt_pointer:
        dw 4095
        dd idt_table
        dd 0


section .bss align=64

idt_table:
        resb 4096

idt_handlers:
        resb 4096

idt_pointer_null:
        resb 16
