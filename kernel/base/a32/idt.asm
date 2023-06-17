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
;; base/a32/idt.asm
;;      Interrupt Descriptor Table
;;

        bits 32

section .text

        extern _ret_user_handler
        extern _idt_handler
        extern _idt_panic
        global _idt_asm_array
        global _idt_asm_gp_handler
        global _idt_asm_handler
        global _idt_asm_panic
        global _idt_load

align 64
        ; uint8_t idt_asm_array[256][16]
_idt_asm_array:
        times 4096 db 0xCC

align 16
        ; const uint8_t idt_asm_gp_handler[]
_idt_asm_gp_handler:
        mov eax, 3                      ; eax = kernel check bits

        ;   [esp+16]   segment register cs
        ;   [esp+12]   instruction pointer
        ;   [esp+8]    error code
        ;   [esp+4]    register eax
        ;   [esp+0]    register ecx

        and eax, [esp+16]               ; detect kernel code
        jnz short _idt_asm_handler      ; jump if not kernel code

        mov eax, ss                     ; eax = segment register ss
        mov ecx, es                     ; ecx = segment register es
        mov es, eax                     ; set valid segment value
        cmp eax, ecx                    ; test segment value
        jne short .end

        mov ecx, ds                     ; ecx = segment register ds
        mov ds, eax                     ; set valid segment value
        cmp eax, ecx                    ; test segment value
        jne short .end

        mov ecx, 13                     ; general-protection exception
        jmp short _idt_asm_handler

.end:   pop ecx                         ; restore register ecx
        pop eax                         ; restore register eax
        add esp, 4                      ; remove error code
        iret

align 64
        ; const uint8_t idt_asm_handler[]
_idt_asm_handler:
        push edx                        ; save register edx
        push ebx                        ; save register ebx
        push es                         ; save segment register es
        push ds                         ; save segment register ds
        mov ebx, esp                    ; save stack
        cld                             ; clear direction flag

        ;   [esp+28]   instruction pointer
        ;   [esp+24]   error code
        ;   [esp+20]   register eax
        ;   [esp+16]   register ecx
        ;   [esp+12]   register edx
        ;   [esp+8]    register ebx
        ;   [esp+4]    segment register es
        ;   [esp+0]    segment register ds

        and esp, 0xFFFFFFF0             ; align stack
        lea edx, [ebx+28]               ; edx = stack at "instruction pointer"

        ; void idt_handler(int num, void *stack)
        ;
        ;   num        interrupt number
        ;   stack      stack at "instruction pointer"

        push 0                          ; push 0
        push 0                          ; push 0
        push edx                        ; push "stack"
        push ecx                        ; push "num"
        call _idt_handler               ; call idt_handler

        mov ecx, esp                    ; ecx = stack pointer
        and ecx, 0xFFFFE000             ; ecx = address of current task
        cmp dword [ecx+28], 0           ; test task->asm_data3
        je short .L1

        lea edx, [ebx+28]               ; edx = address of iret stack
        mov eax, [edx+4]                ; eax = segment register cs
        and eax, 3                      ; eax = eax & 3
        cmp eax, 3                      ; test user space segment
        jne short .L1

        ; void ret_user_handler(struct task *current, void *stack)
        ;
        ;   current    current task structure
        ;   stack      stack at "instruction pointer"

        push 0                          ; push 0
        push 0                          ; push 0
        push edx                        ; push "stack"
        push ecx                        ; push "current"
        sti                             ; enable interrupts
        call _ret_user_handler          ; call _ret_user_handler

.L1:    ; nop
        mov esp, ebx                    ; restore stack
        pop ds                          ; restore segment register ds
        pop es                          ; restore segment register es
        pop ebx                         ; restore register ebx
        pop edx                         ; restore register edx
        pop ecx                         ; restore register ecx
        pop eax                         ; restore register eax
        add esp, 4                      ; remove error code
        iret

align 16
        ; const uint8_t idt_asm_panic[]
        ;
        ; void idt_panic(int num, void *stack, struct idt_context *context);
        ;
        ; struct idt_context {
        ;         cpu_native_t eax;
        ;         cpu_native_t ecx;
        ;         cpu_native_t edx;
        ;         cpu_native_t ebx;
        ;         cpu_native_t ebp;
        ;         cpu_native_t esi;
        ;         cpu_native_t edi;
        ;
        ;         cpu_native_t cr2;
        ;         cpu_native_t cr3;
        ; };
_idt_asm_panic:
        sub esp, 36                     ; allocate stack space

        mov [esp+0], eax                ; save register eax
        mov [esp+4], ecx                ; save register ecx
        mov [esp+8], edx                ; save register edx
        mov [esp+12], ebx               ; save register ebx
        mov [esp+16], ebp               ; save register ebp
        mov [esp+20], esi               ; save register esi
        mov [esp+24], edi               ; save register edi

        mov eax, cr2                    ; eax = cr2
        mov [esp+28], eax               ; save register cr2
        mov eax, cr3                    ; eax = cr3
        mov [esp+32], eax               ; save register cr3

        mov ebp, esp                    ; ebp = "context"
        and esp, 0xFFFFFFF0             ; align stack

        push 0                          ; push 0
        push ebp                        ; push context
        push 0                          ; push NULL
        push 0                          ; push 0
        call _idt_panic                 ; idt_panic(0, NULL, context)
.L1:    hlt                             ; halt instruction
        jmp short .L1

align 16
        ; void idt_load(const void *idt_ptr)
_idt_load:
        mov ecx, [esp+4]                ; ecx = idt_ptr
        lidt [ecx]                      ; load interrupt descriptor table
        ret
