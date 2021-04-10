;;
;; Copyright (c) 2021 Antti Tiihala
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
;; base/a64/task.asm
;;      Context switch
;;

        bits 64

section .text

        extern task_yield
        global task_create_asm
        global task_current
        global task_switch_asm

align 16
        ; void task_create_asm(struct task *new_task,
        ;                      int (*func)(void *), void *arg)
        ;
        ; #define TASK_FLAG_RUNNING         (0x00000001)
        ; #define TASK_FLAG_TERMINATED      (0x00000002)
        ;
        ; struct task {
        ;         uint32_t esp;    /* Offset: 0 */
        ;         uint32_t cr3;    /* Offset: 4 */
        ;         uint64_t id;     /* Offset: 8 */
        ;         int state;       /* Offset: 16 + 0 * sizeof(int) */
        ;         int retval;      /* Offset: 16 + 1 * sizeof(int) */
        ;         ...
        ; };
task_create_asm:
        lea eax, [rcx+0x1FD0]           ; rax = stack pointer

        mov [rax+8], rdx                ; save func
        mov [rax+16], r8                ; save arg
        mov edx, func_start             ; rdx = func_start
        mov [rax], rdx                  ; save func_start

        lea eax, [rax-64]               ; space for pushed registers
        mov [rcx], eax                  ; new_task->esp = stack pointer
        ret

align 16
func_start:
        mov eax, func_return            ; rax = func_return
        xchg rax, [rsp]                 ; set the return address
        mov rcx, [rsp+8]                ; rcx = arg
        jmp rax                         ; func(arg)

align 16
func_return:
        mov ecx, esp                    ; rcx = stack pointer (32-bit)
        and ecx, 0xFFFFE000             ; rcx = address of current task
        mov [rcx+20], eax               ; current->retval = eax
        or dword [rcx+16], 0x00000002   ; set TASK_FLAG_TERMINATED
        call task_yield                 ; switch to another task
        int3                            ; breakpoint exception
.L1:    hlt                             ; halt instruction
        jmp short .L1

align 16
        ; struct task *task_current(void)
task_current:
        mov eax, esp                    ; rax = stack pointer (32-bit)
        and eax, 0xFFFFE000             ; rax = address of current task
        ret

align 16
        ; void task_switch_asm(struct task *next)
task_switch_asm:
        push rbx                        ; save register rbx
        push rbp                        ; save register rbp
        push rsi                        ; save register rsi
        push rdi                        ; save register rdi
        push r12                        ; save register r12
        push r13                        ; save register r13
        push r14                        ; save register r14
        push r15                        ; save register r15

        mov eax, esp                    ; rax = stack pointer (32-bit)
        test eax, 0x1000                ; test stack pointer
        jz short stack_error

        and eax, 0xFFFFE000             ; rax = address of current task
        mov [rax], esp                  ; save stack pointer
        fxsave [rax+0x0C00]             ; save fpu, mmx, and sse state

        mov esp, [rcx]                  ; rsp = next->esp (32-bit)
        mov edx, [rcx+4]                ; rdx = next->cr3 (32-bit)
        fxrstor [rcx+0x0C00]            ; restore fpu, mmx, and sse state

        mov cr3, rdx                    ; change virtual address space
        and dword [rax+16], 0xFFFFFFFE  ; clear TASK_FLAG_RUNNING (previous)
        pop r15                         ; restore register r15
        pop r14                         ; restore register r14
        pop r13                         ; restore register r13
        pop r12                         ; restore register r12
        pop rdi                         ; restore register rdi
        pop rsi                         ; restore register rsi
        pop rbp                         ; restore register rbp
        pop rbx                         ; restore register rbx
        ret

stack_error:
        int3                            ; breakpoint exception
.L1:    hlt                             ; halt instruction
        jmp short .L1
