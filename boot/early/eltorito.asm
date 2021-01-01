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
;; eltorito.asm
;;      El Torito boot loader (ISO 9660 CD-ROM)
;;
;; Build instructions
;;      nasm -f bin -o eltorito.bin eltorito.asm
;;

        BITS 16
        CPU 8086
        ORG 0x7400

ElToritoBoot:
        mov ax, 'EL'                    ; signature
        jmp short .L1                   ; (16-bit mode continues)
        hlt                             ; (32-bit mode)
        jmp short ElToritoBoot          ; (32-bit mode halts)

        ; This first part of code is position-independent.

times 32 - ($ - $$) db 0
.L1:    xor ax, ax                      ; ax = 0x0000
        mov es, ax                      ; es = 0x0000
        mov ds, ax                      ; ds = 0x0000
        mov cx, 2048 / 2                ; cx = bytes to copy / 2
        mov si, 0x7C00                  ; ds:si = source
        mov di, 0x7400                  ; es:di = destination
        cli                             ; disable interrupts
        mov ss, ax                      ; ss = 0x0000 ("uninterruptible")
        mov sp, di                      ; sp = 0x7400
        sti                             ; enable interrupts
        cld                             ; clear direction flag
        rep movsw                       ; relocate this code
        mov bp, 0x0000                  ; bp = 0x0000
        jmp 0x0000:.L2                  ; jump to relocated code
                                        ; (0x0000:0x7400)

.L2:    ; This part of code is position-dependent.

CheckExtensions:
        push ax                         ; push 0x0000 (for es)
        push ax                         ; push 0x0000 (for ds)
        push dx                         ; save register dx
        stc                             ; set carry flag
        mov bx, 0x55AA                  ; bx = magic number
        mov ah, 0x41                    ; ah = 0x41, installation check
        int 0x13                        ; bios call
        sti                             ; enable interrupts
        cld                             ; clear direction flag
        pop dx                          ; restore register dx
        pop ds                          ; ds = 0x0000
        pop es                          ; es = 0x0000
        jc short .Err
        shr cx, 1                       ; move bit 0 (ext. available) to cf
        adc bx, 0x55AA                  ; check magic number
                                        ; (0xAA55 + 0x55AA + 0x0001 == 0x0000)
        jz short FindBootImage
.Err:   mov si, Err1                    ; "read functions not supported"
        jmp near Error

        ; If disk read extensions are available, the CPU is assumed to
        ; be 80386 compatible and 32-bit instructions can be used.

        CPU 386

FindBootImage:
        xor ebx, ebx                    ; ebx = 0x00000000
        mov dh, 0x06                    ; dh = general retry counter
        mov bl, 0x10                    ; read primary volume descriptor
        mov bp, 0x0001                  ; bp = 0x0001, read one block
        mov si, Err2                    ; si = error message, "I/O"
        mov di, 0x7C00                  ; di = 0x7C00
.Read:  call ReadBlocks                 ; read one block
        jc short .Read                  ; (this is not an endless loop)

        cmp word [di+128], 2048         ; test block size
        jne short JumpToError
        mov si, Err3                    ; si = error message, "ISO 9660"

        mov ecx, 0x00000800             ; ecx = 0x00000800
        mov ebx, [di+156+2]             ; ebx = "location of extent"
        mov eax, [di+156+10]            ; eax = "data length"
        add eax, 0x000007FF             ; add 0x000007FF
        and eax, 0xFFFFF800             ; eax = "data length" (full blocks)
        shr eax, 11                     ; eax = blocks to read
        jz short JumpToError            ; (defensive programming)
        mov ebp, 0x00000010             ; bp = 16 (maximum)
        cmp eax, ebp                    ; limit the maximum size
        ja short ReadRootDirectory
        mov ebp, eax                    ; bp = blocks to read

ReadRootDirectory:
        mov al, 0x00                    ; al = 0x00
        mov cx, 0x8400                  ; cx = 0x8400
        cld                             ; clear direction flag
        rep stosb                       ; clear memory from 0x7C00 to 0xFFFF
        mov si, Err2                    ; si = error message, "I/O"
.Read:  call ReadBlocks                 ; read blocks
        jc short .Read                  ; (this is not an endless loop)

FindImageFile:
        mov cl, [ImageFileStruct]       ; cx = size of "ImageFileStruct"
        mov si, ImageFileStruct         ; si = address of "ImageFileStruct"
        mov di, 0x7C20                  ; di = 0x7C20
