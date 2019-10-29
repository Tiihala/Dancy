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
;; init/a64/start.asm
;;      Entry point (64-bit)
;;

        bits 64

section .text

        extern init
        global start

align 16
start:
        push rax                        ; esp -= 8
        push rdx                        ; esp -= 8
        mov eax, esp                    ; eax = esp
        sub al, 0xE0                    ; stack check
        jnz short .halt

        add ebx, 0x00010000             ; rbx = "address of memory map"
        mov ecx, ebx                    ; "void init(void *)"

        test byte [ebx+4], 4            ; test uefi bit
        jz short .L2
.L1:    lea ebx, [ebx+32]               ; next entry
        test byte [ebx+4], 2            ; test B_FLAG_VALID_LEGACY
        jz short .halt
        cmp dword [ebx], 0x80000005     ; test B_MEM_UEFI_SYSCALLS
        jne short .L1
        mov eax, [ebx+8]                ; eax = base
        mov [uefi_syscalls], eax        ; modify the uefi_syscalls variable
.L2:    ; nop

        xor ebx, ebx                    ; rbx = 0
        call init                       ; call init

.halt:  hlt                             ; halt
        jmp short .halt


section .data

        global boot_loader_type
        global uefi_syscalls

align 4
boot_loader_type:
        dd 0x00000000
uefi_syscalls:
        dd 0x00000000
