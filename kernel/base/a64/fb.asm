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
;; base/a64/fb.asm
;;      Standard framebuffer
;;

        bits 64

section .text

        global fb_copy
        global fb_copy_vga

align 16
        ; void fb_copy(const void *fb_page, const void *fb_blit_buffer)
        ;
        ; struct fb_page {
        ;         void *pte;
        ;         void *dst_a;
        ;         void *dst_b;
        ;         void *dst_c;
        ;         size_t size_a;
        ;         size_t size_b;
        ;         size_t size_c;
        ; };
fb_copy:
        push rbx                        ; save register rbx
        push rsi                        ; save register rsi
        push rdi                        ; save register rdi
        mov rbx, rcx                    ; rbx = fb_page
        mov rsi, rdx                    ; rsi = fb_blit_buffer
        mov edx, 3                      ; edx = repeat counter
        cld                             ; (extra safety)

.L1:    mov rdi, [rbx+8]                ; rdi = destination
        mov ecx, [rbx+32]               ; rcx = size (32-bit)
        test ecx, ecx                   ; test zero
        jz short .L4

.L2:    test edi, 7                     ; test destination alignment
        jz short .L3
        movsb                           ; copy one byte
        dec ecx                         ; decrement size
        jnz short .L2

.L3:    mov eax, ecx                    ; eax = size
        and eax, 7                      ; eax = low bits of size
        shr ecx, 3                      ; ecx = aligned size
        rep movsq                       ; copy qwords
        mov ecx, eax                    ; ecx = unaligned size
        rep movsb                       ; copy bytes

.L4:    lea rbx, [rbx+8]                ; next destination
        dec edx                         ; decrement repeat counter
        jnz short .L1

        pop rdi                         ; restore register rdi
        pop rsi                         ; restore register rsi
        pop rbx                         ; restore register rbx
        ret

align 16
        ; void fb_copy_vga(void *s1, const void *s2)
fb_copy_vga:
        push rsi                        ; save register rsi
        push rdi                        ; save register rdi
        mov rdi, rcx                    ; rdi = s1
        mov rsi, rdx                    ; rsi = s2
        mov ecx, 32                     ; rcx = 32
        cld                             ; (extra safety)
        rep movsd                       ; copy 128 bytes (use dwords)
        pop rdi                         ; restore register rdi
        pop rsi                         ; restore register rsi
        ret
