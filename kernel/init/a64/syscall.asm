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
;; init/a64/syscall.asm
;;      Boot loader system calls (64-bit)
;;

        bits 64

section .text

        extern uefi_syscalls
        global b_output_string
        global b_output_string_hl
        global b_output_control
        global b_get_keycode
        global b_get_byte_com1
        global b_put_byte_com1
        global b_get_byte_com2
        global b_put_byte_com2
        global b_get_parameter
        global b_get_structure
        global b_set_read_buffer
        global b_read_blocks
        global b_set_write_buffer
        global b_write_blocks
        global b_pause
        global b_exit

align 16
        ; unsigned long b_output_string(const char *, unsigned int)
b_output_string:
        mov eax, [uefi_syscalls]        ; get the uefi syscall offset
        test eax, eax                   ; test zero
        jz short .bios_boot_loader
        add eax, (0 * 8)                ; syscall offset
        jmp [rax]                       ; jump into the boot loader
.bios_boot_loader:
        push rbx                        ; save register rbx
        mov ebx, ecx                    ; ebx = argument 1
        mov ecx, edx                    ; ecx = argument 2
        mov ah, 0xA0                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        pop rbx                         ; restore register rbx
        ret

align 16
        ; unsigned long b_output_string_hl(const char *, unsigned int)
b_output_string_hl:
        mov eax, [uefi_syscalls]        ; get the uefi syscall offset
        test eax, eax                   ; test zero
        jz short .bios_boot_loader
        add eax, (1 * 8)                ; syscall offset
        jmp [rax]                       ; jump into the boot loader
.bios_boot_loader:
        push rbx                        ; save register rbx
        mov ebx, ecx                    ; ebx = argument 1
        mov ecx, edx                    ; ecx = argument 2
        mov ah, 0xA1                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        pop rbx                         ; restore register rbx
        ret

align 16
        ; unsigned long b_output_control(unsigned int, unsigned int)
b_output_control:
        mov eax, [uefi_syscalls]        ; get the uefi syscall offset
        test eax, eax                   ; test zero
        jz short .bios_boot_loader
        add eax, (2 * 8)                ; syscall offset
        jmp [rax]                       ; jump into the boot loader
.bios_boot_loader:
        push rbx                        ; save register rbx
        mov ebx, ecx                    ; ebx = argument 1
        mov ecx, edx                    ; ecx = argument 2
        mov ah, 0xA2                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        pop rbx                         ; restore register rbx
        ret

align 16
        ; unsigned long b_get_keycode(void)
b_get_keycode:
        mov eax, [uefi_syscalls]        ; get the uefi syscall offset
        test eax, eax                   ; test zero
        jz short .bios_boot_loader
        add eax, (3 * 8)                ; syscall offset
        jmp [rax]                       ; jump into the boot loader
.bios_boot_loader:
        mov ah, 0xA3                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        ret

align 16
        ; unsigned long b_get_byte_com1(void)
b_get_byte_com1:
        mov eax, [uefi_syscalls]        ; get the uefi syscall offset
        test eax, eax                   ; test zero
        jz short .bios_boot_loader
        add eax, (4 * 8)                ; syscall offset
        jmp [rax]                       ; jump into the boot loader
.bios_boot_loader:
        mov ah, 0xA4                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        jc short .end
        or eax, 0xFFFFFF00              ; al = byte
.end:   ret

align 16
        ; unsigned long b_put_byte_com1(unsigned char)
b_put_byte_com1:
        mov eax, [uefi_syscalls]        ; get the uefi syscall offset
        test eax, eax                   ; test zero
        jz short .bios_boot_loader
        add eax, (5 * 8)                ; syscall offset
        jmp [rax]                       ; jump into the boot loader
.bios_boot_loader:
        push rbx                        ; save register rbx
        xor ebx, ebx                    ; ebx = argument 1 (zero)
                                        ; ecx = argument 2
        mov ah, 0xA5                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        jc short .end
        mov eax, 0x00000001             ; eax = 0x00000001
.end:   pop rbx                         ; restore register rbx
        ret

align 16
        ; unsigned long b_get_byte_com2(void)
b_get_byte_com2:
        mov eax, [uefi_syscalls]        ; get the uefi syscall offset
        test eax, eax                   ; test zero
        jz short .bios_boot_loader
        add eax, (6 * 8)                ; syscall offset
        jmp [rax]                       ; jump into the boot loader
.bios_boot_loader:
        mov ah, 0xA6                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        jc short .end
        or eax, 0xFFFFFF00              ; al = byte
.end:   ret

align 16
        ; unsigned long b_put_byte_com2(unsigned char)
b_put_byte_com2:
        mov eax, [uefi_syscalls]        ; get the uefi syscall offset
        test eax, eax                   ; test zero
        jz short .bios_boot_loader
        add eax, (7 * 8)                ; syscall offset
        jmp [rax]                       ; jump into the boot loader
