;;
;; Copyright (c) 2022 Antti Tiihala
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
;; libc/a32/memmove.asm
;;      The C Standard Library
;;

        bits 32

section .text

        global _memmove

align 16
        ; void *memmove(void *s1, const void *s2, size_t n)
_memmove:
        push esi                        ; save register esi
        push edi                        ; save register edi
        cld                             ; (extra safety)
        mov edi, [esp+12]               ; edi = s1
        mov esi, [esp+16]               ; esi = s2
        mov ecx, [esp+20]               ; ecx = n
        cmp edi, esi                    ; compare s1 and s2
        jbe short .L1
        lea edi, [edi+ecx-1]            ; edi = s1 + n - 1
        lea esi, [esi+ecx-1]            ; esi = s2 + n - 1
        std                             ; set direction flag
.L1:    rep movsb                       ; copy bytes
        mov eax, [esp+12]               ; eax = s1
        cld                             ; clear direction flag
        pop edi                         ; restore register edi
        pop esi                         ; restore register esi
        ret
