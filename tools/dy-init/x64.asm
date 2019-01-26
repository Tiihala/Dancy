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
;; dy-init/x64.asm
;;      Code for relocating init executable code
;;
;; Build instructions
;;      nasm -f bin -o x64.bin x64.asm
;;

%define OBJECT_TYPE     0x8664
%define START_1         'star'
%define START_2         't'

        org 0
        bits 64

times 32 db 0
in_x64:
        mov ebx, esp                    ; ebx = stack pointer
        and ebx, 0xFFFF0000             ; ebx = base address
        lea esi, [rbx+object_file]      ; esi = object file
        mov ecx, [rsi]                  ; first dword of object file
        mov ebp, [rsi+16]               ; check optional header and flags
        sub ecx, (OBJECT_TYPE+0x40000)  ; test magic word and sections
        or ecx, ebp                     ; both registers must be zero
        jnz short .halt1                ; simple validity test

        mov eax, [rbx+16]               ; eax = file size
        add eax, 0x0000003F             ; add 63
        and eax, 0xFFFFFFC0             ; align 64, bss offset
        mov [rsi+(20+(3*40)+20)], eax   ; set bss offset

        lea edi, [rsp-16]               ; edi = array of four stack variables
        mov esp, edi                    ; allocate stack space (16 bytes)
        mov ecx, 4                      ; ecx = four sections
        lea edx, [rsi+20+20]            ; edx = first section
.secs:  mov eax, [rdx]                  ; eax = section offset
        add eax, ebx                    ; eax = text address
        jc short .halt1                 ; simple validity test
        mov [rdi], eax                  ; save
        lea edx, [rdx+40]               ; next section
        lea edi, [rdi+4]                ; next stack variable
        sub ecx, 1                      ; decrement counter
        jnz short .secs

        mov ecx, [rsi+12]               ; ecx = number of symbols
        mov edx, 0x0000FFFF             ; edx = 0x0000FFFF (bit mask)
        mov edi, [rsi+8]                ; edi = symbol table offset
        lea edi, [rbx+rdi]              ; edi = symbol table address
        jmp short .syms
.halt1: hlt                             ; this will trigger an exception
        jmp short .halt1                ; should not happen

.syms:  mov eax, [rdi+12]               ; ax = section number
        and eax, edx                    ; clear upper bits and test zero
        jz short .halt2                 ; externals are not allowed
        cmp eax, 0x0004                 ; test upper limit
        ja short .next                  ; ("forward compatibility")
        mov eax, [rsp+4*rax-4]          ; eax = section address

        cmp dword [byte rdi+0], strict dword START_1
        jne short .fix
        cmp dword [byte rdi+4], strict dword START_2
        jne short .fix

        mov ebp, [rdi+8]                ; ebp = symbol value
        lea ebp, [rax+rbp]              ; ebp = start address
.fix:   add [rdi+8], eax                ; fix symbol value
        jc short .halt2                 ; simple validity test
.next:  lea edi, [rdi+18]               ; edi = next symbol
        sub rcx, 1                      ; decrement symbols
        jnz short .syms
        mov esi, (object_file+20)       ; esi = text section entry offset
        jmp short relocate
.halt2: hlt                             ; this will trigger an exception
        jmp short .halt2                ; should not happen

relocate:
        mov ecx, [rbx+rsi+32]           ; cx = relocation entries
        and ecx, 0xFFFF                 ; clear upper bits and test zero
        jz short .next
        mov eax, [rbx+rsi+20]           ; eax = section data offset
        add eax, ebx                    ; eax = section data address
        mov edx, [rbx+rsi+24]           ; edx = relocation entries offset
        add edx, ebx                    ; edx = relocation entries address
        mov [byte rsp+0], ecx           ; "counter"
        mov [byte rsp+4], eax           ; "section data"
        mov [byte rsp+8], edx           ; "relocation entry"
        mov eax, [rbx+object_file+8]    ; eax = symbol table offset
        add eax, ebx                    ; eax = symbol table address
        mov [rsp+12], eax               ; "symbol table"
.fix:   call relocation_entry           ; relocate
        add dword [rsp+8], 10           ; next entry
        sub dword [rsp], 1              ; decrement counter
        jnz short .fix

.next:  lea esi, [rsi+40]               ; next section entry
        cmp esi, (object_file+140)      ; test end of relocatable sections
        jb short relocate
        je short jump_to_start          ; ("extra safety", must be equal)
.halt:  hlt                             ; this will trigger an exception
        jmp short .halt                 ; should not happen

jump_to_start:
        add esp, 16                     ; free stack space
        test ebp, ebp                   ; test start address
        jz short .halt
        push rbp                        ; push start address
        call .func
.halt:  hlt                             ; this will trigger an exception
        jmp short .halt                 ; should not happen
.func:  xor eax, eax                    ; eax = 0
        xor ecx, ecx                    ; ecx = 0
        xor edx, edx                    ; edx = 0
        xor ebp, ebp                    ; ebp = 0
        xor esi, esi                    ; esi = 0
        xor edi, edi                    ; edi = 0
        add esp, 8                      ; adjust return address
        ret

relocation_entry:
        push rbx                        ; save register rbx
        push rsi                        ; save register rsi
        mov eax, 18                     ; eax = symbol size
        xor ecx, ecx                    ; ecx = 0
        lea ebx, [rsp+24]               ; ebx = address of structure

        ; [rbx+0x00] = "counter"
        ; [rbx+0x04] = "section data"
        ; [rbx+0x08] = "relocation entry"
        ; [rbx+0x0C] = "symbol table"

        mov esi, [rbx+8]                ; esi = relocation entry
        or cx, [rsi+8]                  ; ecx = type
        jz short .end                   ; ignore zero type
        mov edi, [rsi+4]                ; edi = symbol table index
        mul edi                         ; eax = symbol table offset
        mov edi, [rsi+0]                ; edi = target offset
        add edi, [rbx+4]                ; edi = target address
        mov esi, [rbx+12]               ; esi = symbol table base
        lea esi, [rsi+rax]              ; esi = symbol table entry
        mov eax, [rsi+8]                ; eax = symbol value

.t1:    cmp ecx, 2                      ; test types 1 and 2
        je short .t2
        ja short .t3
        db 0x48                         ; type 1 is "add [rdi], rax"
.t2:    add [byte rdi], eax             ; relocate
        jmp short .end

.t3:    cmp ecx, 3                      ; test type 3
        jne short .t4
        sub eax, [rbx+4]                ; subtract section base
        add [rdi], eax                  ; relocate
        jmp short .end

.t4:    cmp ecx, 9                      ; test types 4-9
        ja short .dbg
        lea edx, [rdi+rcx]              ; edx = target + distance
        sub eax, edx                    ; eax = "relative"
        add [rdi], eax                  ; relocate
        jmp short .end

.dbg:   mov ebx, [rbx+8]                ; ebx = relocation entry
        mov eax, [rbx+0]                ; eax = target offset
        mov edx, [rbx+4]                ; edx = index
        xor ebx, ebx                    ; ebx = 0
.halt:  hlt                             ; this will trigger an exception
        jmp short .halt                 ; should not happen
.end:   pop rsi                         ; restore register rsi
        pop rbx                         ; restore register rbx
        ret

times 0x1A0 - ($ - $$) db 0
object_file:
