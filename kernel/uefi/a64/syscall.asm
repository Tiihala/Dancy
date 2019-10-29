;;
;; Copyright (c) 2019 Antti Tiihala
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
;; uefi/a64/syscall.asm
;;      Boot loader system calls (UEFI)
;;

        bits 64

section .text

        extern b_output_string
        extern b_output_string_hl
        extern b_output_control
        extern b_get_keycode
        extern b_get_byte_com1
        extern b_put_byte_com1
        extern b_get_byte_com2
        extern b_put_byte_com2
        extern b_get_parameter
        extern b_get_structure
        extern b_set_read_buffer
        extern b_read_blocks
        extern b_set_write_buffer
        extern b_write_blocks
        extern b_pause
        extern b_exit
        extern b_get_time
        global syscall_init
        global syscall_jump

align 16
        ; void syscall_init(void *addr)
syscall_init:
        mov dword [rcx+(0*8)], sys_0    ; b_output_string
        mov dword [rcx+(1*8)], sys_1    ; b_output_string_hl
        mov dword [rcx+(2*8)], sys_2    ; b_output_control
        mov dword [rcx+(3*8)], sys_3    ; b_get_keycode
        mov dword [rcx+(4*8)], sys_4    ; b_get_byte_com1
        mov dword [rcx+(5*8)], sys_5    ; b_put_byte_com1
        mov dword [rcx+(6*8)], sys_6    ; b_get_byte_com2
        mov dword [rcx+(7*8)], sys_7    ; b_put_byte_com2
        mov dword [rcx+(8*8)], sys_8    ; b_get_parameter
        mov dword [rcx+(9*8)], sys_9    ; b_get_structure
        mov dword [rcx+(10*8)], sys_10  ; b_set_read_buffer
        mov dword [rcx+(11*8)], sys_11  ; b_read_blocks
        mov dword [rcx+(12*8)], sys_12  ; b_set_write_buffer
        mov dword [rcx+(13*8)], sys_13  ; b_write_blocks
        mov dword [rcx+(14*8)], sys_14  ; b_pause
        mov dword [rcx+(15*8)], sys_15  ; b_exit
        mov dword [rcx+(16*8)], sys_16  ; b_get_time
        ret

align 16
        ; void syscall_jump(void *addr)
syscall_jump:
        push rbp                        ; save register rbp
        mov rbp, rsp                    ; rbp = current stack pointer

        cmp byte [rcx], 0x8D            ; test the first byte
        jne short .err

        sub rbp, 32                     ; rbp = stack for syscalls
        test rbp, 0x0000000F            ; (extra check)
        jnz short .err
        mov [syscall_stack], rbp        ; 16-byte alignment + shadow space

        lea rsp, [rcx+0xFFF0]           ; set the new stack (in_x64.at)
        lea rcx, [rcx+32]               ; rcx = start address

        push rcx                        ; push the start address
        xor eax, eax                    ; eax = 0
        xor ecx, ecx                    ; ecx = 0
        xor edx, edx                    ; edx = 0
        xor ebx, ebx                    ; ebx = 0
        xor ebp, ebp                    ; ebp = 0
        xor esi, esi                    ; esi = 0
        xor edi, edi                    ; edi = 0
        xor r8, r8                      ; r8 = 0
        xor r9, r9                      ; r9 = 0
        xor r10, r10                    ; r10 = 0
        xor r11, r11                    ; r11 = 0
        xor r12, r12                    ; r12 = 0
        xor r13, r13                    ; r13 = 0
        xor r14, r14                    ; r14 = 0
        xor r15, r15                    ; r15 = 0
        ret                             ; start the in_x64.at

.err:   mov rsp, rbp                    ; restore stack pointer
        pop rbp                         ; restore register rbp
        ret

align 16
sys_0:
        push rbp                        ; save register rbp
        mov rbp, rsp                    ; rbp = current stack pointer
        mov rsp, [syscall_stack]        ; set stack (alignment + shadow space)
        call b_output_string            ; call the syscall function
        mov rsp, rbp                    ; restore stack pointer
        pop rbp                         ; restore register rbp
        ret

align 16
sys_1:
        push rbp                        ; save register rbp
        mov rbp, rsp                    ; rbp = current stack pointer
        mov rsp, [syscall_stack]        ; set stack (alignment + shadow space)
        call b_output_string_hl         ; call the syscall function
        mov rsp, rbp                    ; restore stack pointer
        pop rbp                         ; restore register rbp
        ret

align 16
sys_2:
        push rbp                        ; save register rbp
        mov rbp, rsp                    ; rbp = current stack pointer
        mov rsp, [syscall_stack]        ; set stack (alignment + shadow space)
        call b_output_control           ; call the syscall function
        mov rsp, rbp                    ; restore stack pointer
        pop rbp                         ; restore register rbp
        ret

