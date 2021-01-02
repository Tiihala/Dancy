;;
;; Copyright (c) 2018, 2019, 2020 Antti Tiihala
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

        extern b_pause
        extern boot_loader_type
        extern start_init
        extern uefi_syscalls
        global start

align 16
start:
        push rax                        ; esp -= 8
        push rdx                        ; esp -= 8
        mov eax, esp                    ; eax = esp
        sub al, 0xE0                    ; stack check
        jnz short .halt

        add ebx, 0x00010000             ; rbx = "address of memory map"
        mov ecx, ebx                    ; "void start_init(void *)"

        test byte [ebx+4], 2            ; test bios loader bit
        jnz short .L2
        mov edx, boot_loader_type       ; address of "boot_loader_type"
        mov byte [rdx], 1               ; set the boot loader type (uefi)
.L1:    lea ebx, [ebx+32]               ; next entry
        cmp dword [ebx], 0x80000005     ; test B_MEM_UEFI_SYSCALLS
        jne short .L1
        mov eax, [ebx+8]                ; eax = base
        mov edx, uefi_syscalls          ; address of "uefi_syscalls"
        mov [rdx], eax                  ; modify the uefi_syscalls variable
.L2:    ; nop

        xor ebx, ebx                    ; rbx = 0
        call start_init                 ; call start_init

.pause: call b_pause                    ; "unsigned long b_pause(void)"
        jmp short .pause

.halt:  hlt                             ; halt
        jmp short .halt
