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
;; lib/a32/spin.asm
;;      Spinlock implementation
;;

        bits 32

section .text

        global _spin_lock
        global _spin_unlock

align 32
        ; void spin_lock(int *lock)
_spin_lock:
        mov ecx, [esp+4]                ; ecx = lock
.L1:    xor eax, eax                    ; eax = 0
        lea edx, [eax+1]                ; edx = 1
        lock cmpxchg [ecx], edx         ; try to acquire the lock
        jnz short .L2                   ; jump if already locked
        ret

align 16
.L2:    cmp dword [ecx], 0              ; test zero
        je short .L1                    ; try again
        pause                           ; improve performance
        jmp short .L2

align 16
        ; void spin_unlock(int *lock)
_spin_unlock:
        mov ecx, [esp+4]                ; ecx = lock
        mov eax, 0x00000001             ; eax = 1
        xor edx, edx                    ; edx = 0
        lock cmpxchg [ecx], edx         ; try to release the lock
        ret
