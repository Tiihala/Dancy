;;
;; Copyright (c) 2017, 2018 Antti Tiihala
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
;; gpt.asm
;;      General purpose implementation of Master Boot Record (GPT)
;;
;; Build instructions
;;      nasm -f bin -o gpt.bin gpt.asm
;;

        BITS 16
        ORG 0x0600

MasterBootRecord:
        ; This part is 8086 compatible.
        CPU 8086

        mov ax, 'AT'                    ; signature
        jmp short SetupStack            ; (16-bit mode continues)
        jmp short MasterBootRecord      ; (32-bit or 64-bit mode halts)

times 7 - ($ - $$) db 0
StatusByte:
        db 0x00                         ; 0x00 = primary GPT table
                                        ; 0xFF = alternate GPT table
ReadLbaVariable:
        dd 0x00000001, 0x00000000       ; preset to LBA 1, do not change
                                        ; (this is a multipurpose variable)

        ; The memory area from offset 0x0009 to offset 0x000F contains
        ; zero bytes. It should make detecting the format a bit more
        ; robust, e.g. whether it is an emulated floppy or an HDD (USB).

times 16 - ($ - $$) db 0
BootPartitionGuid:
        db 'GUID_BOOTMANAGER'           ; Offset 0x0010

        ; A disk format tool writes the GUID. It is a good practice to
        ; check this specific string before patching the binary image.

        ; This first part of code is position-independent.

times 32 - ($ - $$) db 0
SetupStack:
        xor ax, ax                      ; ax = 0x0000
        mov es, ax                      ; es = 0x0000
        mov ds, ax                      ; ds = 0x0000
        mov cx, 512 / 2                 ; cx = bytes to copy / 2
        mov si, 0x7C00                  ; ds:si = source
        mov di, 0x0600                  ; es:di = destination
        cli                             ; disable interrupts
times 48 - ($ - $$) db 0
        db 0x8E, 0xD0                   ; (bit 0 is clear)
        ; mov ss, ax                    ; ss = 0x0000 ("uninterruptible")
        mov sp, 0x7C00                  ; sp = 0x7C00
        sti                             ; enable interrupts
        cld                             ; clear direction flag
        rep movsw                       ; relocate boot sector
        mov dh, 0x03                    ; dh = 0x03 (general retry counter)
        jmp 0x0000:CheckExtensions      ; cs = 0x0000

        ; It is very unlikely that "AH=0x41 INT 0x13" modifies segments es
        ; and ds or direction flag. As a defensive practice, those are set
        ; afterwards because it costs practically nothing ("stack usage").

        ; This part of code is position-dependent.

times 64 - ($ - $$) db 0
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
        jz short ReadGptHeader
.Err:   mov si, Err1                    ; lba error
        jmp near Error

        ; If disk read extensions are available, the CPU is assumed to
        ; be 80386 compatible and 32-bit instructions can be used.

        CPU 386

        ; The "ReadGptHeader" may be executed twice, for the primary table and
        ; for the alternate table. The former has a brute force search. If the
        ; primary table is not at LBA 1, we try LBA 2, 4, and 8. According to
        ; the results, the "ReadLba" is patched to support GPT LBAs.

times 96 - ($ - $$) db 0
ReadGptHeader:
        mov si, ReadLbaVariable         ; si = address of "ReadLbaVariable"
        call ReadLba                    ; try to read gpt header
        jc short ReadGptHeader          ; (this is not an endless loop)
        mov ax, 0x0008                  ; ax = 0x0008
        mov cx, ax                      ; cx = 0x0008
        mov si, EfiPrt                  ; ds:si = source
        mov di, 0x8000                  ; es:di = destination
        repe cmpsb                      ; test signature
        mov si, Err2                    ; error,  "? EFI PART"
        je short .L1

        cmp ah, [StatusByte]            ; primary or alternate table?
        jne short ErrorJump             ; (if not primary, no more patching)
        shl byte [ReadLba.Patch], 1     ; patch number of blocks to read
        cmp al, [ReadLba.Patch]         ; test maximum value (8)
        jae ReadGptHeader               ; try to find the header
        jmp short ErrorJump

