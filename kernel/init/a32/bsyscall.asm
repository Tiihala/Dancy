;;
;; Copyright (c) 2018, 2019 Antti Tiihala
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
;; init/a32/bsyscall.asm
;;      Boot loader system calls (32-bit)
;;

        bits 32

section .text

%macro boot_loader_0 1
align 16
global _b_a%1
_b_a%1:
        mov ah, 0xA%1                   ; ah = syscall number
        int 0x20                        ; boot loader syscall
        jnc short .end
        mov eax, 0xFFFFFFFF             ; eax = B_RET_ERROR
.end:   ret
%endmacro

%macro boot_loader_1 1
align 16
global _b_a%1
_b_a%1:
        push ebx                        ; save register ebx
        xor ebx, ebx                    ; ebx = argument 1 (zero)
        mov ecx, [esp+8]                ; ecx = argument 2
        mov ah, 0xA%1                   ; ah = syscall number
        int 0x20                        ; boot loader syscall
        jnc short .end
        mov eax, 0xFFFFFFFF             ; eax = B_RET_ERROR
.end:   pop ebx                         ; restore register ebx
        ret
%endmacro

%macro boot_loader_2 1
align 16
global _b_a%1
_b_a%1:
        push ebx                        ; save register ebx
        mov ebx, [esp+8]                ; ebx = argument 1
        mov ecx, [esp+12]               ; ecx = argument 2
        mov ah, 0xA%1                   ; ah = syscall number
        int 0x20                        ; boot loader syscall
        jnc short .end
        mov eax, 0xFFFFFFFF             ; eax = B_RET_ERROR
.end:   pop ebx                         ; restore register ebx
        ret
%endmacro

;       unsigned long b_a0(const char *, unsigned int);
;       unsigned long b_a1(const char *, unsigned int);
;       unsigned long b_a2(unsigned int, unsigned int);
;       unsigned long b_a3(void);
;       unsigned long b_a4(void);
;       unsigned long b_a5(unsigned char);
;       unsigned long b_a6(void);
;       unsigned long b_a7(unsigned char);
;       unsigned long b_a8(unsigned int);
;       unsigned long b_a9(void *, unsigned int);
;       unsigned long b_aa(void *, unsigned int);
;       unsigned long b_ab(unsigned int, unsigned int);
;       unsigned long b_ac(void *, unsigned int);
;       unsigned long b_ad(unsigned int, unsigned int);
;       unsigned long b_ae(void);
;       unsigned long b_af(void);
;
;       b_output_string         (b_a0)
;       b_output_string_hl      (b_a1)
;       b_output_control        (b_a2)
;       b_get_keycode           (b_a3)
;       b_get_byte_com1         (b_a4)
;       b_put_byte_com1         (b_a5)
;       b_get_byte_com2         (b_a6)
;       b_put_byte_com2         (b_a7)
;       b_get_parameter         (b_a8)
;       b_get_structure         (b_a9)
;       b_set_read_buffer       (b_aa)
;       b_read_blocks           (b_ab)
;       b_set_write_buffer      (b_ac)
;       b_write_blocks          (b_ad)
;       b_pause                 (b_ae)
;       b_exit                  (b_af)

boot_loader_2 0
boot_loader_2 1
boot_loader_2 2
boot_loader_0 3
boot_loader_0 4
boot_loader_1 5
boot_loader_0 6
boot_loader_1 7
boot_loader_1 8
boot_loader_2 9
boot_loader_2 a
boot_loader_2 b
boot_loader_2 c
boot_loader_2 d
boot_loader_0 e
boot_loader_0 f

;       unsigned long b_e0(void);
;       unsigned long b_e1(void *);
;
;       b_get_loader_type       (b_e0)
;       b_get_time              (b_e1)

align 16
global _b_e0
_b_e0:
        mov ah, 0xE0                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        jnc short .end
        xor eax, eax                    ; eax = 0
        xor edx, edx                    ; edx = 0
.end:   ret

align 16
global _b_e1
_b_e1:
        push ebx                        ; save register ebx
        xor ebx, ebx                    ; ebx = argument 1 (zero)
        mov ecx, [esp+8]                ; ecx = argument 2
        mov ah, 0xE1                    ; ah = syscall number
        int 0x20                        ; boot loader syscall
        jnc short .end
        xor eax, eax                    ; eax = 0
        xor edx, edx                    ; edx = 0
        mov [ecx+0x00], eax             ; struct b_time ecx = { 0 }
        mov [ecx+0x04], edx
        mov [ecx+0x08], eax
        mov [ecx+0x0C], edx
.end:   pop ebx                         ; restore register ebx
        ret
