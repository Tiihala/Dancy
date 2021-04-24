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

        extern task_exit
        global task_create_asm
        global task_current
        global task_jump_asm
        global task_switch_asm
        global task_switch_disable
        global task_switch_enable

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
task_create_asm:
        lea eax, [rcx+0x1FD0]           ; rax = stack pointer

        mov [rax+8], rdx                ; save func
        mov [rax+16], r8                ; save arg
        mov edx, func_start             ; rdx = func_start
        mov [rax], rdx                  ; save func_start

        lea eax, [rax-96]               ; space for pushed registers
        mov [rcx], eax                  ; new_task->esp = stack pointer
        ret

align 16
func_start:
        mov eax, func_return            ; rax = func_return
        xchg rax, [rsp]                 ; set the return address
        mov rcx, [rsp+8]                ; rcx = arg
        sti                             ; enable interrupts
        jmp rax                         ; func(arg)

align 16
func_return:
        mov ecx, eax                    ; rcx = retval
        call task_exit                  ; task_exit(retval)
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
        ; void task_jump_asm(addr_t ip, addr_t cs, addr_t sp, addr_t ss)
task_jump_asm:
        push r9                         ; push ss
        push r8                         ; push rsp
        push 0x00000202                 ; push rflags
        push rdx                        ; push cs
        push rcx                        ; push rip

        xor eax, eax                    ; rax = 0
        mov es, eax                     ; set es segment register (0)
        mov ds, eax                     ; set ds segment register (0)
        mov fs, eax                     ; set fs segment register (0)
        mov gs, eax                     ; set gs segment register (0)

        xor ecx, ecx                    ; rcx = 0
        xor edx, edx                    ; rdx = 0
        xor ebx, ebx                    ; rbx = 0
        xor ebp, ebp                    ; rbp = 0
        xor esi, esi                    ; rsi = 0
        xor edi, edi                    ; rdi = 0
        xor r8, r8                      ; r8 = 0
        xor r9, r9                      ; r9 = 0
        xor r10, r10                    ; r10 = 0
        xor r11, r11                    ; r11 = 0
        xor r12, r12                    ; r12 = 0
        xor r13, r13                    ; r13 = 0
        xor r14, r14                    ; r14 = 0
        xor r15, r15                    ; r15 = 0
        iretq                           ; "jump" to user space

align 16
        ; void task_switch_asm(struct task *next, void *tss)
task_switch_asm:
        push rbx                        ; save register rbx
        push rbp                        ; save register rbp
        push rsi                        ; save register rsi
        push rdi                        ; save register rdi
        push r12                        ; save register r12
        push r13                        ; save register r13
        push r14                        ; save register r14
        push r15                        ; save register r15

        mov eax, es                     ; save segment register es
        push rax                        ; ^^^^
        mov eax, ds                     ; save segment register ds
        push rax                        ; ^^^^
        push fs                         ; save segment register fs
        push gs                         ; save segment register gs

        mov eax, esp                    ; rax = stack pointer (32-bit)
        test eax, 0x1000                ; test stack pointer
        jz short stack_error

        and eax, 0xFFFFE000             ; rax = address of current task
        cmp dword [rax+28], 0           ; skip if ndisable is non-zero
        jne short task_switch_asm_end

        lea ebx, [rcx+0x1FF0]           ; ebx = value of rsp0 (32-bit)
        mov [rdx+4], ebx                ; update rsp0 (task-state segment)

        mov [rax], esp                  ; save stack pointer
        fxsave [rax+0x0C00]             ; save fpu, mmx, and sse state

        mov esp, [rcx]                  ; rsp = next->esp (32-bit)
        mov edx, [rcx+4]                ; rdx = next->cr3 (32-bit)
        fxrstor [rcx+0x0C00]            ; restore fpu, mmx, and sse state

        mov cr3, rdx                    ; change virtual address space
        mov dword [rax+16], 0           ; clear active flag (previous task)

task_switch_asm_end:
        pop gs                          ; restore segment register gs
        pop fs                          ; restore segment register fs
        pop rax                         ; restore segment register ds
        mov ds, eax                     ; ^^^^^^^
        pop rax                         ; restore segment register es
        mov es, eax                     ; ^^^^^^^

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

align 16
        ; void task_switch_disable(void)
task_switch_disable:
        mov eax, esp                    ; rax = stack pointer (32-bit)
        and eax, 0xFFFFE000             ; rax = address of current task
        lock add dword [rax+28], 1      ; increment ndisable value
        js short state_error
        ret

align 16
        ; void task_switch_enable(void)
task_switch_enable:
        mov eax, esp                    ; rax = stack pointer (32-bit)
        and eax, 0xFFFFE000             ; rax = address of current task
        lock sub dword [rax+28], 1      ; decrement ndisable value
        js short state_error
        ret

state_error:
        mov dword [rax+28], 0x88000000  ; set erroneous state value
        int3                            ; breakpoint exception
.L1:    hlt                             ; halt instruction
        jmp short .L1
