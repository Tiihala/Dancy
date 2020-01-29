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
;; acpios/a64/glblock.asm
;;      Acquire and release ACPI Global Lock (64-bit)
;;

        bits 64

section .text

        global acpios_acquire_global_lock
        global acpios_release_global_lock

align 16
        ; BOOLEAN acpios_acquire_global_lock(void *facs)
acpios_acquire_global_lock:
        push rbx                        ; save register rbx
        mov rbx, rcx                    ; rbx = facs pointer

        or eax, 0xFFFFFFFF              ; eax = 0xFFFFFFFF
        test rbx, rbx                   ; test zero
        jz short .end                   ; (return true)
        lea rbx, [rbx+16]               ; rbx = global lock pointer

        ; The following algorithm is from
        ; ACPI Specification, Version 6.2

.try:   mov eax, [rbx]                  ; eax = global lock
        mov ecx, eax                    ; ecx = global lock
        and ecx, 0xFFFFFFFE             ; clear pending bit 0
        bts ecx, 1                      ; check and set owner bit 1
        adc ecx, 0                      ; set pending bit if owner bit was set

        lock cmpxchg [rbx], ecx         ; update the global lock value
        jnz short .try                  ; (try again if needed)

        sub cl, 0x03                    ; test the status
        sbb eax, eax                    ; eax = 0xFFFFFFFF (true) or 0 (false)

.end:   pop rbx                         ; restore register rbx
        ret

align 16
        ; BOOLEAN acpios_release_global_lock(void *facs)
acpios_release_global_lock:
        push rbx                        ; save register rbx
        mov rbx, rcx                    ; rbx = facs pointer

        xor eax, eax                    ; eax = 0x00000000
        test rbx, rbx                   ; test zero
        jz short .end                   ; (return false)
        lea rbx, [rbx+16]               ; rbx = global lock pointer

        ; The following algorithm is from
        ; ACPI Specification, Version 6.2

.try:   mov eax, [rbx]                  ; eax = global lock
        mov ecx, eax                    ; ecx = global lock
        and ecx, 0xFFFFFFFC             ; clear owner and pending bits

        lock cmpxchg [rbx], ecx         ; update the global lock value
        jnz short .try                  ; (try again if needed)

        and eax, 0x00000001             ; eax = 1 (true) or 0 (false)

.end:   pop rbx                         ; restore register rbx
        ret
