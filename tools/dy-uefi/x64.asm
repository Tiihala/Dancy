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
;; dy-uefi/x64.asm
;;      PE32+ UEFI Image
;;
;; Build instructions
;;      nasm -f bin -o x64.bin x64.asm
;;

        org 0
        bits 16

dos_stub_header:
        db 'MZ'                         ; magic (Mark Zbikowski)
        dw 0x0000                       ; bytes used in the last block
        dw 0x0004                       ; blocks
        dw 0x0000                       ; relocation entries
        dw 0x0020                       ; header size / 16
        dw 0x0000                       ; additional memory (min)
        dw 0xFFFF                       ; additional memory (max)
        dw 0x0000                       ; relative SS
        dw 0x0400                       ; initial SP
        dw 0x0000                       ; checksum
        dw 0x0040                       ; initial IP
        dw 0x0000                       ; relative CS
        dw 0x0040                       ; relocation entry offset
        dw 0x0000                       ; zero for a normal program

times 0x003C - ($ - $$) db 0
        dw (pe_uefi_header)             ; pointer to pe_uefi_header

times 0x0200 - ($ - $$) db 0
dos_stub_data:
        db 'Dancy Operating System'
        db 0x0D, 0x0A, 0x24

times 0x0240 - ($ - $$) db 0
dos_stub_code:
        mov ax, cs                      ; ax = code segment
        mov ds, ax                      ; use the same segment
        mov ax, 0x0900                  ; write message
        xor dx, dx                      ; offset (dos_stub_data)
        int 0x21                        ; dos system call
        mov ax, 0x4C00                  ; exit with zero
        int 0x21                        ; dos system call
        db 0xEB, 0xFE                   ; endless loop

times 0x0800 - ($ - $$) db 0
pe_uefi_header:
        db 'PE', 0x00, 0x00             ; signature (Portable Executable)

        dw 0x8664                       ; machine (AMD64)
        dw 0x0002                       ; number of sections
        dd 0x00000000                   ; time date stamp
        dd 0x00000000                   ; pointer to symbol table
        dd 0x00000000                   ; number of symbols
        dw 0x00F0                       ; size of optional header
        dw 0x0022                       ; characteristics

        dw 0x020B                       ; magic number (PE32+)
        db 0x00                         ; major linker version
        db 0x00                         ; minor linker version
        dd 0x00001000                   ; size of code
patch1: dd 0x00001000                   ; size of initialized data
        dd 0x00000000                   ; size of uninitialized data
        dd 0x00001000                   ; address of entrypoint
        dd 0x00001000                   ; base of code

        dq 0x0000000180000000           ; imagebase
        dd 0x00001000                   ; section alignment
        dd 0x00001000                   ; file alignment
        dw 0x0000                       ; major operating system version
        dw 0x0000                       ; minor operating system version
        dw 0x0000                       ; major image version
        dw 0x0000                       ; minor image version
        dw 0x0000                       ; major subsystem version
        dw 0x0000                       ; minor subsystem version
        dd 0x00000000                   ; win32 version value
patch2: dd 0x00003000                   ; size of image
        dd 0x00001000                   ; size of headers
        dd 0x00000000                   ; checksum
        dw 0x000A                       ; subsystem
        dw 0x0000                       ; dll characteristics
        dq 0x0000000000010000           ; size of stack reserve
        dq 0x0000000000001000           ; size of stack commit
        dq 0x0000000000010000           ; size of heap reserve
        dq 0x0000000000001000           ; size of heap commit
        dd 0x00000000                   ; loader flags
        dd 0x00000010                   ; number of rva and sizes

        dd 0x00000000, 0x00000000       ; export table
        dd 0x00000000, 0x00000000       ; import table
        dd 0x00000000, 0x00000000       ; resource table
        dd 0x00000000, 0x00000000       ; exception table
        dd 0x00000000, 0x00000000       ; certificate table
        dd 0x00000000, 0x00000000       ; base relocation table
        dd 0x00000000, 0x00000000       ; debug
        dd 0x00000000, 0x00000000       ; architecture
        dd 0x00000000, 0x00000000       ; global ptr
        dd 0x00000000, 0x00000000       ; tls table
        dd 0x00000000, 0x00000000       ; load config table
        dd 0x00000000, 0x00000000       ; bound import
        dd 0x00000000, 0x00000000       ; iat
        dd 0x00000000, 0x00000000       ; delay import descriptor
        dd 0x00000000, 0x00000000       ; clr runtime header
        dd 0x00000000, 0x00000000       ; reserved

        db '.text', 0x00, 0x00, 0x00    ; name
        dd 0x00001000                   ; virtual size
        dd 0x00001000                   ; virtual address
        dd 0x00001000                   ; size of raw data
        dd 0x00001000                   ; pointer to raw data
        dd 0x00000000                   ; pointer to relocations
        dd 0x00000000                   ; pointer to line numbers
        dw 0x0000                       ; number of relocations
        dw 0x0000                       ; number of line numbers
        dd 0x60000020                   ; characteristics

        db '.data', 0x00, 0x00, 0x00    ; name
