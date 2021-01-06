;;
;; Copyright (c) 2019, 2020 Antti Tiihala
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
;; lib/a32/cpu.asm
;;      Miscellaneous assembly functions (32-bit)
;;

        bits 32

section .text

        global _cpu_id
        global _cpu_halt
        global _cpu_ints
        global _cpu_rdtsc
        global _cpu_rdtsc_delay
        global _cpu_rdtsc_diff
        global _cpu_rdmsr
        global _cpu_wrmsr
        global _cpu_in8
        global _cpu_in16
        global _cpu_in32
        global _cpu_out8
        global _cpu_out16
        global _cpu_out32
        global _cpu_read8
        global _cpu_read16
        global _cpu_read32
        global _cpu_write8
        global _cpu_write16
        global _cpu_write32

align 16
        ; void cpu_id(uint32_t *a, uint32_t *c, uint32_t *d, uint32_t *b)
_cpu_id:
        push ebx                        ; save register ebx
        push esi                        ; save register esi

        mov esi, [esp+12]               ; esi = a
        mov eax, [esi]                  ; eax = *a
        mov esi, [esp+16]               ; esi = c
        mov ecx, [esi]                  ; ecx = *c
        mov esi, [esp+20]               ; esi = d
        mov edx, [esi]                  ; edx = *d
        mov esi, [esp+24]               ; esi = b
        mov ebx, [esi]                  ; ebx = *b

        cpuid                           ; cpu identification

        mov esi, [esp+12]               ; esi = a
        mov [esi], eax                  ; *a = eax
        mov esi, [esp+16]               ; esi = c
        mov [esi], ecx                  ; *c = ecx
        mov esi, [esp+20]               ; esi = d
        mov [esi], edx                  ; *d = edx
        mov esi, [esp+24]               ; esi = b
        mov [esi], ebx                  ; *b = ebx

        pop esi                         ; restore register esi
        pop ebx                         ; restore register ebx
        ret

align 16
        ; void cpu_halt(uint32_t counter)
_cpu_halt:
        mov ecx, [esp+4]                ; ecx = counter
        test ecx, ecx                   ; zero is infinite
        jnz short .spin2
.spin1: hlt                             ; halt instruction
        jmp short .spin1
.spin2: hlt                             ; halt instruction
        dec ecx                         ; decrement counter
        jnz short .spin2
        ret

align 16
        ; int cpu_ints(int enable)
_cpu_ints:
        pushfd                          ; push eflags
        mov eax, [esp]                  ; eax = eflags
        and eax, 0x00000200             ; eax = current interrupt flag << 9
        shr eax, 9                      ; eax = current interrupt flag
        or dword [esp], 0x00000200      ; set interrupt flag
        cmp dword [esp+8], 0            ; check input
        jne short .end
        xor dword [esp], 0x00000200     ; clear interrupt flag
.end:   popfd                           ; pop eflags
        ret

align 16
        ; void cpu_rdtsc(uint32_t *a, uint32_t *d)
_cpu_rdtsc:
        rdtsc                           ; read time-stamp counter
        mov ecx, [esp+4]                ; ecx = a
        mov [ecx], eax                  ; *a = eax
        mov ecx, [esp+8]                ; ecx = d
        mov [ecx], edx                  ; *d = edx
        ret

align 16
        ; void cpu_rdtsc_delay(uint32_t a, uint32_t d)
_cpu_rdtsc_delay:
        push ebx                        ; save register ebx
        test dword [esp+12], 0x80000000 ; test highest bit (input d)
        jnz short .end
        rdtsc                           ; read time-stamp counter
        mov ecx, eax                    ; ecx = counter (low dword)
        mov ebx, edx                    ; ebx = counter (high dword)
.spin:  rdtsc                           ; read time-stamp counter
        sub eax, ecx                    ; eax = difference (low dword)
        sbb edx, ebx                    ; edx = difference (high dword)
        sub eax, [esp+8]                ; compare to input a
        sbb edx, [esp+12]               ; compare to input d
        jc short .spin
.end:   pop ebx                         ; restore register ebx
        ret

align 16
        ; void cpu_rdtsc_diff(uint32_t *a, uint32_t *d)
