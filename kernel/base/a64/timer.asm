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

        extern timer_handler
        global timer_apic_base
        global timer_asm_handler_apic
        global timer_asm_handler_pic

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

        and rsp, -16                    ; align stack
        sub rsp, 32                     ; shadow space
        call timer_handler              ; call timer_handler
        mov rsp, rbx                    ; restore stack

        pop r11                         ; restore register r11
        pop r10                         ; restore register r10
        pop r9                          ; restore register r9
        pop r8                          ; restore register r8
        pop rbx                         ; restore register rbx
        pop rdx                         ; restore register rdx
        pop rcx                         ; restore register rcx
        pop rax                         ; restore register rax
        iretq


section .data

        global timer_ticks
        global timer_ticks_64
        global timer_ticks_wait

align 16
        ; uint32_t timer_ticks
timer_ticks:
        ; uint64_t timer_ticks_64
timer_ticks_64:
        dd 0, 0

        ; uint32_t timer_ticks_wait
timer_ticks_wait:
        dd 0