.bios_boot_loader:
        push rbx                        ; save register rbx
        xor ebx, ebx                    ; ebx = argument 1 (zero)
                                        ; ecx = argument 2
        mov ah, 0xA7                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        jc short .end
        mov eax, 0x00000001             ; eax = 0x00000001
.end:   pop rbx                         ; restore register rbx
        ret

align 16
        ; unsigned long b_get_parameter(unsigned int)
b_get_parameter:
        mov eax, [uefi_syscalls]        ; get the uefi syscall offset
        test eax, eax                   ; test zero
        jz short .bios_boot_loader
        add eax, (8 * 8)                ; syscall offset
        jmp [rax]                       ; jump into the boot loader
.bios_boot_loader:
        push rbx                        ; save register rbx
        xor ebx, ebx                    ; ebx = argument 1 (zero)
                                        ; ecx = argument 2
        mov ah, 0xA8                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        pop rbx                         ; restore register rbx
        ret

align 16
        ; unsigned long b_get_structure(void *, unsigned int)
b_get_structure:
        mov eax, [uefi_syscalls]        ; get the uefi syscall offset
        test eax, eax                   ; test zero
        jz short .bios_boot_loader
        add eax, (9 * 8)                ; syscall offset
        jmp [rax]                       ; jump into the boot loader
.bios_boot_loader:
        push rbx                        ; save register rbx
        mov ebx, ecx                    ; ebx = argument 1
        mov ecx, edx                    ; ecx = argument 2
        mov ah, 0xA9                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        pop rbx                         ; restore register rbx
        ret

align 16
        ; unsigned long b_set_read_buffer(void *, unsigned int)
b_set_read_buffer:
        mov eax, [uefi_syscalls]        ; get the uefi syscall offset
        test eax, eax                   ; test zero
        jz short .bios_boot_loader
        add eax, (10 * 8)               ; syscall offset
        jmp [rax]                       ; jump into the boot loader
.bios_boot_loader:
        push rbx                        ; save register rbx
        mov ebx, ecx                    ; ebx = argument 1
        mov ecx, edx                    ; ecx = argument 2
        mov ah, 0xAA                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        pop rbx                         ; restore register rbx
        ret

align 16
        ; unsigned long b_read_blocks(unsigned int, unsigned int)
b_read_blocks:
        mov eax, [uefi_syscalls]        ; get the uefi syscall offset
        test eax, eax                   ; test zero
        jz short .bios_boot_loader
        add eax, (11 * 8)               ; syscall offset
        jmp [rax]                       ; jump into the boot loader
.bios_boot_loader:
        push rbx                        ; save register rbx
        mov ebx, ecx                    ; ebx = argument 1
        mov ecx, edx                    ; ecx = argument 2
        mov ah, 0xAB                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        pop rbx                         ; restore register rbx
        ret

align 16
        ; unsigned long b_set_write_buffer(void *, unsigned int)
b_set_write_buffer:
        mov eax, [uefi_syscalls]        ; get the uefi syscall offset
        test eax, eax                   ; test zero
        jz short .bios_boot_loader
        add eax, (12 * 8)               ; syscall offset
        jmp [rax]                       ; jump into the boot loader
.bios_boot_loader:
        push rbx                        ; save register rbx
        mov ebx, ecx                    ; ebx = argument 1
        mov ecx, edx                    ; ecx = argument 2
        mov ah, 0xAC                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        pop rbx                         ; restore register rbx
        ret

align 16
        ; unsigned long b_write_blocks(unsigned int, unsigned int)
b_write_blocks:
        mov eax, [uefi_syscalls]        ; get the uefi syscall offset
        test eax, eax                   ; test zero
        jz short .bios_boot_loader
        add eax, (13 * 8)               ; syscall offset
        jmp [rax]                       ; jump into the boot loader
.bios_boot_loader:
        push rbx                        ; save register rbx
        mov ebx, ecx                    ; ebx = argument 1
        mov ecx, edx                    ; ecx = argument 2
        mov ah, 0xAD                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        pop rbx                         ; restore register rbx
        ret

align 16
        ; unsigned long b_pause(void)
b_pause:
        mov eax, [uefi_syscalls]        ; get the uefi syscall offset
        test eax, eax                   ; test zero
        jz short .bios_boot_loader
        add eax, (14 * 8)               ; syscall offset
        jmp [rax]                       ; jump into the boot loader
.bios_boot_loader:
        mov ah, 0xAE                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        ret

align 16
        ; unsigned long b_exit(void)
b_exit:
        mov eax, [uefi_syscalls]        ; get the uefi syscall offset
        test eax, eax                   ; test zero
        jz short .bios_boot_loader
        add eax, (15 * 8)               ; syscall offset
        jmp [rax]                       ; jump into the boot loader
.bios_boot_loader:
        mov ah, 0xAF                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        ret