patch3: dd 0x00001000                   ; virtual size
        dd 0x00002000                   ; virtual address
patch4: dd 0x00001000                   ; size of raw data
        dd 0x00002000                   ; pointer to raw data
        dd 0x00000000                   ; pointer to relocations
        dd 0x00000000                   ; pointer to line numbers
        dw 0x0000                       ; number of relocations
        dw 0x0000                       ; number of line numbers
        dd 0xC0000040                   ; characteristics

        bits 64

%define EFI_AllocatePages               0x28
%define EFI_BootServices                0x60
%define EFI_ConOut                      0x40
%define EFI_Exit                        0xD8
%define EFI_FreePages                   0x30
%define EFI_OutputString                0x08
%define EFI_Stall                       0xF8

%define PAGES_TO_ALLOCATE               0x200

times 0x1000 - ($ - $$) db 0
uefi_text_section:
        push r12                        ; save register r12
        lea r12, [rsp]                  ; r12 = original stack pointer - 8
        and rsp, 0xFFFFFFFFFFFFFFF0     ; align stack pointer
        push rcx                        ; push "ImageHandle"
        push rdx                        ; push "SystemTablePointer"
        push r12                        ; push register r12
        push 0                          ; push 0 (address of allocated memory)

        mov ecx, 1                      ; rcx = "Type", "AllocateMaxAddress"
        mov edx, 1                      ; rdx = "MemoryType", "EfiLoaderCode"
        db 0x41, 0xB8                   ; r8 = "Pages"
        dd PAGES_TO_ALLOCATE            ; number of pages

        lea r9, [rsp]                   ; address of allocated memory
        mov dword [r9], 0xEFFFFFFF      ; r9 = "Memory"
        mov rax, [rsp+16]               ; rax = "SystemTablePointer"
        mov rax, [rax+EFI_BootServices] ; rax = "BootServices"
        sub rsp, 32                     ; stack shadow space (allocate)
        call [rax+EFI_AllocatePages]    ; call UEFI function
        test rax, rax                   ; test zero
        jz short copy_object_file

        mov rcx, [rsp+48]               ; rcx = "SystemTablePointer"
        mov rcx, [rcx+EFI_ConOut]       ; rcx = "ConOut"
        lea rdx, [rel alloc_error]      ; rdx = address of message
        call [rcx+EFI_OutputString]     ; call UEFI function

        lea rcx, [5000000]              ; rcx = "Microseconds"
        mov rax, [rsp+48]               ; rax = "SystemTablePointer"
        mov rax, [rax+EFI_BootServices] ; rax = "BootServices"
        call [rax+EFI_Stall]            ; call UEFI function
        mov rsp, r12                    ; restore original stack pointer - 8

        ; "The registers rbx, rbp, rdi, rsi, r12, r13, r14, r15 are
        ;  considered nonvolatile and must be saved and restored."

        pop r12                         ; restore register r12
        ret                             ; return to firmware

