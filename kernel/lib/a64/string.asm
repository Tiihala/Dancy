;;
;; Copyright (c) 2018 Antti Tiihala
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
;; lib/a64/string.asm
;;      C standard library functions
;;

        bits 64

section .text

        global memcmp
        global memcpy
        global memmove
        global memset
        global strlen

align 16
        ; int memcmp(const void *s1, const void *s2, size_t n)
memcmp:
        push rsi                        ; save register rsi
        push rdi                        ; save register rdi
        mov rdi, rcx                    ; rdi = s1
        mov rsi, rdx                    ; rsi = s2
        mov rcx, r8                     ; rcx = n
        xor eax, eax                    ; rax = 0 (return equal)
        test rcx, rcx                   ; test zero
        jz short .L1
        cld                             ; (extra safety)
        rep cmpsb                       ; compare bytes
        je short .L1
        mov eax, 0xFFFFFFFF             ; "below zero"
        ja short .L1
        mov eax, 0x00000001             ; "above zero"
.L1:    pop rdi                         ; restore register rdi
        pop rsi                         ; restore register rsi
        ret

align 16
        ; void *memcpy(void *s1, const void *s2, size_t n)
memcpy:
        push rsi                        ; save register rsi
        push rdi                        ; save register rdi
        mov rdi, rcx                    ; rdi = s1
        mov rsi, rdx                    ; rsi = s2
        mov rdx, rcx                    ; rdx = s1 (return value)
        mov rcx, r8                     ; rcx = n
        cld                             ; (extra safety)
        rep movsb                       ; copy bytes
        mov rax, rdx                    ; rax = s1
        pop rdi                         ; restore register rdi
        pop rsi                         ; restore register rsi
        ret

align 16
        ; void *memmove(void *s1, const void *s2, size_t n)
memmove:
        push rsi                        ; save register rsi
        push rdi                        ; save register rdi
        cld                             ; (extra safety)
        mov rdi, rcx                    ; rdi = s1
        mov rsi, rdx                    ; rsi = s2
        mov rdx, rcx                    ; rdx = s1 (return value)
        mov rcx, r8                     ; rcx = n
        cmp rdi, rsi                    ; compare s1 and s2
        jbe short .L1
        lea rdi, [rdi+rcx-1]            ; rdi = s1 + n - 1
        lea rsi, [rsi+rcx-1]            ; rsi = s2 + n - 1
        std                             ; set direction flag
.L1:    rep movsb                       ; copy bytes
        mov rax, rdx                    ; rax = s1
        cld                             ; clear direction flag
        pop rdi                         ; restore register rdi
        pop rsi                         ; restore register rsi
        ret

align 16
        ; void *memset(void *s, int c, size_t n)
memset:
        push rdi                        ; save register rdi
        mov rdi, rcx                    ; rdi = s
        mov eax, edx                    ; eax = c (low byte)
        mov rdx, rcx                    ; rdx = s (return value)
        mov rcx, r8                     ; rcx = n
        cld                             ; (extra safety)
        rep stosb                       ; copy the low byte
        mov rax, rdx                    ; rax = s
        pop rdi                         ; restore register rdi
        ret

align 16
        ; size_t strlen(const char *s)
strlen:
        push rdi                        ; save register rdi
        mov rdi, rcx                    ; rdi = s
        xor eax, eax                    ; rax = 0
        lea rcx, [rax-1]                ; rcx = SIZE_MAX
        cld                             ; (extra safety)
        repne scasb                     ; find the zero byte
        not rcx                         ; reverse each bit of rcx
        lea rax, [rcx-1]                ; rax = length of the string
        pop rdi                         ; restore register rdi
        ret
