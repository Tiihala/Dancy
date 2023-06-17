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

        extern ret_user_handler
        extern idt_handler
        extern idt_panic
        global idt_asm_array
        global idt_asm_handler
        global idt_asm_panic
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

        mov rcx, rsp                    ; rcx = stack pointer
        and rcx, -8192                  ; rcx = address of current task
        cmp dword [rcx+28], 0           ; test task->asm_data3
        je short .L1

        lea rdx, [rbx+72]               ; rdx = address of iret stack
        mov rax, [rdx+8]                ; rax = segment register cs
        and eax, 3                      ; eax = eax & 3
        cmp eax, 3                      ; test user space segment
        jne short .L1

        ; void ret_user_handler(struct task *current, void *stack)
        ;
        ;   current    current task structure (register rcx)
        ;   stack      stack at "instruction pointer"

        sti                             ; enable interrupts
        call ret_user_handler           ; call ret_user_handler

.L1:    ; nop
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
        ; const uint8_t idt_asm_panic[]
        ;
        ; void idt_panic(int num, void *stack, struct idt_context *context);
        ;
        ; struct idt_context {
        ;         cpu_native_t rax;
        ;         cpu_native_t rcx;
        ;         cpu_native_t rdx;
        ;         cpu_native_t rbx;
        ;         cpu_native_t rbp;
        ;         cpu_native_t rsi;
        ;         cpu_native_t rdi;
        ;
        ;         cpu_native_t r8;
        ;         cpu_native_t r9;
        ;         cpu_native_t r10;
        ;         cpu_native_t r11;
        ;         cpu_native_t r12;
        ;         cpu_native_t r13;
        ;         cpu_native_t r14;
        ;         cpu_native_t r15;
        ;
        ;         cpu_native_t cr2;
        ;         cpu_native_t cr3;
        ; };
idt_asm_panic:
        sub rsp, 136                    ; allocate stack space

        mov [rsp+0], rax                ; save register rax
        mov [rsp+8], rcx                ; save register rcx
        mov [rsp+16], rdx               ; save register rdx
        mov [rsp+24], rbx               ; save register rbx
        mov [rsp+32], rbp               ; save register rbp
        mov [rsp+40], rsi               ; save register rsi
        mov [rsp+48], rdi               ; save register rdi

        mov [rsp+56], r8                ; save register r8
        mov [rsp+64], r9                ; save register r9
        mov [rsp+72], r10               ; save register r10
        mov [rsp+80], r11               ; save register r11
        mov [rsp+88], r12               ; save register r12
        mov [rsp+96], r13               ; save register r13
        mov [rsp+104], r14              ; save register r14
        mov [rsp+112], r15              ; save register r15

        mov rax, cr2                    ; rax = cr2
        mov [rsp+120], rax              ; save register cr2
        mov rax, cr3                    ; rax = cr3
        mov [rsp+128], rax              ; save register cr3

        xor ecx, ecx                    ; rcx = 0
        xor edx, edx                    ; rdx = "NULL"
        mov r8, rsp                     ; r8 = "context"
        and rsp, -16                    ; align stack
        sub rsp, 32                     ; shadow space
        call idt_panic                  ; idt_panic(0, NULL, context)
.L1:    hlt                             ; halt instruction
        jmp short .L1

align 16
        ; void idt_load(const void *idt_ptr)
idt_load:
        lidt [rcx]                      ; load interrupt descriptor table
        ret