copy_object_file:
        mov eax, [rsp+32]               ; rax = allocated memory (low dword)
        mov rdi, [rsp+32]               ; rdi = allocated memory (full)
        sub rdi, rax                    ; test high dword
        jnz short .halt
        xchg rax, rdi                   ; rax = 0
                                        ; rdi = address of allocated memory

        cmp edi, 0xEFFFFFFF             ; test validity
        jae short .halt
        test edi, 0x00000FFF            ; test validity
        jnz short .halt

        mov ecx, [rel patch4]           ; rcx = "size of raw data (data)"
        lea rsi, [rel uefi_object_file] ; source data
        cld                             ; clear direction flag
        rep movsb                       ; move data to allocated memory

        mov ecx, 4096*PAGES_TO_ALLOCATE ; rcx = 4096*PAGES_TO_ALLOCATE
        sub ecx, [rel patch4]           ; rcx = sub "size of raw data (data)"
        jc short .halt                  ; (should not happen)
        cld                             ; clear direction flag
        rep stosb                       ; clear allocated memory (al = 0)

        add rsp, 32                     ; adjust stack pointer
        pop rbx                         ; rbx = address of allocated memory
        cmp r12, [byte rsp+0]           ; stack pointer test
        jne short .halt
        add rsp, 8                      ; adjust stack pointer
        pop r9                          ; r9 = "SystemTablePointer"
        pop r8                          ; r8 = "ImageHandle"
        sub rsp, 16                     ; allocate stack space (16 bytes)
        lea rsi, [rbx]                  ; esi = object file
        jmp short uefi_in_x64
.halt:  hlt                             ; halt
        jmp short .halt

uefi_in_x64:
        mov ecx, [byte rsi+0]           ; first dword of object file
        mov ebp, [byte rsi+16]          ; check optional header and flags
        sub ecx, (0x8664+0x40000)       ; test magic word and sections
        or ecx, ebp                     ; both registers must be zero
        jnz short .halt1                ; simple validity test

        mov eax, [rel patch4]           ; eax = data size
        mov [rsi+(20+(3*40)+20)], eax   ; set bss offset
        lea rdi, [rsp]                  ; rdi = array of four stack variables

        mov ecx, 4                      ; ecx = four sections
        lea edx, [rsi+20+20]            ; edx = first section
.secs:  mov eax, [rdx]                  ; eax = section offset
        add eax, ebx                    ; eax = text address
        jc short .halt1                 ; simple validity test
        mov [rdi], eax                  ; save
        lea rdx, [rdx+40]               ; next section
        lea rdi, [rdi+4]                ; next stack variable
        sub ecx, 1                      ; decrement counter
        jnz short .secs

        mov ecx, [rsi+12]               ; ecx = number of symbols
        mov edx, 0x0000FFFF             ; edx = 0x0000FFFF (bit mask)
        mov edi, [rsi+8]                ; edi = symbol table offset
        lea edi, [rbx+rdi]              ; edi = symbol table address
        jmp short .syms
.halt1: hlt                             ; halt
        jmp short .halt1                ; should not happen

.syms:  mov eax, [rdi+12]               ; ax = section number
        and eax, edx                    ; clear upper bits and test zero
        jz short .halt2                 ; externals are not allowed
        cmp eax, 0x0004                 ; test upper limit
        ja short .next                  ; ("forward compatibility")
        mov eax, [rsp+4*rax-4]          ; eax = section address

        cmp dword [byte rdi+0], strict dword 'star'
        jne short .fix
        cmp dword [byte rdi+4], strict dword 't'
        jne short .fix

        mov ebp, [rdi+8]                ; ebp = symbol value
        lea ebp, [rax+rbp]              ; ebp = start address
.fix:   add [rdi+8], eax                ; fix symbol value
        jc short .halt2                 ; simple validity test
.next:  lea edi, [rdi+18]               ; edi = next symbol
        sub rcx, 1                      ; decrement symbols
        jnz short .syms
        mov esi, 20                     ; esi = text section entry offset
        jmp short relocate
.halt2: hlt                             ; halt
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
        mov eax, [dword rbx+8]          ; eax = symbol table offset
        add eax, ebx                    ; eax = symbol table address
        mov [rsp+12], eax               ; "symbol table"
.fix:   call relocation_entry           ; relocate
        add dword [rsp+8], 10           ; next entry
        sub dword [rsp], 1              ; decrement counter
        jnz short .fix

.next:  lea esi, [rsi+40]               ; next section entry
        cmp esi, 140                    ; test end of relocatable sections
        jb short relocate
        je short jump_to_start          ; ("extra safety", must be equal)
.halt:  hlt                             ; halt
        jmp short .halt                 ; should not happen

