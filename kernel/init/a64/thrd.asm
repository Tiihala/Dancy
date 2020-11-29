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
;; init/a64/thrd.asm
;;      Threads for pre-kernel environment
;;

        bits 64

section .text

        extern init_thrd_current
        extern init_thrd_size
        global init_thrd_create
        global thrd_yield

align 16
        ; void init_thrd_create(void *thr, thrd_start_t func, void *arg)
init_thrd_create:
        push rbx                        ; save register rbx
        mov ebx, init_thrd_size         ; r/ebx = address of init_thrd_size
        mov rbx, [rbx]                  ; rbx = stack size (at least align 32)
        add rbx, rcx                    ; rbx = stack pointer

        mov eax, init_thrd_return       ; r/eax = address of init_thrd_return
        lea rbx, [rbx-40]               ; rbx = stack pointer
        mov [rbx], rax                  ; set the return address

        lea rbx, [rbx-8]                ; rbx = stack pointer
        mov [rbx], rdx                  ; save value (func)

        lea rbx, [rbx-8]                ; rbx = stack pointer
        mov [rbx], r8                   ; save value (arg)

        mov eax, init_thrd_start        ; r/eax = address of init_thrd_start
        lea rbx, [rbx-8]                ; rbx = stack pointer
        mov [rbx], rax                  ; save value

        mov eax, 0x00000202             ; r/eax = flags
        lea rbx, [rbx-72]               ; rbx = stack pointer
        mov [rbx], rax                  ; save value

        mov [rcx+8], rbx                ; set stack (struct init_thrd)
        pop rbx                         ; restore register rbx
        ret

align 16
init_thrd_start:
        pop rcx                         ; rcx = arg
        pop rax                         ; rax = func
        jmp rax                         ; func(arg)

align 16
init_thrd_return:
        mov ebx, init_thrd_current      ; r/ebx = address of init_thrd_current
        mov rbx, [rbx]                  ; rbx = init_thrd_current
        mov [rbx+20], eax               ; write "res"
        mov dword [rbx+24], 1           ; write "terminated"
.L1:    call thrd_yield                 ; call next thread
        jmp short .L1

align 16
        ; void thrd_yield(void)
thrd_yield:
        push rbx                        ; save register rbx
        push rbp                        ; save register rbp
        push rsi                        ; save register rsi
        push rdi                        ; save register rdi
        push r12                        ; save register r12
        push r13                        ; save register r13
        push r14                        ; save register r14
        push r15                        ; save register r15
        pushfq                          ; save flags
        cli                             ; disable interrupts

        mov edi, init_thrd_current      ; r/edi = address of init_thrd_current
        mov rbx, [rdi]                  ; rbx = address of init_thrd_current
        test rbx, rbx                   ; test zero
        jz short .L2

        mov [rbx+8], rsp                ; save current stack pointer
.L1:    mov rbx, [rbx]                  ; rbx = next
        cmp dword [rbx+24], 0           ; check "terminated"
        jne short .L1

        mov [rdi], rbx                  ; change the current pointer
        mov rsp, [rbx+8]                ; read the new stack pointer

.L2:    popfq                           ; restore flags
        pop r15                         ; restore register r15
        pop r14                         ; restore register r14
        pop r13                         ; restore register r13
        pop r12                         ; restore register r12
        pop rdi                         ; restore register rdi
        pop rsi                         ; restore register rsi
        pop rbp                         ; restore register rbp
        pop rbx                         ; restore register rbx
        ret
