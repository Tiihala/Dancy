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
;; libc/a32/memcmp.asm
;;      The C Standard Library
;;

        bits 32

section .text

        global _memcmp

align 16
        ; int memcmp(const void *s1, const void *s2, size_t n)
_memcmp:
        push esi                        ; save register esi
        push edi                        ; save register edi
        mov edi, [esp+12]               ; edi = s1
        mov esi, [esp+16]               ; esi = s2
        mov ecx, [esp+20]               ; ecx = n
        xor eax, eax                    ; eax = 0 (return equal)
        test ecx, ecx                   ; test zero
        jz short .L1
        cld                             ; (extra safety)
        rep cmpsb                       ; compare bytes
        je short .L1
        mov eax, 0xFFFFFFFF             ; "below zero"
        ja short .L1
        mov eax, 0x00000001             ; "above zero"
.L1:    pop edi                         ; restore register edi
        pop esi                         ; restore register esi
        ret