jump_to_start:
        xor eax, eax                    ; rax = 0
        mov rsp, r12                    ; restore original stack pointer - 8
        mov [rsp], rax                  ; write zero

        ; Data section is used for saving "ImageHandle", "SystemTablePointer"
        ; and "ObjectFile" for the "exit_to_firmware" procedure.

        lea rax, [rel (uefi_object_file+0xC0)]
        and rsp, 0xFFFFFFFFFFFFFFF0     ; align stack pointer

        mov [byte rax+0x00], r8         ; save "ImageHandle"
        mov [byte rax+0x08], r9         ; save "SystemTablePointer"
        mov [byte rax+0x10], rbx        ; save "ObjectFile"

        mov rcx, r8                     ; rcx = "ImageHandle"
        mov rdx, r9                     ; rdx = "SystemTablePointer"
        mov r8, rbx                     ; r8 = "ObjectFile"

        lea rax, [rel exit_to_firmware] ; rax = return address
        push 0                          ; "sub rsp, 8" (shadow space)
        push r8                         ; "sub rsp, 8" (shadow space)
        push rdx                        ; "sub rsp, 8" (shadow space)
        push rcx                        ; "sub rsp, 8" (shadow space)
        push rax                        ; return address (exit_to_firmware)

        cmp ebp, strict dword 0         ; test start address
        je short .go
        push rbp                        ; return address (start)

.go:    xor eax, eax                    ; rax = 0
        xor ebx, ebx                    ; rbx = 0
        xor ebp, ebp                    ; rbp = 0
        xor esi, esi                    ; rsi = 0
        xor edi, edi                    ; rdi = 0
        xor r9, r9                      ; r9 = 0
        xor r10, r10                    ; r10 = 0
        xor r11, r11                    ; r11 = 0
        xor r12, r12                    ; r12 = 0
        xor r13, r13                    ; r13 = 0
        xor r14, r14                    ; r14 = 0
        xor r15, r15                    ; r15 = 0
        ret

exit_to_firmware:
        lea rbx, [rel (uefi_object_file+0xC0)]

        mov rcx, [rbx+0x10]             ; rcx = "ObjectFile"
        mov edx, PAGES_TO_ALLOCATE      ; edx = "Pages"
        mov rax, [rbx+0x08]             ; rax = "SystemTablePointer"
        mov rax, [rax+EFI_BootServices] ; rax = "BootServices"
        call [rax+EFI_FreePages]        ; call UEFI function
        test rax, rax                   ; test zero
        jnz short .halt

        mov rcx, [rbx]                  ; rcx = "ImageHandle"
        xor rdx, rdx                    ; rdx = "ExitStatus"
        xor r8, r8                      ; r8 = "ExitDataSize"
        xor r9, r9                      ; r9 = "ExitData"
        mov rax, [rbx+0x08]             ; rax = "SystemTablePointer"
        mov rax, [rax+EFI_BootServices] ; rax = "BootServices"
        call [rax+EFI_Exit]             ; call UEFI function
.halt:  hlt                             ; halt
        jmp near .halt

relocation_entry:
        push rbx                        ; save register rbx
        push rsi                        ; save register rsi
        xor eax, eax                    ; eax = 0
        xor ecx, ecx                    ; ecx = 0
        lea rbx, [rsp+24]               ; rbx = address of structure

        ; [rbx+0x00] = "counter"
        ; [rbx+0x04] = "section data"
        ; [rbx+0x08] = "relocation entry"
        ; [rbx+0x0C] = "symbol table"

        mov al, 18                      ; eax = symbol size
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
.halt:  hlt                             ; halt
        jmp short .halt                 ; should not happen
.end:   pop rsi                         ; restore register rsi
        pop rbx                         ; restore register rbx
        ret

align 128, db 0
alloc_error:
dw __utf16__('AllocatePages: out of resources'), 13, 10, 0

times 0x2000 - ($ - $$) db 0
uefi_object_file:
        dd patch1                       ; offset of "size of initialized data"
        dd patch2                       ; offset of "size of image"
        dd patch3                       ; offset of "virtual size (data)"
        dd patch4                       ; offset of "size of raw data (data)"

times 0x2020 - ($ - $$) db 0
