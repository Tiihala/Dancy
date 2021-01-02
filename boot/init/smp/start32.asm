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
;; init/smp/start32.asm
;;      Trampoline code (32-bit)
;;
;; Build instructions
;;      nasm -f bin -o start32.bin start32.asm
;;

        org 0
        bits 16

smp_trampoline:
        lidt [cs:smp_idt_null]          ; load null idt
        wbinvd                          ; write back and invalidate cache

        db 0xB8, 0x41, 0x54             ; 16-bit mode test
        jmp short smp_entry16           ; (16-bit mode continues)
.L1:    hlt                             ; (32-bit mode)
        jmp short .L1                   ; (32-bit mode halts)

smp_entry16:
        cli                             ; disable interrupts
        mov bx, cs                      ; bx = code segment
        mov ds, bx                      ; set data segment
        and ebx, 0x0000FFFF             ; clear upper bits
        shl ebx, 4                      ; ebx = base address

        mov eax, ebx                    ; eax = base address
        add eax, gdt_table              ; eax = address of "gdt_table"
        mov [gdt_offset], eax           ; relocate the offset

        mov eax, ebx                    ; eax = base address
        add eax, smp_entry32            ; eax = address of "smp_entry32"
        mov [smp_entry32_far_jmp], eax  ; relocate the offset

        mov eax, cr0                    ; eax = cr0
        or ax, 1                        ; enable protected mode
        lgdt [gdt_pointer]              ; load gdt
        mov cr0, eax                    ; update cr0
        jmp near .L2                    ; extra jump

.L2:    db 0x66, 0xFF, 0x2E             ; jmp dword far [smp_entry32_far_jmp]
        dw smp_entry32_far_jmp

        bits 32

times 0x50 - ($ - $$) db 0
smp_entry32:
        db 0xB8, 0x41, 0x54             ; 32-bit mode test
        db 0xEB, 0xFE                   ; (16-bit mode stops)

        mov eax, 0x10                   ; eax = data selector
        mov es, eax                     ; set extra segment
        mov ds, eax                     ; set data segment
        mov fs, eax                     ; set fs
        mov gs, eax                     ; set gs

        mov ss, eax                     ; set stack segment
        mov esp, [ebx+smp_ap_stack]     ; load stack pointer
        lea edi, [ebx+smp_ap_status]    ; edi = address of "smp_ap_status"

.L1:    mov eax, 0                      ; eax = 0
        mov edx, 1                      ; edx = 1
        lock cmpxchg [edi], edx         ; smp_ap_status = 1
        jnz short .L1                   ; (must not happen)

.L2:    mov eax, 2                      ; eax = 2
        mov edx, 3                      ; edx = 3
        lock cmpxchg [edi], edx         ; smp_ap_status = 3
        jz short smp_approved           ; wait until bsp sets status 2

.L3:    cmp dword [edi], 2              ; test status value
        je near .L2                     ; try again (long instruction)
        pause                           ; improve performance
        jmp near .L3                    ; (long instruction)

smp_approved:
        mov eax, [ebx+smp_ap_paging]    ; eax = cr3 value
        mov cr3, eax                    ; load paging tables

        mov eax, cr0                    ; eax = cr0
        bts eax, 31                     ; enable paging
        mov cr0, eax                    ; update cr0

        cld                             ; clear direction flag
        mov eax, 0                      ; eax = 0
        xchg eax, [esp]                 ; eax = entry point
        jmp eax                         ; jump to entry point

times 0x100 - ($ - $$) db 0

        dw 0
smp_idt_null:
        dw 0                            ; limit 0
        dd 0                            ; base 0

        dw 0
gdt_pointer:
        dw ((gdt_table_end - gdt_table) - 1)
gdt_offset:
        dd gdt_table                    ; (relocated)

gdt_table:
        db 0x00, 0x00, 0x00, 0x00       ; 0x00 (null)
        db 0x00, 0x00, 0x00, 0x00
        db 0xFF, 0xFF, 0x00, 0x00       ; 0x08 (code)
        db 0x00, 0x9B, 0xCF, 0x00
        db 0xFF, 0xFF, 0x00, 0x00       ; 0x10 (data)
        db 0x00, 0x93, 0xCF, 0x00
gdt_table_end:

smp_entry32_far_jmp:
        dd 0                            ; (relocated)
        dw 0x08                         ; (code)

times 0x1F0 - ($ - $$) db 0

; struct smp_ap_info {
;         uint32_t ap_stack;
;         uint32_t ap_paging;
;         uint32_t ap_status;
; };

smp_ap_stack:   dd 0
smp_ap_paging:  dd 0
smp_ap_status:  dd 0

times 0x200 - ($ - $$) db 0
