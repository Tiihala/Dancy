;;
;; Copyright (c) 2019, 2020, 2021 Antti Tiihala
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
;; lib/a64/cpu.asm
;;      Miscellaneous assembly functions (64-bit)
;;

        bits 64

section .text

        global cpu_id
        global cpu_halt
        global cpu_ints
        global cpu_invlpg
        global cpu_wbinvd
        global cpu_rdtsc
        global cpu_rdtsc_delay
        global cpu_rdtsc_diff
        global cpu_rdmsr
        global cpu_wrmsr
        global cpu_add32
        global cpu_sub32
        global cpu_btr32
        global cpu_bts32
        global cpu_in8
        global cpu_in16
        global cpu_in32
        global cpu_out8
        global cpu_out16
        global cpu_out32
        global cpu_read8
        global cpu_read16
        global cpu_read32
        global cpu_read64
        global cpu_read_cr0
        global cpu_read_cr2
        global cpu_read_cr3
        global cpu_read_cr4
        global cpu_read_flags
        global cpu_write8
        global cpu_write16
        global cpu_write32
        global cpu_write64
        global cpu_write_cr0
        global cpu_write_cr2
        global cpu_write_cr3
        global cpu_write_cr4
        global cpu_xchg

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
        call __serialize_execution      ; (registers preserved)
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
        ; void cpu_invlpg(const void *address)
cpu_invlpg:
        invlpg [rcx]                    ; invalidate tlb entry
        ret

align 16
        ; void cpu_wbinvd(void)
cpu_wbinvd:
        wbinvd                          ; write back and invalidate cache
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
        ; void cpu_rdmsr(uint32_t msr, uint32_t *a, uint32_t *d)
cpu_rdmsr:
        mov r9, rdx                     ; r9 = a
        rdmsr                           ; read the register (ecx = msr)
        mov [r9], eax                   ; *a = eax
        mov [r8], edx                   ; *d = edx
        ret

align 16
        ; void cpu_wrmsr(uint32_t msr, uint32_t a, uint32_t d)
cpu_wrmsr:
        mov eax, edx                    ; eax = a
        mov edx, r8d                    ; edx = d
        wrmsr                           ; write the register (ecx = msr)
        ret

align 16
        ; uint32_t cpu_add32(void *address, uint32_t value)
cpu_add32:
        push rbx                        ; save register rbx
        mov rbx, rcx                    ; rbx = address

.L1:    mov eax, [rbx]                  ; eax = *((uint32_t *)address)
        mov ecx, eax                    ; ecx = current
        add ecx, edx                    ; ecx = current + value
        lock cmpxchg [rbx], ecx         ; try to update
        jnz short .L1                   ; retry if needed

        mov eax, ecx                    ; eax = latest value
        pop rbx                         ; restore register rbx
        ret

align 16
        ; uint32_t cpu_sub32(void *address, uint32_t value)
cpu_sub32:
        push rbx                        ; save register rbx
        mov rbx, rcx                    ; rbx = address

.L1:    mov eax, [rbx]                  ; eax = *((uint32_t *)address)
        mov ecx, eax                    ; ecx = current
        sub ecx, edx                    ; ecx = current - value
        lock cmpxchg [rbx], ecx         ; try to update
        jnz short .L1                   ; retry if needed

        mov eax, ecx                    ; eax = latest value
        pop rbx                         ; restore register rbx
        ret

align 16
        ; int cpu_btr32(void *address, uint32_t value)
cpu_btr32:
        xor eax, eax                    ; eax = 0
        lock btr [rcx], edx             ; bit test and reset
        adc eax, 0                      ; eax = return value
        ret

align 16
        ; int cpu_bts32(void *address, uint32_t value)
cpu_bts32:
        xor eax, eax                    ; eax = 0
        lock bts [rcx], edx             ; bit test and set
        adc eax, 0                      ; eax = return value
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
        call __serialize_execution      ; (registers preserved)
        mov al, [rcx]                   ; al = value
        and eax, 0xFF                   ; clear upper bits
        ret

