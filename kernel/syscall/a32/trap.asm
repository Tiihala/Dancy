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
;; syscall/a32/trap.asm
;;      System calls
;;

        bits 32

section .text

        extern _ret_user_handler
        extern _syscall_handler
        global _syscall_init_asm

align 16
        ; void syscall_init_asm(void)
_syscall_init_asm:
        push ebp                        ; save register ebp
        mov ebp, esp                    ; ebp = stack pointer
        sub esp, 16                     ; allocate stack space

        sidt [ebp-10]                   ; store idt register
        mov eax, [ebp-8]                ; eax = base address
        add eax, (0x64 * 8)             ; eax = entry address
        mov ecx, _syscall_asm_handler   ; ecx = address of syscall_asm_handler

        mov edx, ecx                    ; edx = ecx
        and edx, 0x0000FFFF             ; edx = (ecx & 0xFFFF)
        or  edx, 0x00080000             ; edx = (ecx & 0xFFFF) | 0x00080000
        mov [eax+0], edx                ; install first part

        mov edx, ecx                    ; edx = ecx
        and edx, 0xFFFF0000             ; edx = (ecx & 0xFFFF0000)
        or  edx, 0x0000EF00             ; edx = (ecx & 0xFFFF0000) | 0xEF00
        mov [eax+4], edx                ; install second part

        leave                           ; release stack frame
        ret

align 64
_syscall_asm_handler:
        push ecx                        ; save register ecx
        push ebp                        ; save register ebp
        push es                         ; save segment register es
        push ds                         ; save segment register ds
        mov ebp, esp                    ; save stack
        cld                             ; clear direction flag

        and esp, -16                    ; align stack

        ; long long syscall_handler(int arg0, ...)
        ;
        ; arg0 = eax
        ; arg1 = ecx
        ; arg2 = edx
        ; arg3 = ebx
        ; arg4 = esi
        ; arg5 = edi

        push 0                          ; push 0
        push 0                          ; push 0
        push edi                        ; push arg5
        push esi                        ; push arg4
        push ebx                        ; push arg3
        push edx                        ; push arg2
        push ecx                        ; push arg1
        push eax                        ; push arg0
        call _syscall_handler           ; call syscall_handler

        mov ecx, esp                    ; ecx = stack pointer
        and ecx, 0xFFFFE000             ; ecx = address of current task
        cmp dword [ecx+28], 0           ; test task->asm_data3
        je short .L2

        add esp, 24                     ; pop six dwords
        push eax                        ; save register eax
        push edx                        ; save register edx

        lea edx, [ebp+16]               ; edx = address of iret stack
        mov eax, [edx+4]                ; eax = segment register cs
        and eax, 3                      ; eax = eax & 3
        cmp eax, 3                      ; test user space segment
        jne short .L1

        push 0                          ; push 0
        push 0                          ; push 0
        push edx                        ; push address of iret stack
        push ecx                        ; push address of current task
        call _ret_user_handler          ; call _ret_user_handler
        add esp, 16                     ; pop four dwords

.L1:    pop edx                         ; restore register edx
        pop eax                         ; restore register eax

.L2:    ; nop
        mov esp, ebp                    ; restore stack
        pop ds                          ; restore segment register ds
        pop es                          ; restore segment register es
        pop ebp                         ; restore register ebp
        pop ecx                         ; restore register ecx
        iret