.Check: test byte [di-7], 0x02          ; test directory flag
        jnz short .Next
        cmp cl, [di]                    ; test one byte first
        jne short .Next
        pusha                           ; save all registers
        cld                             ; clear direction flag
        rep cmpsb                       ; compare bytes
        popa                            ; restore all registers
        je short ImageFileFound
.Next:  xor ax, ax                      ; ax = 0x0000
        or al, [di-0x20]                ; al = size of directory record
        jz short .Stop
        add di, ax                      ; di = next record
        jmp short .Check
.Stop:  mov si, Err4                    ; si = error message, "file not found"

JumpToError:
        jmp near Error

ImageFileFound:
        mov ebx, [di-0x1E]              ; ebx = "location of extent"
        mov bp, 0x0001                  ; bp = 0x0001, read one block
        mov si, Err2                    ; si = error message, "I/O"
.Read:  call ReadBlocks                 ; read one block
        jc short .Read                  ; (this is not an endless loop)
        mov dh, 0x00                    ; dh = 0x00
        mov si, Err5                    ; si = error message, "image format"
        cmp word [0x7C00], 0x3CEB       ; test jump instruction
        jne short Error
        cmp word [0x7C0B], 0x0800       ; test block size
        jne short Error

        ; This is the end of this El Torito boot loader. One of the key things
        ; is to update the "hidden sectors" value so that the next phase knows
        ; its absolute location on the media. Otherwise, it should work just
        ; like a "partitioned" hard disk drive with 2048-byte sectors.

        mov [0x7C1C], ebx               ; update "hidden sectors"
        xor ax, ax                      ; ax = 0x0000
        xor cx, cx                      ; cx = 0x0000
        xor si, si                      ; si = 0x0000
        xor di, di                      ; di = 0x0000
        jmp near 0x7C00                 ; jump to the boot sector

align 16
ReadBlocks:
        pushad                          ; save all registers
        mov ax, 0x7200                  ; ax = 0x7200
        mov cx, 0x0020                  ; cx = 0x0020 (buffer size + extra)
        mov di, 0x7200                  ; di = 0x7200 (buffer address)
        cld                             ; clear direction flag (critical)
        rep stosb                       ; clear buffer and some extra bytes
        mov si, ax                      ; si = 0x7200
        mov byte [byte si+0x00], 0x10   ; size
        mov word [byte si+0x02], bp     ; number of blocks
        mov byte [byte si+0x05], 0x7C   ; transfer buffer (0x7C00)
        mov dword [byte si+0x08], ebx   ; lba
        stc                             ; set carry flag
        mov dh, 0x00                    ; dl = drive number, dh = 0x00
        mov ah, 0x42                    ; ah = 0x42 (al = 0x00), extended read
        int 0x13                        ; bios call
        sti                             ; enable interrupts
        popad                           ; restore all registers
        jnc short .L1
        dec dh                          ; decrement counter (preserve cf)
        jz short Error
.L1:    ret

Error:
        ; This part is 8086 compatible.
        CPU 8086

        mov ah, 0x0F                    ; get active page -> register bh

        ; The code below assumes that direction flag is cleared because
        ; it was explicitly cleared at the beginning of this code. Also,
        ; the code below assumes that register bx (bh = active page) is
        ; preserved. Both of these things are not considered to be very
        ; critical (or likely to happen) because this is a non-return
        ; error handler.

.L1:    push si                         ; save register si
        int 0x10                        ; bios call
        sti                             ; enable interrupts (for halt loop)
        mov bl, 0x07                    ; bl = color, bh = active page
        pop si                          ; restore register si
        lodsb                           ; al = [si++] (character)
        and al, 0xFF                    ; test zero
        mov ah, 0x0E                    ; teletype output
        jnz short .L1                   ; loop
.Halt:  hlt                             ; halt
        jmp short .Halt                 ; endless loop (interrupts enabled)

Err1:   db '! INT 0x13, AH=0x42 is not supported', 13, 0
Err2:   db '! I/O error', 13, 0
Err3:   db '! ISO 9660 error', 13, 0
Err4:   db '! Image not available', 13, 0
Err5:   db '! Image format error', 13, 0

ImageFileStruct:
        db (ImageFileStructEnd - ImageFileName)
ImageFileName:
        db 'DANCY.IMG;1'
ImageFileStructEnd:

times 510 - ($ - $$) db 0
BootSignature:
        db 0x55, 0xAA

times 2048 - ($ - $$) db 0
ElToritoBootEnd:
