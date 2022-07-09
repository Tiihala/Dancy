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
;; lib/a64/coff.asm
;;      Object relocation (64-bit)
;;

        bits 64

section .text

        global coff_relocate

align 16
        ; void coff_relocate(void *base, void *reloc, void *symbol)
coff_relocate:
        push rbx                        ; save register rbx
        push rsi                        ; save register rsi
        push rdi                        ; save register rdi

        mov rbx, rcx                    ; rbx = base
        mov rsi, rdx                    ; rsi = reloc
        mov rdi, r8                     ; rdi = symbol

        mov eax, [rdi+8]                ; eax = symbol value
        mov edi, ebx                    ; edi = base
        add edi, [rsi]                  ; edi = target for relocation

        xor ecx, ecx                    ; ecx = 0
        or cx, [rsi+8]                  ; ecx = reloc type
        jz short .end

.t1:    cmp ecx, 2                      ; test types 1 and 2
        je short .t2
        ja short .t3
        db 0x48                         ; type 1 is "add [rdi], rax"
.t2:    add [byte rdi], eax             ; relocate
        jmp short .end

.t3:    cmp ecx, 3                      ; test type 3
        jne short .t4
        sub eax, ebx                    ; subtract section base
        add [rdi], eax                  ; relocate
        jmp short .end

.t4:    cmp ecx, 9                      ; test types 4-9
        ja short .dbg
        lea edx, [rdi+rcx]              ; edx = target + distance
        sub eax, edx                    ; eax = "relative"
        add [rdi], eax                  ; relocate
        jmp short .end

.dbg:   hlt                             ; halt
        jmp short .dbg

.end:   pop rdi                         ; restore register rdi
        pop rsi                         ; restore register rsi
        pop rbx                         ; restore register rbx
        ret
