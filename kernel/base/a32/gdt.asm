;;
;; Copyright (c) 2021 Antti Tiihala
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
;; base/a32/gdt.asm
;;      Global Descriptor Table
;;

        bits 32

section .text

        global _gdt_load
        global _gdt_load_cs
        global _gdt_load_es
        global _gdt_load_ss
        global _gdt_load_ds
        global _gdt_load_fs
        global _gdt_load_gs
        global _gdt_load_tss

align 16
        ; void gdt_load(const void *gdt_ptr)
_gdt_load:
        mov ecx, [esp+4]                ; ecx = gdt_ptr
        lgdt [ecx]                      ; load global descriptor table
        xor ecx, ecx                    ; ecx = 0 (segment selector)
        lldt cx                         ; load local descriptor table
        ret

align 16
        ; void gdt_load_cs(int sel)
_gdt_load_cs:
        push ebx                        ; save register ebx
        mov eax, _gdt_load_cs_end       ; eax = address of _gdt_load_cs_end
        mov ecx, [esp+8]                ; ecx = sel

        sub esp, 8                      ; decrement stack pointer
        mov ebx, esp                    ; ebx = stack pointer
        mov [ebx+0], eax                ; offset
        mov [ebx+4], ecx                ; selector
        db 0xFF, 0x2B                   ; jmp far [ebx]

_gdt_load_cs_end:
        add esp, 8                      ; restore stack pointer
        pop ebx                         ; restore register ebx
        ret

align 16
        ; void gdt_load_es(int sel)
_gdt_load_es:
        mov ecx, [esp+4]                ; ecx = sel
        mov es, ecx                     ; set segment register es
        ret

align 16
        ; void gdt_load_ss(int sel)
_gdt_load_ss:
        mov ecx, [esp+4]                ; ecx = sel
        mov ss, ecx                     ; set segment register ss
        ret

align 16
        ; void gdt_load_ds(int sel)
_gdt_load_ds:
        mov ecx, [esp+4]                ; ecx = sel
        mov ds, ecx                     ; set segment register ds
        ret

align 16
        ; void gdt_load_fs(int sel)
_gdt_load_fs:
        mov ecx, [esp+4]                ; ecx = sel
        mov fs, ecx                     ; set segment register fs
        ret

align 16
        ; void gdt_load_gs(int sel)
_gdt_load_gs:
        mov ecx, [esp+4]                ; ecx = sel
        mov gs, ecx                     ; set segment register gs
        ret

align 16
        ; void gdt_load_tss(int sel)
_gdt_load_tss:
        mov ecx, [esp+4]                ; ecx = sel
        ltr cx                          ; load task register
        ret
