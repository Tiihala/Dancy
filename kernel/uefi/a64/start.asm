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
;; uefi/a64/start.asm
;;      Boot Loader (UEFI)
;;

        bits 64

%define EFI_BootServices                0x60
%define EFI_OutputString                0x08
%define EFI_ConOut                      0x40

section .text

        global start

align 16
        ; void start(uint64_t, uint64_t)
start:
        xor eax, eax                    ; rax = 0
        push rax                        ; push 0
        sub rax, 16                     ; rax = 0xFFFFFFFFFFFFFFF0
        and rsp, rax                    ; align stack pointer

        mov rcx, rdx                    ; rcx = "SystemTablePointer"
        mov rcx, [rcx+EFI_ConOut]       ; rcx = "ConOut"
        lea rdx, [rel hello_world]      ; rdx = address of message
        sub rsp, 32                     ; stack shadow space (allocate)
        call [rcx+EFI_OutputString]     ; call UEFI function
        add rsp, 32                     ; stack shadow space (free)
        db 0x90, 0xF4, 0xF4, 0xEB, 0xFC ; endless loop

section .data

hello_world:
        dw __utf16__('hello, uefi world'), 13, 10, 0
