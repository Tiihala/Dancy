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
;; base/a32/task.asm
;;      Context switch
;;

        bits 32

section .text

        extern _task_yield
        global _task_create_asm
        global _task_current
        global _task_switch_asm
        global _task_patch_fxsave
        global _task_patch_fxrstor

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
_task_create_asm:
        mov ecx, [esp+4]                ; ecx = new_task
        mov edx, [esp+8]                ; edx = func
        lea eax, [ecx+0x1FE8]           ; eax = stack pointer

        mov [eax+4], edx                ; save func
        mov edx, [esp+12]               ; edx = arg
        mov [eax+8], edx                ; save arg
        mov edx, _func_start            ; edx = _func_start
        mov [eax], edx                  ; save _func_start

        lea eax, [eax-16]               ; space for pushed registers
        mov [ecx], eax                  ; new_task->esp = stack pointer
        ret

align 16
_func_start:
        mov eax, _func_return           ; eax = _func_return
        xchg eax, [esp]                 ; set the return address
        jmp eax                         ; func(arg)

align 16
_func_return:
        mov ecx, esp                    ; ecx = stack pointer
        and ecx, 0xFFFFE000             ; ecx = address of current task
        mov [ecx+20], eax               ; current->retval = eax
        or dword [ecx+16], 0x00000002   ; set TASK_FLAG_TERMINATED
        call _task_yield                ; switch to another task
        int3                            ; breakpoint exception
.L1:    hlt                             ; halt instruction
        jmp short .L1

align 16
        ; struct task *task_current(void)
_task_current:
        mov eax, esp                    ; eax = stack pointer
        and eax, 0xFFFFE000             ; eax = address of current task
        ret

align 16
        ; void task_switch_asm(struct task *next)
_task_switch_asm:
        push ebx                        ; save register ebx
        push ebp                        ; save register ebp
        push esi                        ; save register esi
        push edi                        ; save register edi

        mov eax, esp                    ; eax = stack pointer
        mov ecx, [esp+20]               ; ecx = (struct task *)next
        test eax, 0x1000                ; test stack pointer
        jz short _stack_error

        and eax, 0xFFFFE000             ; eax = address of current task
        lea ebx, [eax+0x0C00]           ; ebx = address of fxsave area
        mov [eax], esp                  ; save stack pointer

        ; uint8_t task_patch_fxsave[3]
_task_patch_fxsave:
        db 0x0F, 0xAE, 0x03             ; "fxsave [ebx]"

        mov esp, [ecx]                  ; esp = next->esp
        mov edx, [ecx+4]                ; edx = next->cr3
        lea ebx, [ecx+0x0C00]           ; ebx = address of fxrstor area

        ; uint8_t task_patch_fxrstor[3]
_task_patch_fxrstor:
        db 0x0F, 0xAE, 0x0B             ; "fxrstor [ebx]"

        mov cr3, edx                    ; change virtual address space
        and dword [eax+16], 0xFFFFFFFE  ; clear TASK_FLAG_RUNNING (previous)
        pop edi                         ; restore register edi
        pop esi                         ; restore register esi
        pop ebp                         ; restore register ebp
        pop ebx                         ; restore register ebx
        ret

_stack_error:
        int3                            ; breakpoint exception
.L1:    hlt                             ; halt instruction
        jmp short .L1
