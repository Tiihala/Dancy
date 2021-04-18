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
        global _task_switch_disable
        global _task_switch_enable
        global _task_patch_fxsave
        global _task_patch_fxrstor

align 16
        ; void task_create_asm(struct task *new_task,
        ;                      int (*func)(void *), void *arg)
        ;
        ; struct task {
        ;         uint32_t esp;    /* Offset: 0 */
        ;         uint32_t cr3;    /* Offset: 4 */
        ;         uint64_t id;     /* Offset: 8 */
        ;         int active;      /* Offset: 16 + 0 * sizeof(int) */
        ;         int retval;      /* Offset: 16 + 1 * sizeof(int) */
        ;         int stopped;     /* Offset: 16 + 2 * sizeof(int) */
        ;         int ndisable;    /* Offset: 16 + 3 * sizeof(int) */
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
        mov dword [ecx+24], 1           ; set stopped flag
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
        cmp dword [eax+28], 0           ; skip if ndisable is non-zero
        jne short _task_switch_asm_end

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
        mov dword [eax+16], 0           ; clear active flag (previous task)

_task_switch_asm_end:
        pop edi                         ; restore register edi
        pop esi                         ; restore register esi
        pop ebp                         ; restore register ebp
        pop ebx                         ; restore register ebx
        sti                             ; enable interrupts
        ret

_stack_error:
        int3                            ; breakpoint exception
.L1:    hlt                             ; halt instruction
        jmp short .L1

align 16
        ; void task_switch_disable(void)
_task_switch_disable:
        mov eax, esp                    ; eax = stack pointer
        and eax, 0xFFFFE000             ; eax = address of current task
        lock add dword [eax+28], 1      ; increment ndisable value
        js short _state_error
        ret

align 16
        ; void task_switch_enable(void)
_task_switch_enable:
        mov eax, esp                    ; eax = stack pointer
        and eax, 0xFFFFE000             ; eax = address of current task
        lock sub dword [eax+28], 1      ; decrement ndisable value
        js short _state_error
        ret

_state_error:
        mov dword [eax+28], 0x88000000  ; set erroneous state value
        int3                            ; breakpoint exception
.L1:    hlt                             ; halt instruction
        jmp short .L1
