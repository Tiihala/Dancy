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
;; dy-init/ia32.asm
;;      Code for relocating init executable code
;;
;; Build instructions
;;      nasm -f bin -o ia32.bin ia32.asm
;;

%define OBJECT_TYPE     0x014C
%define START_1         '_sta'
%define START_2         'rt'

        org 0
        bits 32

times 32 db 0
in_ia32:
        mov ebx, esp                    ; ebx = stack pointer
        and ebx, 0xFFFF0000             ; ebx = base address
        lea esi, [ebx+object_file]      ; esi = object file
        mov ecx, [esi]                  ; first dword of object file
        mov ebp, [esi+16]               ; check optional header and flags
        sub ecx, (OBJECT_TYPE+0x40000)  ; test magic word and sections
        or ecx, ebp                     ; both registers must be zero
        jnz short .halt1                ; simple validity test

        mov eax, [ebx+16]               ; eax = file size
        add eax, 0x0000003F             ; add 63
        and eax, 0xFFFFFFC0             ; align 64, bss offset
        mov [esi+(20+(3*40)+20)], eax   ; set bss offset

        lea edi, [esp-16]               ; edi = array of four stack variables
        mov esp, edi                    ; allocate stack space (16 bytes)
        mov ecx, 4                      ; ecx = four sections
        lea edx, [esi+20+20]            ; edx = first section
.secs:  mov eax, [edx]                  ; eax = section offset
        add eax, ebx                    ; eax = text address
        jc short .halt1                 ; simple validity test
        mov [edi], eax                  ; save
        lea edx, [edx+40]               ; next section
        lea edi, [edi+4]                ; next stack variable
        sub ecx, 1                      ; decrement counter
        jnz short .secs

        mov ecx, [esi+12]               ; ecx = number of symbols
        mov edx, 0x0000FFFF             ; edx = 0x0000FFFF (bit mask)
        mov edi, [esi+8]                ; edi = symbol table offset
        lea edi, [ebx+edi]              ; edi = symbol table address
        jmp short .syms
.halt1: hlt                             ; this will trigger an exception
        jmp short .halt1                ; should not happen

.syms:  mov eax, [edi+12]               ; ax = section number
        and eax, edx                    ; clear upper bits and test zero
        jz short .halt2                 ; externals are not allowed
        cmp eax, 0x0004                 ; test upper limit
        ja short .next                  ; ("forward compatibility")
        mov eax, [esp+4*eax-4]          ; eax = section address

        cmp dword [byte edi+0], strict dword START_1
        jne short .fix
        cmp dword [byte edi+4], strict dword START_2
        jne short .fix

        mov ebp, [edi+8]                ; ebp = symbol value
        lea ebp, [eax+ebp]              ; ebp = start address
.fix:   add [edi+8], eax                ; fix symbol value
        jc short .halt2                 ; simple validity test
.next:  lea edi, [edi+18]               ; edi = next symbol
        sub ecx, 1                      ; decrement symbols
        jnz short .syms
        lea esi, [object_file+20]       ; esi = text section entry offset
        jmp short relocate
.halt2: hlt                             ; this will trigger an exception
        jmp short .halt2                ; should not happen

relocate:
        mov ecx, [ebx+esi+32]           ; cx = relocation entries
        and ecx, 0xFFFF                 ; clear upper bits and test zero
        jz short .next
        mov eax, [ebx+esi+20]           ; eax = section data offset
        add eax, ebx                    ; eax = section data address
        mov edx, [ebx+esi+24]           ; edx = relocation entries offset
        add edx, ebx                    ; edx = relocation entries address
        mov [byte esp+0], ecx           ; "counter"
        mov [byte esp+4], eax           ; "section data"
        mov [byte esp+8], edx           ; "relocation entry"
        mov eax, [ebx+object_file+8]    ; eax = symbol table offset
        add eax, ebx                    ; eax = symbol table address
        mov [esp+12], eax               ; "symbol table"
.fix:   call relocation_entry           ; relocate
        add dword [esp+8], 10           ; next entry
        sub dword [esp], 1              ; decrement counter
        jnz short .fix

.next:  lea esi, [esi+40]               ; next section entry
        cmp esi, (object_file+140)      ; test end of relocatable sections
        jb short relocate
        je short jump_to_start          ; ("extra safety", must be equal)
.halt:  hlt                             ; this will trigger an exception
        jmp short .halt                 ; should not happen

jump_to_start:
        add esp, 16                     ; free stack space
        test ebp, ebp                   ; test start address
        jz short .halt
        push ebp                        ; push start address
        call .func
.halt:  hlt                             ; this will trigger an exception
        jmp short .halt                 ; should not happen
.func:  xor eax, eax                    ; eax = 0
        xor ecx, ecx                    ; ecx = 0
        xor edx, edx                    ; edx = 0
        xor ebp, ebp                    ; ebp = 0
        xor esi, esi                    ; esi = 0
        xor edi, edi                    ; edi = 0
        add esp, 4                      ; adjust return address
        ret

relocation_entry:
        push ebx                        ; save register ebx
        push esi                        ; save register esi
        mov eax, 18                     ; eax = symbol size
        xor ecx, ecx                    ; ecx = 0
        lea ebx, [esp+12]               ; ebx = address of structure

        ; [ebx+0x00] = "counter"
        ; [ebx+0x04] = "section data"
        ; [ebx+0x08] = "relocation entry"
        ; [ebx+0x0C] = "symbol table"

        mov esi, [ebx+8]                ; esi = relocation entry
        or cx, [esi+8]                  ; ecx = type
        jz short .end                   ; ignore zero type
        mov edi, [esi+4]                ; edi = symbol table index
        mul edi                         ; eax = symbol table offset
        mov edi, [esi+0]                ; edi = target offset
        add edi, [ebx+4]                ; edi = target address
        mov esi, [ebx+12]               ; esi = symbol table base
        lea esi, [esi+eax]              ; esi = symbol table entry
        mov eax, [esi+8]                ; eax = symbol value

.t6:    cmp ecx, 6                      ; test type 6
        jne short .t7
        add [edi], eax                  ; relocate
        jmp short .end

.t7:    cmp ecx, 7                      ; test type 7
        jne short .t20
        lea ebx, [ebx+4]                ; ebx = ebx + 4
        sub eax, [ebx]                  ; subtract section base
        add [edi], eax                  ; relocate
        jmp short .end

.t20:   cmp ecx, 20                     ; test type 20
        jne short .dbg
        lea ecx, [edi+4]                ; ecx = target + 4
        sub eax, ecx                    ; eax = "relative"
        add [edi], eax                  ; relocate
        jmp short .end

.dbg:   xor ebp, ebp                    ; ebp = 0
        mov ebx, [ebx+8]                ; ebx = relocation entry
        mov eax, [ebx+0]                ; eax = target offset
        mov edx, [ebx+4]                ; edx = index
        xor ebx, ebx                    ; ebx = 0
.halt:  hlt                             ; this will trigger an exception
        jmp short .halt                 ; should not happen
.end:   pop esi                         ; restore register esi
        pop ebx                         ; restore register ebx
        ret

times 0x1A0 - ($ - $$) db 0
object_file:
