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
;; base/a64/timer.asm
;;      Kernel timer
;;

        bits 64

section .text

        extern ret_user_handler
        extern timer_handler
        extern timer_handler_ap
        global timer_apic_base
        global timer_asm_handler_apic
        global timer_asm_handler_apic_ap
        global timer_asm_handler_pic
        global timer_read

align 64
        ; const uint8_t timer_asm_handler_apic[]
timer_asm_handler_apic:
        push rax                        ; save register rax
        push rcx                        ; save register rcx
        push rdx                        ; save register rdx

        ; uint8_t timer_apic_base[8]
        db 0x48, 0xBA                   ; "mov rdx, [timer_apic_base]"
timer_apic_base: db 0,0,0,0,0,0,0,0
        mov dword [rdx+0xB0], 0         ; end of interrupt

        mov edx, timer_ticks            ; rdx = address of timer_ticks
        lock inc qword [rdx]            ; increment ticks

        mov edx, timer_ticks_wait       ; rdx = address of timer_ticks_wait
.L1:    mov eax, [rdx]                  ; read current value
        test eax, eax                   ; test zero
        jz short timer_call_handler

        lea rcx, [rax-1]                ; ecx = decremented wait value
        lock cmpxchg [rdx], ecx         ; try to decrement
        jnz short .L1                   ; retry if needed

        pop rdx                         ; restore register rdx
        pop rcx                         ; restore register rcx
        pop rax                         ; restore register rax
        iretq

align 64
        ; const uint8_t timer_asm_handler_pic[]
timer_asm_handler_pic:
        push rax                        ; save register rax
        push rcx                        ; save register rcx
        push rdx                        ; save register rdx

        mov al, 0x20                    ; al = 0x20
        out 0x20, al                    ; end of interrupt

        mov edx, timer_ticks            ; rdx = address of timer_ticks
        lock inc qword [rdx]            ; increment ticks

        mov edx, timer_ticks_wait       ; rdx = address of timer_ticks_wait
.L1:    mov eax, [rdx]                  ; read current value
        test eax, eax                   ; test zero
        jz short timer_call_handler

        lea rcx, [rax-1]                ; ecx = decremented wait value
        lock cmpxchg [rdx], ecx         ; try to decrement
        jnz short .L1                   ; retry if needed

        pop rdx                         ; restore register rdx
        pop rcx                         ; restore register rcx
        pop rax                         ; restore register rax
        iretq

align 16
timer_call_handler:
        push rbx                        ; save register rbx
        push r8                         ; save register r8
        push r9                         ; save register r9
        push r10                        ; save register r10
        push r11                        ; save register r11
        mov rbx, rsp                    ; save stack
        cld                             ; clear direction flag

        mov eax, timer_ticks_wait       ; rax = address of timer_ticks_wait
        mov dword [rax], 10             ; timer_ticks_wait = 10

        mov rax, rsp                    ; rax = stack pointer
        and rax, -8192                  ; rax = address of current task
        mov ecx, 1                      ; ecx = 1
        xchg [rax+24], ecx              ; task->asm_data2 = 1
        test ecx, ecx                   ; test zero
        jz short .L1

        mov eax, timer_fault_count      ; rax = address of timer_fault_count
        lock add dword [rax], 1         ; timer_fault_count += 1
        jmp short .end

.L1:    sti                             ; enable interrupts
        and rsp, -16                    ; align stack
        sub rsp, 32                     ; shadow space
        call timer_handler              ; call timer_handler

        mov rcx, rsp                    ; rcx = stack pointer
        and rcx, -8192                  ; rcx = address of current task
        cmp dword [rcx+28], 0           ; test task->asm_data3
        je short .L2

        lea rdx, [rbx+64]               ; rdx = address of iret stack
        mov rax, [rdx+8]                ; rax = segment register cs
        and eax, 3                      ; eax = eax & 3
        cmp eax, 3                      ; test user space segment
        jne short .L2
        call ret_user_handler           ; call ret_user_handler

.L2:    ; nop
        mov rsp, rbx                    ; restore stack

        cli                             ; disable interrupts
        and rbx, -8192                  ; rbx = address of current task
        mov dword [rbx+24], 0           ; task->asm_data2 = 0

.end:   pop r11                         ; restore register r11
        pop r10                         ; restore register r10
        pop r9                          ; restore register r9
        pop r8                          ; restore register r8
        pop rbx                         ; restore register rbx
        pop rdx                         ; restore register rdx
        pop rcx                         ; restore register rcx
        pop rax                         ; restore register rax
        iretq

align 64
        ; const uint8_t timer_asm_handler_apic_ap[]
timer_asm_handler_apic_ap:
        push rax                        ; save register rax
        push rcx                        ; save register rcx
        push rdx                        ; save register rdx
        push rbx                        ; save register rbx
        push r8                         ; save register r8
        push r9                         ; save register r9
        push r10                        ; save register r10
        push r11                        ; save register r11
        mov rbx, rsp                    ; save stack
        cld                             ; clear direction flag

        mov edx, timer_apic_base        ; rdx = address of timer_apic_base
        mov rax, [rdx]                  ; rax = timer_apic_base
        mov dword [rax+0xB0], 0         ; end of interrupt

        mov rax, rsp                    ; rax = stack pointer
        and rax, -8192                  ; rax = address of current task
        mov ecx, 1                      ; ecx = 1
        xchg [rax+24], ecx              ; task->asm_data2 = 1
        test ecx, ecx                   ; test zero
        jz short .L1

        mov eax, timer_fault_count      ; rax = address of timer_fault_count
        lock add dword [rax], 1         ; timer_fault_count += 1
        jmp short .end

.L1:    sti                             ; enable interrupts
        and rsp, -16                    ; align stack
        sub rsp, 32                     ; shadow space
        call timer_handler_ap           ; call timer_handler_ap

        mov rcx, rsp                    ; rcx = stack pointer
        and rcx, -8192                  ; rcx = address of current task
        cmp dword [rcx+28], 0           ; test task->asm_data3
        je short .L2

        lea rdx, [rbx+64]               ; rdx = address of iret stack
        mov rax, [rdx+8]                ; rax = segment register cs
        and eax, 3                      ; eax = eax & 3
        cmp eax, 3                      ; test user space segment
        jne short .L2
        call ret_user_handler           ; call ret_user_handler

.L2:    ; nop
        mov rsp, rbx                    ; restore stack

        cli                             ; disable interrupts
        and rbx, -8192                  ; rbx = address of current task
        mov dword [rbx+24], 0           ; task->asm_data2 = 0

.end:   pop r11                         ; restore register r11
        pop r10                         ; restore register r10
        pop r9                          ; restore register r9
        pop r8                          ; restore register r8
        pop rbx                         ; restore register rbx
        pop rdx                         ; restore register rdx
        pop rcx                         ; restore register rcx
        pop rax                         ; restore register rax
        iretq

align 16
        ; uint64_t timer_read(void)
timer_read:
        mov edx, timer_ticks_64         ; rdx = address of timer_ticks_64
        mov rax, [rdx]                  ; rax = ticks
        ret


section .data

        global timer_ticks
        global timer_ticks_64
        global timer_ticks_wait
        global timer_fault_count

align 16
        ; uint32_t timer_ticks
timer_ticks:
        ; uint64_t timer_ticks_64
timer_ticks_64:
        dd 0, 0

        ; uint32_t timer_ticks_wait
timer_ticks_wait:
        dd 0

        ; uint32_t timer_fault_count
timer_fault_count:
        dd 0