_cpu_rdtsc_diff:
        rdtsc                           ; read time-stamp counter
        mov ecx, [esp+4]                ; ecx = a
        sub eax, [ecx]                  ; eax -= *a
        mov [ecx], eax                  ; *a = eax
        mov ecx, [esp+8]                ; ecx = d
        sbb edx, [ecx]                  ; edx -= *d
        mov [ecx], edx                  ; *d = edx
        ret

align 16
        ; void cpu_rdmsr(uint32_t msr, uint32_t *a, uint32_t *d)
_cpu_rdmsr:
        mov ecx, [esp+4]                ; ecx = model specific register
        rdmsr                           ; read the register
        mov ecx, [esp+8]                ; ecx = a
        mov [ecx], eax                  ; *a = eax
        mov ecx, [esp+12]               ; ecx = d
        mov [ecx], edx                  ; *d = edx
        ret

align 16
        ; void cpu_wrmsr(uint32_t msr, uint32_t a, uint32_t d)
_cpu_wrmsr:
        mov ecx, [esp+4]                ; ecx = model specific register
        mov eax, [esp+8]                ; eax = a
        mov edx, [esp+12]               ; edx = d
        wrmsr                           ; write the register
        ret

align 16
        ; uint8_t cpu_in8(uint16_t port)
_cpu_in8:
        mov edx, [esp+4]                ; dx = port
        in al, dx                       ; input from port
        and eax, 0xFF                   ; clear upper bits
        ret

align 16
        ; uint16_t cpu_in16(uint16_t port)
_cpu_in16:
        mov edx, [esp+4]                ; dx = port
        in ax, dx                       ; input from port
        and eax, 0xFFFF                 ; clear upper bits
        ret

align 16
        ; uint32_t cpu_in32(uint16_t port)
_cpu_in32:
        mov edx, [esp+4]                ; dx = port
        in eax, dx                      ; input from port
        ret

align 16
        ; void cpu_out8(uint16_t port, uint8_t value)
_cpu_out8:
        mov edx, [esp+4]                ; dx = port
        mov eax, [esp+8]                ; al = value
        out dx, al                      ; output to port
        ret

align 16
        ; void cpu_out16(uint16_t port, uint16_t value)
_cpu_out16:
        mov edx, [esp+4]                ; dx = port
        mov eax, [esp+8]                ; ax = value
        out dx, ax                      ; output to port
        ret

align 16
        ; void cpu_out32(uint16_t port, uint32_t value)
_cpu_out32:
        mov edx, [esp+4]                ; dx = port
        mov eax, [esp+8]                ; eax = value
        out dx, eax                     ; output to port
        ret

align 16
        ; uint8_t cpu_read8(const void *address)
_cpu_read8:
        mov ecx, [esp+4]                ; ecx = address
        mov al, [ecx]                   ; al = value
        and eax, 0xFF                   ; clear upper bits
        ret

align 16
        ; uint16_t cpu_read16(const void *address)
_cpu_read16:
        mov ecx, [esp+4]                ; ecx = address
        mov ax, [ecx]                   ; ax = value
        and eax, 0xFFFF                 ; clear upper bits
        ret

align 16
        ; uint32_t cpu_read32(const void *address)
_cpu_read32:
        mov ecx, [esp+4]                ; ecx = address
        mov eax, [ecx]                  ; eax = value
        ret

align 16
        ; void cpu_write8(void *address, uint8_t value)
_cpu_write8:
        mov ecx, [esp+4]                ; ecx = address
        mov dl, [esp+8]                 ; dl = value
        mov [ecx], dl                   ; write
        wbinvd                          ; flush internal caches
        ret

align 16
        ; void cpu_write16(void *address, uint16_t value)
_cpu_write16:
        mov ecx, [esp+4]                ; ecx = address
        mov dx, [esp+8]                 ; dx = value
        mov [ecx], dx                   ; write
        wbinvd                          ; flush internal caches
        ret

align 16
        ; void cpu_write32(void *address, uint32_t value)
_cpu_write32:
        mov ecx, [esp+4]                ; ecx = address
        mov edx, [esp+8]                ; edx = value
        mov [ecx], edx                  ; write
        wbinvd                          ; flush internal caches
        ret
