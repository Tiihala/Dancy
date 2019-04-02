;;
;; Copyright (c) 2019 Antti Tiihala
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
;; dark/a32/dark.asm
;;      The dark corner of Dancy (32-bit)
;;

        bits 32

section .text

        global __aulldiv
        global __aullrem
        global ___chkstk_ms
        global __chkstk
        global ___udivdi3
        global ___umoddi3

align 16
__aulldiv:
        push ecx                        ; save register ecx
        push ebx                        ; save register ebx
        xor eax, eax                    ; eax = 0 (remainder, low dword)
        xor edx, edx                    ; edx = 0 (remainder, high dword)
        push eax                        ; push "quotient" (high dword)
        push edx                        ; push "quotient" (low dword)

        mov ecx, 63                     ; ecx = loop counter
        mov ebx, [esp+28]               ; test variable "b" (low dword)
        or ebx, [esp+32]                ; test variable "b" (high dword)
        jnz short .L1
        div ebx                         ; trigger "div by zero" exception

.L1:    shl eax, 1                      ; remainder <<= 1
        rcl edx, 1                      ; (high dword)
        mov ebx, 1                      ; ebx = 1
        rol ebx, cl                     ; rotate
        cmp ecx, 32                     ; low or high dword
        jb short .L2
        and ebx, [esp+24]               ; variable "a" (high dword)
        jmp short .L3
.L2:    and ebx, [esp+20]               ; variable "a" (low dword)
.L3:    jz short .L4
        or eax, 1                       ; set least significant bit

.L4:    push eax                        ; save register eax
        push edx                        ; save register edx
        sub eax, [esp+36]               ; variable "b" (low dword)
        sbb edx, [esp+40]               ; variable "b" (high dword)
        jnc short .L5
        pop edx                         ; restore register edx
        pop eax                         ; restore register eax
        jmp short .L7

.L5:    add esp, 8                      ; remove eax and edx from stack
        mov ebx, 1                      ; ebx = 1
        rol ebx, cl                     ; rotate
        cmp ecx, 32                     ; low or high dword
        jb short .L6
        or [esp+4], ebx                 ; set bit (quotient, high dword)
        jmp short .L7
.L6:    or [esp], ebx                   ; set bit (quotient, low dword)
.L7:    sub ecx, 1                      ; decrement counter
        jnc short .L1

        mov eax, [esp]                  ; eax = quotient (low dword)
        mov edx, [esp+4]                ; eax = quotient (high dword)
        add esp, 8                      ; remove stack variables
        pop ebx                         ; restore register ebx
        pop ecx                         ; restore register ecx
        ret 16

align 16
__aullrem:
        push ecx                        ; save register ecx
        push ebx                        ; save register ebx
        xor eax, eax                    ; eax = 0 (remainder, low dword)
        xor edx, edx                    ; edx = 0 (remainder, high dword)
        push eax                        ; push "quotient" (high dword)
        push edx                        ; push "quotient" (low dword)

        mov ecx, 63                     ; ecx = loop counter
        mov ebx, [esp+28]               ; test variable "b" (low dword)
        or ebx, [esp+32]                ; test variable "b" (high dword)
        jnz short .L1
        div ebx                         ; trigger "div by zero" exception

.L1:    shl eax, 1                      ; remainder <<= 1
        rcl edx, 1                      ; (high dword)
        mov ebx, 1                      ; ebx = 1
        rol ebx, cl                     ; rotate
        cmp ecx, 32                     ; low or high dword
        jb short .L2
        and ebx, [esp+24]               ; variable "a" (high dword)
        jmp short .L3
.L2:    and ebx, [esp+20]               ; variable "a" (low dword)
.L3:    jz short .L4
        or eax, 1                       ; set least significant bit

.L4:    push eax                        ; save register eax
        push edx                        ; save register edx
        sub eax, [esp+36]               ; variable "b" (low dword)
        sbb edx, [esp+40]               ; variable "b" (high dword)
        jnc short .L5
        pop edx                         ; restore register edx
        pop eax                         ; restore register eax
        jmp short .L7

.L5:    add esp, 8                      ; remove eax and edx from stack
        mov ebx, 1                      ; ebx = 1
        rol ebx, cl                     ; rotate
        cmp ecx, 32                     ; low or high dword
        jb short .L6
        or [esp+4], ebx                 ; set bit (quotient, high dword)
        jmp short .L7
.L6:    or [esp], ebx                   ; set bit (quotient, low dword)
.L7:    sub ecx, 1                      ; decrement counter
        jnc short .L1

        add esp, 8                      ; remove stack variables
        pop ebx                         ; restore register ebx
        pop ecx                         ; restore register ecx
        ret 16

align 16
___chkstk_ms:
        nop                             ; do nothing
        ret

align 16
__chkstk:
        sub esp, eax                    ; modify esp (scary)
        push ecx                        ; save register ecx
        mov ecx, [esp+eax+4]            ; ecx = instruction pointer
        mov [esp+4], ecx                ; set return address
        pop ecx                         ; restore register ecx
        ret

align 16
___udivdi3:
        push dword [esp+16]             ; push "b" (high dword)
        push dword [esp+16]             ; push "b" (low dword)
        push dword [esp+16]             ; push "a" (high dword)
        push dword [esp+16]             ; push "a" (low dword)
        call __aulldiv                  ; (stack cleaned by callee)
        ret

align 16
___umoddi3:
        push dword [esp+16]             ; push "b" (high dword)
        push dword [esp+16]             ; push "b" (low dword)
        push dword [esp+16]             ; push "a" (high dword)
        push dword [esp+16]             ; push "a" (low dword)
        call __aullrem                  ; (stack cleaned by callee)
        ret
