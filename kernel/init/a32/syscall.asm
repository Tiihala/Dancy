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
;; init/a32/syscall.asm
;;      Boot loader system calls (32-bit)
;;

        bits 32

section .text

        global _b_output_string
        global _b_output_string_hl
        global _b_output_control
        global _b_get_keycode
        global _b_get_byte_com1
        global _b_put_byte_com1
        global _b_get_byte_com2
        global _b_put_byte_com2
        global _b_get_parameter
        global _b_get_structure
        global _b_set_read_buffer
        global _b_read_blocks
        global _b_set_write_buffer
        global _b_write_blocks
        global _b_pause
        global _b_exit

align 16
        ; unsigned long b_output_string(const char *, unsigned int)
_b_output_string:
        push ebx                        ; save register ebx
        mov ebx, [esp+8]                ; ebx = argument 1
        mov ecx, [esp+12]               ; ecx = argument 2
        mov ah, 0xA0                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        pop ebx                         ; restore register ebx
        ret

align 16
        ; unsigned long b_output_string_hl(const char *, unsigned int)
_b_output_string_hl:
        push ebx                        ; save register ebx
        mov ebx, [esp+8]                ; ebx = argument 1
        mov ecx, [esp+12]               ; ecx = argument 2
        mov ah, 0xA1                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        pop ebx                         ; restore register ebx
        ret

align 16
        ; unsigned long b_output_control(unsigned int, unsigned int)
_b_output_control:
        push ebx                        ; save register ebx
        mov ebx, [esp+8]                ; ebx = argument 1
        mov ecx, [esp+12]               ; ecx = argument 2
        mov ah, 0xA2                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        pop ebx                         ; restore register ebx
        ret

align 16
        ; unsigned long b_get_keycode(void)
_b_get_keycode:
        mov ah, 0xA3                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        ret

align 16
        ; unsigned long b_get_byte_com1(void)
_b_get_byte_com1:
        mov ah, 0xA4                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        jc short .end
        or eax, 0xFFFFFF00              ; al = byte
.end:   ret

align 16
        ; unsigned long b_put_byte_com1(unsigned char)
_b_put_byte_com1:
        push ebx                        ; save register ebx
        xor ebx, ebx                    ; ebx = argument 1 (zero)
        mov ecx, [esp+8]                ; ecx = argument 2
        mov ah, 0xA5                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        jc short .end
        mov eax, 0x00000001             ; eax = 0x00000001
.end:   pop ebx                         ; restore register ebx
        ret

align 16
        ; unsigned long b_get_byte_com2(void)
_b_get_byte_com2:
        mov ah, 0xA6                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        jc short .end
        or eax, 0xFFFFFF00              ; al = byte
.end:   ret

align 16
        ; unsigned long b_put_byte_com2(unsigned char)
_b_put_byte_com2:
        push ebx                        ; save register ebx
        xor ebx, ebx                    ; ebx = argument 1 (zero)
        mov ecx, [esp+8]                ; ecx = argument 2
        mov ah, 0xA7                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        jc short .end
        mov eax, 0x00000001             ; eax = 0x00000001
.end:   pop ebx                         ; restore register ebx
        ret

align 16
        ; unsigned long b_get_parameter(unsigned int)
_b_get_parameter:
        push ebx                        ; save register ebx
        xor ebx, ebx                    ; ebx = argument 1 (zero)
        mov ecx, [esp+8]                ; ecx = argument 2
        mov ah, 0xA8                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        pop ebx                         ; restore register ebx
        ret

align 16
        ; unsigned long b_get_structure(void *, unsigned int)
_b_get_structure:
        push ebx                        ; save register ebx
        mov ebx, [esp+8]                ; ebx = argument 1
        mov ecx, [esp+12]               ; ecx = argument 2
        mov ah, 0xA9                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        pop ebx                         ; restore register ebx
        ret

align 16
        ; unsigned long b_set_read_buffer(void *, unsigned int)
_b_set_read_buffer:
        push ebx                        ; save register ebx
        mov ebx, [esp+8]                ; ebx = argument 1
        mov ecx, [esp+12]               ; ecx = argument 2
        mov ah, 0xAA                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        pop ebx                         ; restore register ebx
        ret

align 16
        ; unsigned long b_read_blocks(unsigned int, unsigned int)
_b_read_blocks:
        push ebx                        ; save register ebx
        mov ebx, [esp+8]                ; ebx = argument 1
        mov ecx, [esp+12]               ; ecx = argument 2
        mov ah, 0xAB                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        pop ebx                         ; restore register ebx
        ret

align 16
        ; unsigned long b_set_write_buffer(void *, unsigned int)
_b_set_write_buffer:
        push ebx                        ; save register ebx
        mov ebx, [esp+8]                ; ebx = argument 1
        mov ecx, [esp+12]               ; ecx = argument 2
        mov ah, 0xAC                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        pop ebx                         ; restore register ebx
        ret

align 16
        ; unsigned long b_write_blocks(unsigned int, unsigned int)
_b_write_blocks:
        push ebx                        ; save register ebx
        mov ebx, [esp+8]                ; ebx = argument 1
        mov ecx, [esp+12]               ; ecx = argument 2
        mov ah, 0xAD                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        pop ebx                         ; restore register ebx
        ret

align 16
        ; unsigned long b_pause(void)
_b_pause:
        mov ah, 0xAE                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        ret

align 16
        ; unsigned long b_exit(void)
_b_exit:
        mov ah, 0xAF                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        ret


section .data

        global _boot_loader_type

align 4
_boot_loader_type:
        dd 0x00000000
