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
;; init/a32/ld.asm
;;      Object relocation (32-bit)
;;

        bits 32

section .text

        global _ld_relocate

align 16
        ; void ld_relocate(void *base, void *reloc, void *symbol)
_ld_relocate:
        push ebx                        ; save register ebx
        push esi                        ; save register esi
        push edi                        ; save register edi

        mov ebx, [esp+16]               ; ebx = base
        mov esi, [esp+20]               ; esi = reloc
        mov edi, [esp+24]               ; edi = symbol

        mov eax, [edi+8]                ; eax = symbol value
        mov edi, ebx                    ; edi = base
        add edi, [esi]                  ; edi = target for relocation

        xor ecx, ecx                    ; ecx = 0
        or cx, [esi+8]                  ; ecx = reloc type
        jz short .end

.t6:    cmp ecx, 6                      ; test type 6
        jne short .t7
        add [edi], eax                  ; relocate
        jmp short .end

.t7:    cmp ecx, 7                      ; test type 7
        jne short .t20
        sub eax, ebx                    ; subtract section base
        add [edi], eax                  ; relocate
        jmp short .end

.t20:   cmp ecx, 20                     ; test type 20
        jne short .dbg
        lea ecx, [edi+4]                ; ecx = target + 4
        sub eax, ecx                    ; eax = "relative"
        add [edi], eax                  ; relocate
        jmp short .end

.dbg:   hlt                             ; halt
        jmp short .dbg

.end:   pop edi                         ; restore register edi
        pop esi                         ; restore register esi
        pop ebx                         ; restore register ebx
        ret
