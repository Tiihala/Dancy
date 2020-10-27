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
;; init/a64/cpu.asm
;;      Miscellaneous assembly functions (64-bit)
;;

        bits 64

section .text

        global cpu_id
        global cpu_halt
        global cpu_ints
        global cpu_rdtsc
        global cpu_rdtsc_delay
        global cpu_rdtsc_diff
        global cpu_in8
        global cpu_in16
        global cpu_in32
        global cpu_out8
        global cpu_out16
        global cpu_out32
        global cpu_read8
        global cpu_read16
        global cpu_read32
        global cpu_write8
        global cpu_write16
        global cpu_write32

align 16
        ; void cpu_id(uint32_t *a, uint32_t *c, uint32_t *d, uint32_t *b)
cpu_id:
        push rcx                        ; save register rcx
        push rdx                        ; save register rdx
        push rbx                        ; save register rbx

        mov eax, [rcx]                  ; eax = *a
        mov ecx, [rdx]                  ; ecx = *c
        mov edx, [r8]                   ; edx = *d
        mov ebx, [r9]                   ; ebx = *b

        cpuid                           ; cpu identification

        mov [r9], ebx                   ; *b = ebx
        mov [r8], edx                   ; *d = edx
        pop rbx                         ; restore register rbx
        pop rdx                         ; restore register rdx

        mov [rdx], ecx                  ; *c = ecx
        pop rcx                         ; restore register rcx
        mov [rcx], eax                  ; *a = eax
        ret

align 16
        ; void cpu_halt(uint32_t counter)
cpu_halt:
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
cpu_ints:
        pushfq                          ; push rflags
        mov eax, [rsp]                  ; eax = rflags (low dword)
        and eax, 0x00000200             ; eax = current interrupt flag << 9
        shr eax, 9                      ; eax = current interrupt flag
        or dword [rsp], 0x00000200      ; set interrupt flag
        test ecx, ecx                   ; check input
        jnz short .end
        xor dword [rsp], 0x00000200     ; clear interrupt flag
.end:   popfq                           ; pop rflags
        ret

align 16
        ; void cpu_rdtsc(uint32_t *a, uint32_t *d)
cpu_rdtsc:
        push rdx                        ; save register rdx
        rdtsc                           ; read time-stamp counter
        mov [rcx], eax                  ; *a = eax
        pop rax                         ; restore register rdx to rax
        mov [rax], edx                  ; *d = edx
        ret

align 16
        ; void cpu_rdtsc_delay(uint32_t a, uint32_t d)
cpu_rdtsc_delay:
        push rbx                        ; save register rbx
        test edx, edx                   ; test highest bit (input d)
        js short .end
        mov r8d, ecx                    ; r8d = a
        mov r9d, edx                    ; r9d = d
        rdtsc                           ; read time-stamp counter
        mov ecx, eax                    ; ecx = counter (low dword)
        mov ebx, edx                    ; ebx = counter (high dword)
.spin:  rdtsc                           ; read time-stamp counter
        sub eax, ecx                    ; eax = difference (low dword)
        sbb edx, ebx                    ; edx = difference (high dword)
        sub eax, r8d                    ; compare to input a
        sbb edx, r9d                    ; compare to input d
        jc short .spin
.end:   pop rbx                         ; restore register rbx
        ret

align 16
        ; void cpu_rdtsc_diff(uint32_t *a, uint32_t *d)
cpu_rdtsc_diff:
        push rdx                        ; save register rdx
        rdtsc                           ; read time-stamp counter
        sub eax, [rcx]                  ; eax -= *a
        mov [rcx], eax                  ; *a = eax
        pop rax                         ; restore register rdx to rax
        sbb edx, [rax]                  ; edx -= *d
        mov [rax], edx                  ; *d = edx
        ret

align 16
        ; uint8_t cpu_in8(uint16_t port)
cpu_in8:
        mov edx, ecx                    ; dx = port
        in al, dx                       ; input from port
        and eax, 0xFF                   ; clear upper bits
        ret

align 16
        ; uint16_t cpu_in16(uint16_t port)
cpu_in16:
        mov edx, ecx                    ; dx = port
        in ax, dx                       ; input from port
        and eax, 0xFFFF                 ; clear upper bits
        ret

align 16
        ; uint32_t cpu_in32(uint16_t port)
cpu_in32:
        mov edx, ecx                    ; dx = port
        in eax, dx                      ; input from port
        ret

align 16
        ; void cpu_out8(uint16_t port, uint8_t value)
cpu_out8:
        mov eax, edx                    ; al = value
        mov edx, ecx                    ; dx = port
        out dx, al                      ; output to port
        ret

align 16
        ; void cpu_out16(uint16_t port, uint16_t value)
cpu_out16:
        mov eax, edx                    ; ax = value
        mov edx, ecx                    ; dx = port
        out dx, ax                      ; output to port
        ret

align 16
        ; void cpu_out32(uint16_t port, uint32_t value)
cpu_out32:
        mov eax, edx                    ; eax = value
        mov edx, ecx                    ; dx = port
        out dx, eax                     ; output to port
        ret

align 16
        ; uint8_t cpu_read8(const void *address)
cpu_read8:
        mov al, [rcx]                   ; al = value
        and eax, 0xFF                   ; clear upper bits
        ret

align 16
        ; uint16_t cpu_read16(const void *address)
cpu_read16:
        mov ax, [rcx]                   ; ax = value
        and eax, 0xFFFF                 ; clear upper bits
        ret

align 16
        ; uint32_t cpu_read32(const void *address)
cpu_read32:
        mov eax, [rcx]                  ; eax = value
        ret

align 16
        ; void cpu_write8(void *address, uint8_t value)
cpu_write8:
        mov [rcx], dl                   ; write
        ret

align 16
        ; void cpu_write16(void *address, uint16_t value)
cpu_write16:
        mov [rcx], dx                   ; write
        ret

align 16
        ; void cpu_write32(void *address, uint32_t value)
cpu_write32:
        mov [rcx], edx                  ; write
        ret
