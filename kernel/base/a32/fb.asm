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
;; base/a32/fb.asm
;;      Standard framebuffer
;;

        bits 32

section .text

        global _fb_copy
        global _fb_copy_vga

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
_fb_copy:
        push ebx                        ; save register ebx
        push esi                        ; save register esi
        push edi                        ; save register edi
        mov ebx, [esp+16]               ; ebx = fb_page
        mov esi, [esp+20]               ; esi = fb_blit_buffer
        mov edx, 3                      ; edx = repeat counter
        cld                             ; (extra safety)

.L1:    mov edi, [ebx+4]                ; edi = destination
        mov ecx, [ebx+16]               ; ecx = size
        test ecx, ecx                   ; test zero
        jz short .L4

.L2:    test edi, 3                     ; test destination alignment
        jz short .L3
        movsb                           ; copy one byte
        dec ecx                         ; decrement size
        jnz short .L2

.L3:    mov eax, ecx                    ; eax = size
        and eax, 3                      ; eax = low bits of size
        shr ecx, 2                      ; ecx = aligned size
        rep movsd                       ; copy dwords
        mov ecx, eax                    ; ecx = unaligned size
        rep movsb                       ; copy bytes

.L4:    lea ebx, [ebx+4]                ; next destination
        dec edx                         ; decrement repeat counter
        jnz short .L1

        pop edi                         ; restore register edi
        pop esi                         ; restore register esi
        pop ebx                         ; restore register ebx
        ret

align 16
        ; void fb_copy_vga(void *s1, const void *s2)
_fb_copy_vga:
        push esi                        ; save register esi
        push edi                        ; save register edi
        mov edi, [esp+12]               ; edi = s1
        mov esi, [esp+16]               ; esi = s2
        mov ecx, 32                     ; ecx = 32
        cld                             ; (extra safety)
        rep movsd                       ; copy 128 bytes
        pop edi                         ; restore register edi
        pop esi                         ; restore register esi
        ret