align 16
        ; uint16_t cpu_read16(const void *address)
cpu_read16:
        call __serialize_execution      ; (registers preserved)
        mov ax, [rcx]                   ; ax = value
        and eax, 0xFFFF                 ; clear upper bits
        ret

align 16
        ; uint32_t cpu_read32(const void *address)
cpu_read32:
        call __serialize_execution      ; (registers preserved)
        mov eax, [rcx]                  ; eax = value
        ret

align 16
        ; uint64_t cpu_read64(const void *address)
cpu_read64:
        call __serialize_execution      ; (registers preserved)
        mov rax, [rcx]                  ; rax = value
        ret

align 16
        ; cpu_native_t cpu_read_cr0(void)
cpu_read_cr0:
        mov rax, cr0                    ; rax = control register cr0
        ret

align 16
        ; cpu_native_t cpu_read_cr2(void)
cpu_read_cr2:
        mov rax, cr2                    ; rax = control register cr2
        ret

align 16
        ; cpu_native_t cpu_read_cr3(void)
cpu_read_cr3:
        mov rax, cr3                    ; rax = control register cr3
        ret

align 16
        ; cpu_native_t cpu_read_cr4(void)
cpu_read_cr4:
        mov rax, cr4                    ; rax = control register cr4
        ret

align 16
        ; cpu_native_t cpu_read_flags(void)
cpu_read_flags:
        pushfq                          ; push rflags
        pop rax                         ; rax = rflags
        ret

align 16
        ; void cpu_write8(void *address, uint8_t value)
cpu_write8:
        call __serialize_execution      ; (registers preserved)
        mov [rcx], dl                   ; write
        call __serialize_execution      ; (registers preserved)
        ret

align 16
        ; void cpu_write16(void *address, uint16_t value)
cpu_write16:
        call __serialize_execution      ; (registers preserved)
        mov [rcx], dx                   ; write
        call __serialize_execution      ; (registers preserved)
        ret

align 16
        ; void cpu_write32(void *address, uint32_t value)
cpu_write32:
        call __serialize_execution      ; (registers preserved)
        mov [rcx], edx                  ; write
        call __serialize_execution      ; (registers preserved)
        ret

align 16
        ; void cpu_write64(void *address, uint64_t value)
cpu_write64:
        call __serialize_execution      ; (registers preserved)
        mov [rcx], rdx                  ; write
        call __serialize_execution      ; (registers preserved)
        ret

align 16
        ; void cpu_write_cr0(cpu_native_t value)
cpu_write_cr0:
        mov cr0, rcx                    ; cr0 = value
        ret

align 16
        ; void cpu_write_cr2(cpu_native_t value)
cpu_write_cr2:
        mov cr2, rcx                    ; cr2 = value
        ret

align 16
        ; void cpu_write_cr3(cpu_native_t value)
cpu_write_cr3:
        mov cr3, rcx                    ; cr3 = value
        ret

align 16
        ; void cpu_write_cr4(cpu_native_t value)
cpu_write_cr4:
        mov cr4, rcx                    ; cr4 = value
        ret

align 16
        ; cpu_native_t cpu_xchg(cpu_native_t *address, cpu_native_t value)
cpu_xchg:
        mov rax, rdx                    ; rax = value
        xchg [rcx], rax                 ; exchange memory with register
        ret

align 16
        ; Internal procedure for serializing instruction execution
        ;
        ; All registers are preserved.
__serialize_execution:
        push rax                        ; save register rax
        push rcx                        ; save register rcx
        push rdx                        ; save register rdx
        push rbx                        ; save register rbx
        xor eax, eax                    ; eax = 0
        cpuid                           ; serializing instruction
        pop rbx                         ; restore register rbx
        pop rdx                         ; restore register rdx
        pop rcx                         ; restore register rcx
        pop rax                         ; restore register rax
        ret
