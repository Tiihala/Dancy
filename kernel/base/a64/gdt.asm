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
;; base/a64/gdt.asm
;;      Global Descriptor Table
;;

        bits 64

section .text

        global gdt_load
        global gdt_load_cs
        global gdt_load_es
        global gdt_load_ss
        global gdt_load_ds
        global gdt_load_fs
        global gdt_load_gs
        global gdt_load_tss

align 16
        ; void gdt_load(const void *gdt_ptr)
gdt_load:
        lgdt [rcx]                      ; load global descriptor table
        xor ecx, ecx                    ; ecx = 0 (segment selector)
        lldt cx                         ; load local descriptor table
        ret

align 16
        ; void gdt_load_cs(int sel)
gdt_load_cs:
        push rbx                        ; save register rbx
        mov eax, gdt_load_cs_end        ; eax = address of gdt_load_cs_end

        sub rsp, 8                      ; decrement stack pointer
        mov rbx, rsp                    ; rbx = stack pointer
        mov [rbx+0], eax                ; offset
        mov [rbx+4], ecx                ; selector
        db 0xFF, 0x2B                   ; jmp far [rbx] (32-bit)

gdt_load_cs_end:
        add rsp, 8                      ; restore stack pointer
        pop rbx                         ; restore register rbx
        ret

align 16
        ; void gdt_load_es(int sel)
gdt_load_es:
        mov es, ecx                     ; set segment register es
        ret

align 16
        ; void gdt_load_ss(int sel)
gdt_load_ss:
        mov ss, ecx                     ; set segment register ss
        ret

align 16
        ; void gdt_load_ds(int sel)
gdt_load_ds:
        mov ds, ecx                     ; set segment register ds
        ret

align 16
        ; void gdt_load_fs(int sel)
gdt_load_fs:
        mov fs, ecx                     ; set segment register fs
        ret

align 16
        ; void gdt_load_gs(int sel)
gdt_load_gs:
        mov gs, ecx                     ; set segment register gs
        ret

align 16
        ; void gdt_load_tss(int sel)
gdt_load_tss:
        ltr cx                          ; load task register
        ret
