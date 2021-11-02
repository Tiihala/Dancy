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
;; base/a32/timer.asm
;;      Kernel timer
;;

        bits 32

section .text

        extern _timer_handler
        extern _timer_handler_ap
        global _timer_apic_base
        global _timer_asm_handler_apic
        global _timer_asm_handler_apic_ap
        global _timer_asm_handler_pic
        global _timer_read

align 64
        ; const uint8_t timer_asm_handler_apic[]
_timer_asm_handler_apic:
        push ebp                        ; save register ebp
        push eax                        ; save register eax
        push ecx                        ; save register ecx
        push edx                        ; save register edx
        push ebx                        ; save register ebx

        ; uint8_t timer_apic_base[4]
        db 0xBD                         ; "mov ebp, [_timer_apic_base]"
_timer_apic_base: db 0, 0, 0, 0
        mov dword [ebp+0xB0], 0         ; end of interrupt

        mov ebp, _timer_ticks_64        ; ebp = address of _timer_ticks_64
        mov eax, [ebp+0]                ; eax = ticks (low dword)
        mov edx, [ebp+4]                ; edx = ticks (high dword)
        mov ebx, eax                    ; ebx = ticks (low dword)
        mov ecx, edx                    ; ecx = ticks (high dword)
        add ebx, 1                      ; increment ticks (low dword)
        adc ecx, 0                      ; increment ticks (high dword)
        lock cmpxchg8b [ebp]            ; update [_timer_ticks_64]

        mov ebp, _timer_ticks_wait      ; ebp = address of _timer_ticks_wait
.L1:    mov eax, [ebp]                  ; read current value
        test eax, eax                   ; test zero
        jz near _timer_call_handler

        lea ecx, [eax-1]                ; ecx = decremented wait value
        lock cmpxchg [ebp], ecx         ; try to decrement
        jnz short .L1                   ; retry if needed

        pop ebx                         ; restore register ebx
        pop edx                         ; restore register edx
        pop ecx                         ; restore register ecx
        pop eax                         ; restore register eax
        pop ebp                         ; restore register ebp
        iret

align 64
        ; const uint8_t timer_asm_handler_pic[]
_timer_asm_handler_pic:
        push ebp                        ; save register ebp
        push eax                        ; save register eax
        push ecx                        ; save register ecx
        push edx                        ; save register edx
        push ebx                        ; save register ebx

        mov al, 0x20                    ; al = 0x20
        out 0x20, al                    ; end of interrupt

        mov ebp, _timer_ticks_64        ; ebp = address of _timer_ticks_64
        mov eax, [ebp+0]                ; eax = ticks (low dword)
        mov edx, [ebp+4]                ; edx = ticks (high dword)
        mov ebx, eax                    ; ebx = ticks (low dword)
        mov ecx, edx                    ; ecx = ticks (high dword)
        add ebx, 1                      ; increment ticks (low dword)
        adc ecx, 0                      ; increment ticks (high dword)
        lock cmpxchg8b [ebp]            ; update [_timer_ticks_64]

        mov ebp, _timer_ticks_wait      ; ebp = address of _timer_ticks_wait
.L1:    mov eax, [ebp]                  ; read current value
        test eax, eax                   ; test zero
        jz short _timer_call_handler

        lea ecx, [eax-1]                ; ecx = decremented wait value
        lock cmpxchg [ebp], ecx         ; try to decrement
        jnz short .L1                   ; retry if needed

        pop ebx                         ; restore register ebx
        pop edx                         ; restore register edx
        pop ecx                         ; restore register ecx
        pop eax                         ; restore register eax
        pop ebp                         ; restore register ebp
        iret

align 16
_timer_call_handler:
        push es                         ; save segment register es
        push ds                         ; save segment register ds
        mov ebp, esp                    ; save stack
        cld                             ; clear direction flag

        mov eax, _timer_ticks_wait      ; eax = address of _timer_ticks_wait
        mov dword [eax], 10             ; _timer_ticks_wait = 10

        mov eax, esp                    ; eax = stack pointer
        and eax, 0xFFFFE000             ; eax = address of current task
        mov ecx, 1                      ; ecx = 1
        xchg [eax+24], ecx              ; task->asm_data2 = 1
        test ecx, ecx                   ; test zero
        jz short .L1

        mov eax, _timer_fault_count     ; eax = address of _timer_fault_count
        lock add dword [eax], 1         ; _timer_fault_count += 1
        jmp short .end

.L1:    sti                             ; enable interrupts
        and esp, 0xFFFFFFF0             ; align stack
        call _timer_handler             ; call _timer_handler
        mov esp, ebp                    ; restore stack

        cli                             ; disable interrupts
        and ebp, 0xFFFFE000             ; ebp = address of current task
        mov dword [ebp+24], 0           ; task->asm_data2 = 0

.end:   pop ds                          ; restore segment register ds
        pop es                          ; restore segment register es
        pop ebx                         ; restore register ebx
        pop edx                         ; restore register edx
        pop ecx                         ; restore register ecx
        pop eax                         ; restore register eax
        pop ebp                         ; restore register ebp
        iret

align 64
        ; const uint8_t timer_asm_handler_apic_ap[]
_timer_asm_handler_apic_ap:
        push eax                        ; save register eax
        push ecx                        ; save register ecx
        push edx                        ; save register edx
        push ebx                        ; save register ebx
        push es                         ; save segment register es
        push ds                         ; save segment register ds
        mov ebx, esp                    ; save stack
        cld                             ; clear direction flag

        mov eax, [_timer_apic_base]     ; eax = _timer_apic_base
        mov dword [eax+0xB0], 0         ; end of interrupt

        mov eax, esp                    ; eax = stack pointer
        and eax, 0xFFFFE000             ; eax = address of current task
        mov ecx, 1                      ; ecx = 1
        xchg [eax+24], ecx              ; task->asm_data2 = 1
        test ecx, ecx                   ; test zero
        jz short .L1

        mov eax, _timer_fault_count     ; eax = address of _timer_fault_count
        lock add dword [eax], 1         ; _timer_fault_count += 1
        jmp short .end

.L1:    sti                             ; enable interrupts
        and esp, 0xFFFFFFF0             ; align stack
        call _timer_handler_ap          ; call _timer_handler_ap
        mov esp, ebx                    ; restore stack

        cli                             ; disable interrupts
        and ebx, 0xFFFFE000             ; ebx = address of current task
        mov dword [ebx+24], 0           ; task->asm_data2 = 0

.end:   pop ds                          ; restore segment register ds
        pop es                          ; restore segment register es
        pop ebx                         ; restore register ebx
        pop edx                         ; restore register edx
        pop ecx                         ; restore register ecx
        pop eax                         ; restore register eax
        iret

align 16
        ; uint64_t timer_read(void)
_timer_read:
        push ebx                        ; save register ebx
        xor eax, eax                    ; eax = 0
        xor ecx, ecx                    ; ecx = 0
        xor edx, edx                    ; edx = 0
        xor ebx, ebx                    ; ebx = 0
        lock cmpxchg8b [_timer_ticks]   ; edx:eax = ticks
        pop ebx                         ; restore register ebx
        ret


section .data

        global _timer_ticks
        global _timer_ticks_64
        global _timer_ticks_wait
        global _timer_fault_count

align 16
        ; uint32_t timer_ticks
_timer_ticks:
        ; uint64_t timer_ticks_64
_timer_ticks_64:
        dd 0, 0

        ; uint32_t timer_ticks_wait
_timer_ticks_wait:
        dd 0

        ; uint32_t timer_fault_count
_timer_fault_count:
        dd 0
