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
;; lib/a64/spin.asm
;;      Spinlock implementation
;;

        bits 64

section .text

        global spin_enter
        global spin_leave
        global spin_lock
        global spin_unlock

align 16
        ; void spin_enter(void **lock_local)
spin_enter:
        pushfq                          ; push flags
        mov eax, [rsp]                  ; eax = flags
        and eax, 0x200                  ; eax = interrupt flag << 9
        shr eax, 9                      ; eax = interrupt flag

        mov rdx, rcx                    ; rdx = lock_local
        mov rcx, [rdx]                  ; rcx = *lock_local
        and rcx, -2                     ; rcx = "lock" for spin_lock

        or rax, rcx                     ; rax = *lock_local + interrupt flag
        mov [rdx], rax                  ; write updated value

        cli                             ; disable interrupts
        call spin_lock                  ; use spin_lock function
        add rsp, 8                      ; pop flags
        ret

align 16
        ; void spin_leave(void **lock_local)
spin_leave:
        push rbx                        ; save register rbx
        mov rbx, rcx                    ; rbx = lock_local
        mov rcx, [rbx]                  ; rcx = *lock_local
        and rcx, -2                     ; rcx = "lock" for spin_unlock

        call spin_unlock                ; use spin_unlock function
        mov rax, [rbx]                  ; rax = *lock_local
        test eax, 1                     ; test interrupt flag
        jz short .L1

        sti                             ; enable interrupts
.L1:    and rax, -2                     ; clear interrupt flag
        mov [rbx], rax                  ; write updated value
        pop rbx                         ; restore register rbx
        ret

align 32
        ; void spin_lock(int *lock)
spin_lock:
        mov edx, 0x00000001             ; edx = 1
.L1:    xor eax, eax                    ; eax = 0
        lock cmpxchg [rcx], edx         ; try to acquire the lock
        jnz short .L2                   ; jump if already locked
        ret

align 16
.L2:    cmp dword [rcx], 0              ; test zero
        je short .L1                    ; try again
        pause                           ; improve performance
        jmp short .L2

align 16
        ; void spin_unlock(int *lock)
spin_unlock:
        mov eax, 0x00000001             ; eax = 1
        xor edx, edx                    ; edx = 0
        lock cmpxchg [rcx], edx         ; try to release the lock
        ret