align 16
sys_3:
        push rbp                        ; save register rbp
        mov rbp, rsp                    ; rbp = current stack pointer
        mov rsp, [syscall_stack]        ; set stack (alignment + shadow space)
        call b_get_keycode              ; call the syscall function
        mov rsp, rbp                    ; restore stack pointer
        pop rbp                         ; restore register rbp
        ret

align 16
sys_4:
        push rbp                        ; save register rbp
        mov rbp, rsp                    ; rbp = current stack pointer
        mov rsp, [syscall_stack]        ; set stack (alignment + shadow space)
        call b_get_byte_com1            ; call the syscall function
        mov rsp, rbp                    ; restore stack pointer
        pop rbp                         ; restore register rbp
        ret

align 16
sys_5:
        push rbp                        ; save register rbp
        mov rbp, rsp                    ; rbp = current stack pointer
        mov rsp, [syscall_stack]        ; set stack (alignment + shadow space)
        call b_put_byte_com1            ; call the syscall function
        mov rsp, rbp                    ; restore stack pointer
        pop rbp                         ; restore register rbp
        ret

align 16
sys_6:
        push rbp                        ; save register rbp
        mov rbp, rsp                    ; rbp = current stack pointer
        mov rsp, [syscall_stack]        ; set stack (alignment + shadow space)
        call b_get_byte_com2            ; call the syscall function
        mov rsp, rbp                    ; restore stack pointer
        pop rbp                         ; restore register rbp
        ret

align 16
sys_7:
        push rbp                        ; save register rbp
        mov rbp, rsp                    ; rbp = current stack pointer
        mov rsp, [syscall_stack]        ; set stack (alignment + shadow space)
        call b_put_byte_com2            ; call the syscall function
        mov rsp, rbp                    ; restore stack pointer
        pop rbp                         ; restore register rbp
        ret

align 16
sys_8:
        push rbp                        ; save register rbp
        mov rbp, rsp                    ; rbp = current stack pointer
        mov rsp, [syscall_stack]        ; set stack (alignment + shadow space)
        call b_get_parameter            ; call the syscall function
        mov rsp, rbp                    ; restore stack pointer
        pop rbp                         ; restore register rbp
        ret

align 16
sys_9:
        push rbp                        ; save register rbp
        mov rbp, rsp                    ; rbp = current stack pointer
        mov rsp, [syscall_stack]        ; set stack (alignment + shadow space)
        call b_get_structure            ; call the syscall function
        mov rsp, rbp                    ; restore stack pointer
        pop rbp                         ; restore register rbp
        ret

align 16
sys_10:
        push rbp                        ; save register rbp
        mov rbp, rsp                    ; rbp = current stack pointer
        mov rsp, [syscall_stack]        ; set stack (alignment + shadow space)
        call b_set_read_buffer          ; call the syscall function
        mov rsp, rbp                    ; restore stack pointer
        pop rbp                         ; restore register rbp
        ret

align 16
sys_11:
        push rbp                        ; save register rbp
        mov rbp, rsp                    ; rbp = current stack pointer
        mov rsp, [syscall_stack]        ; set stack (alignment + shadow space)
        call b_read_blocks              ; call the syscall function
        mov rsp, rbp                    ; restore stack pointer
        pop rbp                         ; restore register rbp
        ret

align 16
sys_12:
        push rbp                        ; save register rbp
        mov rbp, rsp                    ; rbp = current stack pointer
        mov rsp, [syscall_stack]        ; set stack (alignment + shadow space)
        call b_set_write_buffer         ; call the syscall function
        mov rsp, rbp                    ; restore stack pointer
        pop rbp                         ; restore register rbp
        ret

align 16
sys_13:
        push rbp                        ; save register rbp
        mov rbp, rsp                    ; rbp = current stack pointer
        mov rsp, [syscall_stack]        ; set stack (alignment + shadow space)
        call b_write_blocks             ; call the syscall function
        mov rsp, rbp                    ; restore stack pointer
        pop rbp                         ; restore register rbp
        ret

align 16
sys_14:
        push rbp                        ; save register rbp
        mov rbp, rsp                    ; rbp = current stack pointer
        mov rsp, [syscall_stack]        ; set stack (alignment + shadow space)
        call b_pause                    ; call the syscall function
        mov rsp, rbp                    ; restore stack pointer
        pop rbp                         ; restore register rbp
        ret

align 16
sys_15:
        push rbp                        ; save register rbp
        mov rbp, rsp                    ; rbp = current stack pointer
        mov rsp, [syscall_stack]        ; set stack (alignment + shadow space)
        call b_exit                     ; call the syscall function
        mov rsp, rbp                    ; restore stack pointer
        pop rbp                         ; restore register rbp
        ret

align 16
sys_16:
        push rbp                        ; save register rbp
        mov rbp, rsp                    ; rbp = current stack pointer
        mov rsp, [syscall_stack]        ; set stack (alignment + shadow space)
        call b_get_time                 ; call the syscall function
        mov rsp, rbp                    ; restore stack pointer
        pop rbp                         ; restore register rbp
        ret


section .data

align 8
syscall_stack:
        dq 0
