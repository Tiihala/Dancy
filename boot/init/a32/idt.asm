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
;; init/a32/idt.asm
;;      Interrupt Descriptor Table
;;

        bits 32

section .text

        extern _idt_handler
        global _idt_init
        global _idt_load_null
        global _idt_restore

align 16
        ; void idt_init(void)
_idt_init:
        push ebx                        ; save register ebx
        push edi                        ; save register edi

        mov ebx, idt_handlers           ; ebx = address of idt_handlers

        ; Generate code for each interrupt handler
        ;
        ;   push 0                      ; push error code
        ;   push eax                    ; save register eax
        ;   mov eax, idt_common         ; eax = address of idt_common
        ;   push ecx                    ; save register ecx
        ;   mov ecx, <interrupt>        ; ecx = interrupt number
        ;   jmp eax                     ; jmp idt_common
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

.L1:    mov dword [ebx+0], 0xB850006A   ; write instructions (1/4)
        mov dword [ebx+4], eax          ; write instructions (2/4)
        mov dword [ebx+8], edx          ; write instructions (3/4)
        mov dword [ebx+12], 0xE0FF0000  ; write instructions (4/4)

        add ebx, 16                     ; next handler
        add edx, 0x00010000             ; modify "mov ecx, <interrupt>"
        dec ecx                         ; decrement counter
        jnz short .L1

        mov eax, 0xB8509090             ; eax = instructions (1/4, "nops")
        mov ebx, idt_handlers           ; ebx = address of idt_handlers
        mov [ebx+(8*16)], eax           ; double fault exception
        mov [ebx+(10*16)], eax          ; invalid tss exception
        mov [ebx+(11*16)], eax          ; segment not present exception
        mov [ebx+(12*16)], eax          ; stack exception
        mov [ebx+(13*16)], eax          ; general protection exception
        mov [ebx+(14*16)], eax          ; page fault exception
        mov [ebx+(17*16)], eax          ; alignment check exception
        mov [ebx+(30*16)], eax          ; security exception

        ; Install interrupt descriptors

        mov ecx, 256                    ; ecx = number of interrupts
        mov edi, idt_table              ; edi = address of idt_table

.L2:    mov eax, ebx                    ; eax = address of idt_handler
        and eax, 0x0000FFFF             ; eax = address bits 0-15
        or eax, 0x00080000              ; set target code segment selector
        mov edx, ebx                    ; edx = address of idt_handler
        and edx, 0xFFFF0000             ; edx = address bits 16-31
        or edx, 0x00008E00              ; set 32-bit interrupt gate

        mov [edi+0], eax                ; write descriptor (1/2)
        mov [edi+4], edx                ; write descriptor (2/2)

        add edi, 8                      ; next handler
        add ebx, 16                     ; next descriptor
        dec ecx                         ; decrement counter
        jnz short .L2

        ; Load the interrupt descriptor table register

        mov ebx, idt_pointer            ; ebx = address of idt_pointer
        lidt [ebx]                      ; load the register

        pop edi                         ; restore register edi
        pop ebx                         ; restore register ebx
        ret

align 16
idt_common:
        push edx                        ; save register edx
        push ebx                        ; save register ebx
        mov ebx, esp                    ; save stack
        cld                             ; clear direction flag

        ;   [esp+20]   instruction pointer
        ;   [esp+16]   error code
        ;   [esp+12]   register eax
        ;   [esp+8]    register ecx
        ;   [esp+4]    register edx
        ;   [esp+0]    register ebx

        and esp, 0xFFFFFFF0             ; align stack
        lea edx, [ebx+20]               ; edx = stack at "instruction pointer"

        ; void idt_handler(unsigned num, const void *stack)
        ;
        ;   num        interrupt number
        ;   stack      stack at "instruction pointer"

        push 0                          ; push 0
        push 0                          ; push 0
        push edx                        ; push "stack"
        push ecx                        ; push "num"
        call _idt_handler               ; call idt_handler

        mov esp, ebx                    ; restore stack
        pop ebx                         ; restore register ebx
        pop edx                         ; restore register edx
        pop ecx                         ; restore register ecx
        pop eax                         ; restore register eax
        add esp, 4                      ; remove error code
        iret

align 16
        ; void idt_load_null(void)
_idt_load_null:
        mov eax, (idt_pointer_null + 6) ; eax = address of "null pointer"
        lidt [eax]                      ; load the register
        ret

align 16
        ; void idt_restore(void)
_idt_restore:
        mov eax, idt_pointer            ; eax = address of idt_pointer
        lidt [eax]                      ; load the register
        ret


section .data

align 16
        dw 0, 0, 0
idt_pointer:
        dw 2047
        dd idt_table
        dd 0


section .bss align=64

idt_table:
        resb 2048

idt_handlers:
        resb 4096

idt_pointer_null:
        resb 16
