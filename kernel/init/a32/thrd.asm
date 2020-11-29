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
;; init/a32/thrd.asm
;;      Threads for pre-kernel environment
;;

        bits 32

section .text

        extern _init_thrd_current
        extern _init_thrd_size
        global _init_thrd_create
        global _thrd_yield

align 16
        ; void init_thrd_create(void *thr, thrd_start_t func, void *arg)
_init_thrd_create:
        push ebx                        ; save register ebx
        mov ebx, [esp+8]                ; ebx = thr
        add ebx, [_init_thrd_size]      ; add stack size (at least align 16)

        mov eax, _init_thrd_return      ; eax = address of init_thrd_return
        lea ebx, [ebx-20]               ; ebx = stack pointer
        mov [ebx], eax                  ; set the return address

        mov eax, [esp+12]               ; eax = func
        lea ebx, [ebx-4]                ; ebx = stack pointer
        mov [ebx], eax                  ; save value

        mov eax, [esp+16]               ; eax = arg
        lea ebx, [ebx-4]                ; ebx = stack pointer
        mov [ebx], eax                  ; save value

        mov eax, _init_thrd_start       ; eax = address of init_thrd_start
        lea ebx, [ebx-4]                ; ebx = stack pointer
        mov [ebx], eax                  ; save value

        mov eax, 0x00000202             ; eax = flags
        lea ebx, [ebx-20]               ; ebx = stack pointer
        mov [ebx], eax                  ; save value

        lea eax, [ebx]                  ; eax = stack pointer for thrd_yield
        mov ebx, [esp+8]                ; ebx = thr

        mov [ebx+4], eax                ; set stack (struct init_thrd)
        pop ebx                         ; restore register ebx
        ret

align 16
_init_thrd_start:
        pop ecx                         ; ecx = arg
        pop eax                         ; eax = func
        mov [esp+4], ecx                ; prepare the first argument
        jmp eax                         ; func(arg)

align 16
_init_thrd_return:
        mov ebx, [_init_thrd_current]   ; ebx = init_thrd_current
        mov [ebx+12], eax               ; write "res"
        mov dword [ebx+16], 1           ; write "terminated"
.L1:    call _thrd_yield                ; call next thread
        jmp short .L1

align 16
        ; void thrd_yield(void)
_thrd_yield:
        push ebx                        ; save register ebx
        push ebp                        ; save register ebp
        push esi                        ; save register esi
        push edi                        ; save register edi
        pushfd                          ; save flags
        cli                             ; disable interrupts

        xor ebx, ebx                    ; ebx = 0
        add ebx, [_init_thrd_current]   ; ebx = init_thrd_current
        jz short .L2

        mov [ebx+4], esp                ; save current stack pointer
.L1:    mov ebx, [ebx]                  ; ebx = next
        cmp dword [ebx+16], 0           ; check "terminated"
        jne short .L1

        mov [_init_thrd_current], ebx   ; change the current pointer
        mov esp, [ebx+4]                ; read the new stack pointer

.L2:    popfd                           ; restore flags
        pop edi                         ; restore register edi
        pop esi                         ; restore register esi
        pop ebp                         ; restore register ebp
        pop ebx                         ; restore register ebx
        ret
