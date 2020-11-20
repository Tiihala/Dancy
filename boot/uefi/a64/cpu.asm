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
;; uefi/a64/cpu.asm
;;      Miscellaneous assembly functions (64-bit)
;;

        bits 64

section .text

        global cpu_rdtsc
        global cpu_rdtsc_delay
        global cpu_rdtsc_diff

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