.L1:    mov eax, [di-8+32]              ; eax = alternate lba (low dword)
        mov [ReadLbaVariable+0], eax    ; (if trying the alternate table)
        mov eax, [di-8+36]              ; eax = alternate lba (high dword)
        mov [ReadLbaVariable+4], eax    ; (if trying the alternate table)
        push dword [di-8+76]            ; push partition entry lba (high)
        push dword [di-8+72]            ; push partition entry lba (low)

        mov ax, [di-8+40]               ; ax = first usable lba
                                        ; (case zero will be checked)
        sub ax, strict word 0x0003      ; ax = first usable lba - 3
        mov bx, [di-8+84]               ; bx = size of the partition entry
                                        ; (case zero will be checked)
        dec bx                          ; bx = size of the partition entry - 1

ReadPartitionTable:
        mov si, sp                      ; si = address of partition entry lba
        mov di, 0x8010                  ; es:di = "UniquePartitionGuid"
        pusha                           ; save all registers
        call ReadLba                    ; try to read partition table
        popa                            ; restore all registers

        ; It was a carefully made decision to ignore the read error and rely
        ; on searching the boot manager GUID. The buffer was cleared so the
        ; data are from the disk. The search will not stop if there are some
        ; erroneous disk blocks.

.Find:  mov cl, 16                      ; cl = 16, ch = 0x00
        mov si, BootPartitionGuid       ; ds:si = "BootPartitionGuid"
        push di                         ; save register di
        repe cmpsb                      ; compare
        je short BootManagerEntryFound
        pop di                          ; restore register di

        ; It is very important that "ReadLba" clears the buffer. This will
        ; always search the whole 4096-byte buffer because it significantly
        ; simplifies the logic of handling different block sizes. Please
        ; note that there is no extra overhead if the boot partition entry
        ; entry is in the first block.

        inc di                          ; increment di
        add di, bx                      ; di = next entry
        jc short TryAlternateTable      ; catch partition entry zero size
                                        ; (bx - 1 -> 0xFFFF -> overflow)
        cmp di, 0x9000                  ; test end of block
        jb short .Find

        mov si, sp                      ; si = address of partition entry lba
        add dword [si], 0x00000001      ; increment lba (low dword only)
        jc short TryAlternateTable      ; assumed that this does not happen

        ; This slightly weakens the _alternate_ table handling but saves
        ; some bytes. The behavior is safe and defined if "jc jump" happens.

        dec ax                          ; decrement "blocks to read"
        jns short ReadPartitionTable

TryAlternateTable:
        sub byte [StatusByte], 0x01     ; decrement status byte
        jc short ReadGptHeader          ; (try the alternate table if 0xFF)
        mov si, Err2                    ; error message
ErrorJump:
        jmp near Error

BootManagerEntryFound:
        mov esp, 0x00007C00             ; esp = 0x00007C00
        test byte [di+16], 1            ; test attribute bit 0
        jz short TryAlternateTable      ; (stack is ok)

        ; This bit 0 test is more important than it may look like. If LBA 0
        ; (MBR) is accidentally read instead of a partition array block, it
        ; has the correct GUID (offset 0x10). This bit detects that case and
        ; that scenario is _not_ considered extremely unlikely because zero
        ; is a little bit special value. Please make sure that offset 48 of
        ; this MBR does not have bit 0 set.

        push dword [di+4]               ; push boot manager lba (high dword)
        push dword [di]                 ; push boot manager lba (low dword)

ReadBootManager:
        mov si, sp                      ; si = boot manager lba
        call ReadLba                    ; try to read boot partition
        jc short ReadBootManager        ; (this is not an endless loop)
        cmp word [0x7DFE], 0xAA55       ; test magic value
        jne short TryAlternateTable     ; (stack is ok)
        mov dh, [ReadLba.Patch]         ; dh = gpt block size in sectors

PrepareStackFrame:
        push dx                         ; save register dx
        mov cx, 18                      ; for a "popad" instruction + 2
.L1:    push cs                         ; push word 0x0000
        loop .L1
        pop ds                          ; ds = 0x0000 (defensive programming)
        pop es                          ; es = 0x0000 (defensive programming)

