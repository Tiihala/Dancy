;;
;; Copyright (c) 2022 Antti Tiihala
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
;; syscall/a64/trap.asm
;;      System calls
;;

        bits 64

section .text

        extern syscall_handler
        global syscall_init_asm

align 16
        ; void syscall_init_asm(void)
syscall_init_asm:
        push rbp                        ; save register rbp
        mov rbp, rsp                    ; rbp = stack pointer
        sub rsp, 16                     ; allocate stack space

        sidt [rbp-10]                   ; store idt register
        mov rax, [rbp-8]                ; rax = base address
        add rax, (0x64 * 16)            ; rax = entry address
        mov ecx, syscall_asm_handler    ; rcx = address of syscall_asm_handler

        mov edx, ecx                    ; rdx = rcx (32-bit)
        and edx, 0x0000FFFF             ; edx = (ecx & 0xFFFF)
        or  edx, 0x00080000             ; edx = (ecx & 0xFFFF) | 0x00080000
        mov [rax+0], edx                ; install first part

        mov edx, ecx                    ; edx = ecx
        and edx, 0xFFFF0000             ; edx = (ecx & 0xFFFF0000)
        or  edx, 0x0000EF00             ; edx = (ecx & 0xFFFF0000) | 0xEF00
        mov [rax+4], edx                ; install second part

        xor edx, edx                    ; rdx = 0
        mov [rax+8], rdx                ; install "third" part

        leave                           ; release stack frame
        ret

align 64
syscall_asm_handler:
        push rcx                        ; save register rcx
        push rbp                        ; save register rbp
        push r8                         ; save register r8
        push r9                         ; save register r9
        push r10                        ; save register r10
        push r11                        ; save register r11
        mov rbp, rsp                    ; save stack
        cld                             ; clear direction flag

        and rsp, -16                    ; align stack

        ; long long syscall_handler(int arg0, ...)
        ;
        ; arg0 = eax (32-bit)
        ; arg1 = rcx
        ; arg2 = rdx
        ; arg3 = rbx
        ; arg4 = rsi
        ; arg5 = rdi

        push rdi                        ; push arg5
        push rsi                        ; push arg4
        push rbx                        ; push arg3 (shadow space)
        push rdx                        ; push arg2 (shadow space)
        push rcx                        ; push arg1 (shadow space)
        push rax                        ; push arg0 (shadow space)

        mov ecx, [rsp+0]                ; set arg0 (32-bit)
        mov rdx, [rsp+8]                ; set arg1
        mov r8, [rsp+16]                ; set arg2
        mov r9, [rsp+24]                ; set arg3
        call syscall_handler            ; call syscall_handler
        xor edx, edx                    ; rdx = 0, rax = (long long)retval

        mov rsp, rbp                    ; restore stack
        pop r11                         ; restore register r11
        pop r10                         ; restore register r10
        pop r9                          ; restore register r9
        pop r8                          ; restore register r8
        pop rbp                         ; restore register rbp
        pop rcx                         ; restore register rcx
        iretq
