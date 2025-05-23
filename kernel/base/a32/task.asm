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
;;      Assembly functions for task management
;;

        bits 32

section .text

        extern _task_exit
        global _task_create_asm
        global _task_current
        global _task_jump_asm
        global _task_switch_asm
        global _task_switch_disable
        global _task_switch_enable
        global _task_read_next
        global _task_write_next
        global _task_patch_fxsave
        global _task_patch_fxrstor

align 16
        ; void task_create_asm(struct task *new_task,
        ;                      int (*func)(void *), void *arg)
        ;
        ; struct task {
        ;         uint64_t sp;     /* Offset: 0 */
        ;         uint64_t cr3;    /* Offset: 8 */
        ;         int active;      /* Offset: 16 + 0 * sizeof(int) */
        ;         int asm_data1;   /* Offset: 16 + 1 * sizeof(int) */
        ;         int asm_data2;   /* Offset: 16 + 2 * sizeof(int) */
        ;         int asm_data3;   /* Offset: 16 + 3 * sizeof(int) */
        ;         addr_t next;     /* Offset: 16 + 4 * sizeof(int) */
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

        lea eax, [eax-24]               ; space for pushed registers
        mov [ecx], eax                  ; new_task->sp = stack pointer
        ret

align 16
_func_start:
        mov eax, _func_return           ; eax = _func_return
        xchg eax, [esp]                 ; set the return address
        sti                             ; enable interrupts
        jmp eax                         ; func(arg)

align 16
_func_return:
        mov ecx, eax                    ; ecx = retval
        mov [esp], ecx                  ; "push" retval
        call _task_exit                 ; task_exit(retval)
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
        ; void task_jump_asm(addr_t ip, addr_t cs, addr_t sp, addr_t ss)
_task_jump_asm:
        mov ecx, [esp+4]                ; ecx = ip (eip)
        mov edx, [esp+8]                ; rdx = cs
        mov esi, [esp+12]               ; esi = sp (esp)
        mov edi, [esp+16]               ; edi = ss

        push edi                        ; push ss
        push esi                        ; push esp
        push 0x00000202                 ; push eflags
        push edx                        ; push cs
        push ecx                        ; push eip

        xor eax, eax                    ; eax = 0
        mov es, edi                     ; set es segment register
        mov ds, edi                     ; set ds segment register
        mov fs, eax                     ; set fs segment register (0)
        mov gs, eax                     ; set gs segment register (0)

        xor ecx, ecx                    ; ecx = 0
        xor edx, edx                    ; edx = 0
        xor ebx, ebx                    ; ebx = 0
        xor ebp, ebp                    ; ebp = 0
        xor esi, esi                    ; esi = 0
        xor edi, edi                    ; edi = 0
        iret                            ; "jump" to user space

align 16
        ; void task_switch_asm(struct task *next, void *tss)
_task_switch_asm:
        push ebx                        ; save register ebx
        push ebp                        ; save register ebp
        push esi                        ; save register esi
        push edi                        ; save register edi
        push fs                         ; save segment register fs
        push gs                         ; save segment register gs

        mov eax, esp                    ; eax = stack pointer
        mov ecx, [esp+28]               ; ecx = (struct task *)next
        mov edx, [esp+32]               ; edx = (void *)tss
        test eax, 0x1800                ; test stack pointer
        jz short _stack_error

        and eax, 0xFFFFE000             ; eax = address of current task
        cmp dword [eax+20], 2           ; check if task switching is enabled
        jb short _task_switch_asm_esp0

        or dword [eax+20], 1            ; set bit 0 if switching is skipped
        mov dword [ecx+16], 0           ; clear active flag (next task)
        jmp short _task_switch_asm_end

_task_switch_asm_esp0:
        lea ebx, [ecx+0x1FF0]           ; ebx = value of esp0
        mov [edx+4], ebx                ; update esp0 (task-state segment)

        lea ebx, [eax+0x0400]           ; ebx = address of fxsave area
        mov [eax], esp                  ; save stack pointer

        ; uint8_t task_patch_fxsave[3]
_task_patch_fxsave:
        db 0x0F, 0xAE, 0x03             ; "fxsave [ebx]"

        mov esp, [ecx]                  ; esp = next->sp
        mov edx, [ecx+8]                ; edx = next->cr3
        lea ebx, [ecx+0x0400]           ; ebx = address of fxrstor area

        ; uint8_t task_patch_fxrstor[3]
_task_patch_fxrstor:
        db 0x0F, 0xAE, 0x0B             ; "fxrstor [ebx]"

        mov cr3, edx                    ; change virtual address space
        mov dword [eax+16], 0           ; clear active flag (previous task)

_task_switch_asm_end:
        pop gs                          ; restore segment register gs
        pop fs                          ; restore segment register fs
        pop edi                         ; restore register edi
        pop esi                         ; restore register esi
        pop ebp                         ; restore register ebp
        pop ebx                         ; restore register ebx
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
        add dword [eax+20], 2           ; increment asm_data1 (ignore bit 0)
        js short _state_error
        ret

align 16
        ; void task_switch_enable(void)
_task_switch_enable:
        mov eax, esp                    ; eax = stack pointer
        and eax, 0xFFFFE000             ; eax = address of current task
        sub dword [eax+20], 2           ; decrement asm_data1 (ignore bit 0)
        js short _state_error
        and dword [eax+20], 0xFFFFFFFE  ; clear bit 0
        ret

_state_error:
        mov dword [eax+20], 0x88000000  ; set erroneous state value
        int3                            ; breakpoint exception
.L1:    hlt                             ; halt instruction
        jmp short .L1

align 16
        ; struct task *task_read_next(const struct task *task)
_task_read_next:
        mov ecx, [esp+4]                ; ecx = task
        mov eax, [ecx+32]               ; eax = (struct task *)task->next
        ret

align 16
        ; struct task *task_write_next(struct task *task, struct task *next)
_task_write_next:
        mov ecx, [esp+4]                ; ecx = task
        mov eax, [esp+8]                ; eax = next
        mov [ecx+32], eax               ; task->next = (addr_t)next
        ret
