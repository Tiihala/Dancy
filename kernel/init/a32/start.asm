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
;; init/a32/start.asm
;;      Entry point (32-bit)
;;

        bits 32

section .text

        extern _init
        global _start

align 16
_start:
        push eax                        ; esp -= 4
        push edx                        ; esp -= 4
        mov eax, esp                    ; eax = esp
        sub al, 0xE8                    ; stack check
        jnz short .halt

        mov ah, 0xE0                    ; get boot loader type
        int 0x20                        ; boot loader syscall
        jc short .map                   ; type 0 if syscall was not supported
        mov [_boot_loader_type], eax    ; save boot loader type

.map:   add ebx, 0x00010000             ; ebx = "address of memory map"
        push dword 0                    ; push 0x00000000
        push ebx                        ; "void init(void *)"
        xor ebx, ebx                    ; ebx = 0
        call _init                      ; call init

.halt:  mov ah, 0xAE                    ; b_pause
        int 0x20                        ; boot loader syscall
        jmp short .halt


section .data

        global _boot_loader_type

align 4
_boot_loader_type:
        dd 0x00000000