RelocateSector:
        mov ch, 0x01                    ; cx = bytes to copy
        mov si, 0x8000                  ; si = source
        mov di, 0x7C00                  ; di = destination
        cld                             ; clear direction flag
        rep movsw                       ; relocate sector

JumpToNextStage:
        popad                           ; clear general purpose registers
        pop dx                          ; dx = return value

        mov cl, dh                      ; cl = return value
        mov dh, 0x00                    ; dh = 0x00
        mov ebp, 'GPT '                 ; magic string

        ; This is the end of this Master Boot Record. The next stage
        ; takes control of the boot process.
        ;
        ; [esp+4] = (dword) lba (high)
        ; [esp+0] = (dword) lba (low)
        ;
        ; cl = gpt block size in sectors
        ; dl = boot drive number
        ;
        ; eax = 0x00000000      ecx = 0x000000??
        ; edx = 0x000000??      ebx = 0x00000000
        ; esp = 0x00007DF8      ebp = 'GPT '
        ; esi = 0x00000000      edi = 0x00000000
        ;
        ; es = cs = 0x0000
        ; ss = ds = 0x0000
        ; fs = unknown
        ; gs = unknown

        jmp near 0x7C00

times 304 - ($ - $$) db 0
ReadLba:
        push dx                         ; save register dx
        push dword [si]                 ; push lba (low dword)
        push dword [si+0x04]            ; push lba (high dword)
        xor ax, ax                      ; ax = 0x0000
        mov cx, 0x8200                  ; cx = 0x8200
        mov si, 0x7E00                  ; si = 0x7E00
        mov di, si                      ; di = 0x7E00
        cld                             ; clear direction flag (critical)
        rep stosb                       ; clear dap buffer and block buffer
        mov byte [si], 0x10             ; dap size
        ; mov byte [si+0x02], 0x01      ; number of blocks
        db 0xC6, 0x44, 0x02
.Patch: db 0x01                         ; this value can be patched

        ; If "ReadLba" is patched, the input LBA is translated too. The caller
        ; may assume that this procedure reads GPT blocks. The "int 0x13" can
        ; use some other sector size but it must be smaller or equal.

        mov byte [si+0x05], 0x80        ; transfer buffer (0x8000)
        pop ebx                         ; ebx = lba (high dword)
        pop ecx                         ; ecx = lba (low dword)
        mov dh, [ReadLba.Patch]         ; dh = number of blocks
.L1:    add [si+0x08], ecx              ; lba (low dword)
        adc [si+0x0C], ebx              ; lba (high dword)
        jc short .L3                    ; (defensive programming)
        dec dh                          ; decrement counter
        jnz short .L1

.L2:    mov ah, 0x42                    ; ah = 0x42 (al = 0x00), extended read
        int 0x13                        ; bios call
        jnc short .L3                   ; if no errors, return dh = 0x03
        mov ah, 0x00                    ; ah = 0x00, reset disk
        int 0x13                        ; bios call
        pop dx                          ; restore register dx
        stc                             ; set carry flag
        jmp short .L4
.L3:    pop dx                          ; restore register dx
        mov dh, 0x04                    ; reset counter
.L4:    mov si, Err3                    ; disk error
        dec dh                          ; decrement counter (cf preserved)
        jz short Error
        sti                             ; enable interrupts
        ret

Error:
        ; This part is 8086 compatible.
        CPU 8086

        mov ah, 0x0F                    ; get active page -> register bh

        ; The code below assumes that direction flag is cleared because
        ; it was explicitly cleared at the beginning of this MBR. Also,
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

Err1:   db '! LBA', 13, 0               ; "! LBA"
Err2:   db '? '                         ; "? EFI PART"
EfiPrt: db 'EFI PART', 13, 0
Err3:   db '! HDD I/O'                  ; "! HDD I/O"
        db 13, 0

times 440 - ($ - $$) db 0
        db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
ProtectivePartitionTable:
        ; The legacy partition table is not used by this MBR code.

times 510 - ($ - $$) db 0
PartitionTableEnd:
        db 0x55, 0xAA
