;;
;; Copyright (c) 2020 Antti Tiihala
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
;; init/a64/gdt.asm
;;      Global Descriptor Table
;;

        bits 64

section .text

        global gdt_init

align 16
        ; void gdt_init(void)
gdt_init:
        push rbx                        ; save register rbx

        mov ebx, gdt_pointer            ; ebx = address of gdt_pointer
        lgdt [rbx]                      ; load global descriptor table

        xor ebx, ebx                    ; ebx = 0 (segment selector)
        lldt bx                         ; load local descriptor table

        mov ebx, 0x10                   ; ebx = data segment
        mov es, ebx                     ; set segment register es
        mov ss, ebx                     ; set segment register ss
        mov ds, ebx                     ; set segment register ds

        xor ebx, ebx                    ; ebx = null segment
        mov fs, ebx                     ; set segment register fs
        mov gs, ebx                     ; set segment register gs

        mov ebx, seg_cs_flush           ; ebx = address of seg_cs_flush
        db 0xFF, 0x2B                   ; jmp far [rbx] (32-bit)

gdt_init_end:
        pop rbx                         ; restore register rbx
        ret


section .data

align 16
gdt_table:
        db 0x00, 0x00, 0x00, 0x00       ; 0x00 (null)
        db 0x00, 0x00, 0x00, 0x00
        db 0xFF, 0xFF, 0x00, 0x00       ; 0x08 (code)
        db 0x00, 0x9B, 0xAF, 0x00
        db 0xFF, 0xFF, 0x00, 0x00       ; 0x10 (data)
        db 0x00, 0x93, 0xCF, 0x00
gdt_table_end:

align 16
        dw 0, 0, 0
gdt_pointer:
        dw ((gdt_table_end - gdt_table) - 1)
        dd gdt_table
        dd 0

seg_cs_flush:
        dd (gdt_init_end)
        dw 0x08
