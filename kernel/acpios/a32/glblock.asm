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
;; acpios/a32/glblock.asm
;;      Acquire and release ACPI Global Lock (32-bit)
;;

        bits 32

section .text

        global _acpios_acquire_global_lock
        global _acpios_release_global_lock

align 16
        ; BOOLEAN acpios_acquire_global_lock(void *facs)
_acpios_acquire_global_lock:
        push ebx                        ; save register ebx
        mov ebx, [esp+8]                ; ebx = facs pointer

        or eax, 0xFFFFFFFF              ; eax = 0xFFFFFFFF
        test ebx, ebx                   ; test zero
        jz short .end                   ; (return true)
        lea ebx, [ebx+16]               ; ebx = global lock pointer

        ; The following algorithm is from
        ; ACPI Specification, Version 6.2

.try:   mov eax, [ebx]                  ; eax = global lock
        mov ecx, eax                    ; ecx = global lock
        and ecx, 0xFFFFFFFE             ; clear pending bit 0
        bts ecx, 1                      ; check and set owner bit 1
        adc ecx, 0                      ; set pending bit if owner bit was set

        lock cmpxchg [ebx], ecx         ; update the global lock value
        jnz short .try                  ; (try again if needed)

        sub cl, 0x03                    ; test the status
        sbb eax, eax                    ; eax = 0xFFFFFFFF (true) or 0 (false)

.end:   pop ebx                         ; restore register ebx
        ret

align 16
        ; BOOLEAN acpios_release_global_lock(void *facs)
_acpios_release_global_lock:
        push ebx                        ; save register ebx
        mov ebx, [esp+8]                ; ebx = facs pointer

        xor eax, eax                    ; eax = 0x00000000
        test ebx, ebx                   ; test zero
        jz short .end                   ; (return false)
        lea ebx, [ebx+16]               ; ebx = global lock pointer

        ; The following algorithm is from
        ; ACPI Specification, Version 6.2

.try:   mov eax, [ebx]                  ; eax = global lock
        mov ecx, eax                    ; ecx = global lock
        and ecx, 0xFFFFFFFC             ; clear owner and pending bits

        lock cmpxchg [ebx], ecx         ; update the global lock value
        jnz short .try                  ; (try again if needed)

        and eax, 0x00000001             ; eax = 1 (true) or 0 (false)

.end:   pop ebx                         ; restore register ebx
        ret
