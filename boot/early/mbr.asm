;;
;; Copyright (c) 2017 Antti Tiihala
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
;; mbr.asm
;;      General purpose implementation of Master Boot Record
;;
;; Build instructions
;;      nasm -f bin -o mbr.bin mbr.asm
;;
;; Version
;;      2017-09-25      VERSION 1.2
;;

        BITS 16
        CPU 8086
        ORG 0x0600

MasterBootRecord:

        ; Original author _wishes_ that you change the signature
        ; if you change the master boot record code. However, it
        ; is not mandatory according to the license. Also, this
        ; instruction can be changed to "mov dx, <drive number>"
        ; if you want to replace the value given by the previous
        ; stage of boot process (usually BIOS).

        mov ax, 'AT'                    ; signature
        jmp short .L1                   ; (16-bit mode continues)
        hlt                             ; (32-bit mode)
        jmp short MasterBootRecord      ; (32-bit mode halts)

        ; The memory area from offset 0x0008 to offset 0x001F contains
        ; zero bytes. It should make detecting the format more robust,
        ; e.g. whether it is an emulated floppy or a hard disk (USB).

        ; This first part of code is position-independent.

times 32 - ($ - $$) db 0
.L1:    xor ax, ax                      ; ax = 0x0000
        mov es, ax                      ; es = 0x0000
        mov ds, ax                      ; ds = 0x0000
        mov cx, 512 / 2                 ; cx = bytes to copy / 2
        mov si, 0x7C00                  ; ds:si = source
        mov di, 0x0600                  ; es:di = destination
        cli                             ; disable interrupts
        mov ss, ax                      ; ss = 0x0000 ("uninterruptible")
        mov sp, si                      ; sp = 0x7C00
        sti                             ; enable interrupts
        cld                             ; clear direction flag
        rep movsw                       ; relocate boot sector
        mov bp, PartitionTable          ; bp = address of "PartitionTable"
        jmp 0x0000:CheckTable           ; jump to relocated code
                                        ; (0x0000:0x0640)

        ; This part of code is position-dependent.

times 64 - ($ - $$) db 0
CheckTable:
        mov cl, [byte bp+0x00]          ; cl = partition status byte
        shl cl, 1                       ; move bit 7 to carry flag
        sbb ax, strict word 0x0000      ; subtract with carry flag
        jnc short .Next                 ; first active partition?
        lea si, [byte bp+0x00]          ; si = active partition
.Next:  lea bp, [byte bp+0x10]          ; bp = bp + 16
        cmp bp, PartitionTableEnd       ; test end of partition table
        jb short CheckTable             ; loop

        cmp al, 0xFF                    ; only one active partition allowed
        jne short .Err
        mov ax, [byte si+8]             ; test first sector (low word)
        or ax, [byte si+10]             ; (high word)
        jz short .Err                   ; zero is not allowed
        mov ax, [byte si+12]            ; test number of sectors (low word)
        or ax, [byte si+14]             ; (high word)
        jnz short .Ok                   ; zero is not allowed
.Err:   mov si, Msg1                    ; partition table error
        jmp near Error
.Ok:    ; nop

        ; Make sure that code above does not modify register dl
        ; because it contains the "boot drive" number. Register
        ; si contains the address of active partition entry.

CheckExtensions:
        push dx                         ; save register dx
        push si                         ; save register si
        mov bx, 0x55AA                  ; bx = magic number
        stc                             ; set carry flag
        mov ah, 0x41                    ; ah = 0x41, installation check
        int 0x13                        ; bios call
        jc short .No
        cmp bx, 0xAA55                  ; check magic number
        jne short .No
        shr cx, 1                       ; test bit 0 (functions available)
        jc short .Yes
.No:    mov ax, ReadChs                 ; ax = function pointer (chs)
        jmp short .Ok
.Yes:   mov ax, ReadLba                 ; ax = function pointer (lba)
.Ok:    pop si                          ; restore register si
        pop dx                          ; restore register dx

CallReadFunction:
        mov dh, 0x04                    ; dh = 0x04 (counter)
.L1:    mov word [0x7DFE], 0x0000       ; clear signature area in the buffer
        push ax                         ; save register ax
        push dx                         ; save register dx
        push si                         ; save register si
        call ax                         ; call "ReadChs" or "ReadLba"
        pop si                          ; restore register si
        pop dx                          ; restore register dx
        pop ax                          ; restore register ax
        jnc short .L2                   ; skip reset disk if success
        call ResetDisk                  ; reset disk
        sub dh, 0x01                    ; decrement counter
        jnz short .L1                   ; try again
        mov si, Msg3                    ; disk error
        jmp near Error
.L2:    ; nop

JumpToActivePartition:
        mov ax, [0x7DFE]                ; ax = test signature
        sub ax, 0xAA55                  ; test signature and clear ax
        jnz short .Err                  ; if jump, partition is not bootable
        mov cx, es                      ; cx = 0x0000 (segment es)
        mov bx, ds                      ; bx = 0x0000 (segment ds)

        ; Something has gone seriously wrong if segment registers
        ; es and ds are not what they should be. The only correct
        ; action is to minimize damage because unexpected things
        ; could have occured before this.

        or cx, bx                       ; test zero
        jnz short .Err                  ; extremely unlikely (!)
                                        ; (the error message may not work)

        and dh, 0x00                    ; dl = boot drive number, dh = 0x00
        xor bp, bp                      ; bp = 0x0000
        xor di, di                      ; di = 0x0000

        ; This is the end of this Master Boot Record. The next stage,
        ; the Volume Boot Record, takes control of the boot process.
        ;
        ; ax = 0x0000
        ; cx = 0x0000
        ; dx = boot drive number (dh = 0x00)
        ; bx = 0x0000
        ; sp = 0x7C00
        ; bp = 0x0000
        ; si = active partition
        ; di = 0x0000
        ;
        ; es = cs = ss = ds = 0x0000
        ; fs = unknown (if available)
        ; gs = unknown (if available)

        jmp near 0x7C00                 ; jump to active partition

.Err:   mov si, Msg1                    ; partition table error
        jmp near Error

times 208 - ($ - $$) db 0
ReadChs:
        push dx                         ; save register dx
        push si                         ; save register si
        xor cx, cx                      ; cx = 0x0000
        xor di, di                      ; di = 0x0000, clear carry flag

        ; Carry flag is cleared because if the next interrupt does not
        ; correctly _clear_ the flag (success), the whole boot process
        ; stops. For disk read functions, it is assumed that the carry
        ; is handled properly but this "get drive parameters" could be
        ; considered less tested globally. If register cx is not modi-
        ; fied, the booting process stops. If it is modified, then
        ; the function is assumed to have worked fine.

        mov ah, 0x08                    ; ah = 0x08, get drive parameters
        int 0x13                        ; bios call
        jc short .Err

        ; The previous interrupt may change segment es if this is
        ; a real floppy or an emulated floppy. The latter is much
        ; more likely but still relatively unlikely with MBRs.

        xor ax, ax                      ; ax = 0x0000
        mov es, ax                      ; es = 0x0000 ("floppy safe")
        and cx, strict word 0x003F      ; cx = sectors per track
        jz short .Err                   ; (very important, i.e. div by 0)
        mov al, dh                      ; al = maximum head number
        add ax, strict word 0x0001      ; ax = number of heads
        mov bx, ax                      ; bx = number of heads
                                        ; (bx guaranteed to be non-zero)
        pop si                          ; restore register si

        mov bp, sp                      ; bp = stack pointer
        mov ax, [byte si+10]            ; ax = lba (high word)
        xor dx, dx                      ; dx = 0x0000
        div cx                          ; lba / sectors per track (part 1/2)
        push ax                         ; save "temp" (high word)
        mov ax, [byte si+8]             ; ax = lba (low word)
        div cx                          ; lba / sectors per track (part 2/2)
        push ax                         ; save "temp" (low word)
        mov si, dx                      ; si = remainder
        mov ax, [byte bp-2]             ; ax = "temp" (high word)
        xor dx, dx                      ; dx = 0x0000
        div bx                          ; "temp" / number of heads (part 1/2)
        test ax, ax                     ; test zero
        jnz short .Err                  ; (jump if unsupported by CHS)
        pop ax                          ; ax = "temp" (low word)
        div bx                          ; "temp" / number of heads (part 2/2)
        mov sp, bp                      ; restore stack pointer

        test ax, 0xFC00                 ; 0b00000011_11111111
                                        ;         ^^ ^^^^^^^^ cylinder
        jnz short .Err                  ; (jump if unsupported by CHS)
        lea cx, [byte si+0x01]          ; cl = sector number
        ror ah, 1                       ; right rotate
        ror ah, 1                       ; right rotate
        or cl, ah                       ; cl [bits 7-6] = cylinder high bits
        mov ch, al                      ; ch = cylinder number
        mov ah, dl                      ; al = head number

        pop dx                          ; restore register dx
        mov dh, ah                      ; dl = drive number, dh = head number
        mov bx, 0x7C00                  ; es:bx = data buffer
        stc                             ; set carry flag
        mov ax, 0x0201                  ; ah = 0x02, read one sector
        int 0x13                        ; bios call
        sti                             ; enable interrupts
        ret
.Err:   mov si, Msg2                    ; geometry error
        jmp short Error

ResetDisk:
        push ax                         ; save register ax
        push dx                         ; save register dx
        push si                         ; save register si
        mov ah, 0x00                    ; ah = 0x00, reset disk system
        int 0x13                        ; bios call
        pop si                          ; restore register si
        pop dx                          ; restore register dx
        pop ax                          ; restore register ax
        ret

ReadLba:
        push word [byte si+0x08]        ; push lba (low word)
        push word [byte si+0x0A]        ; push lba (high word)
        mov ax, 0x0800                  ; ax = 0x0800
        mov cx, 0x0020                  ; cx = 0x0020 (buffer size + extra)
        mov di, 0x0800                  ; di = 0x0800 (buffer address)
        cld                             ; clear direction flag (critical)
        rep stosb                       ; clear buffer and some extra bytes
        mov si, ax                      ; si = 0x0800
        mov byte [byte si+0x00], 0x10   ; size
        mov byte [byte si+0x02], 0x01   ; number of blocks
        mov byte [byte si+0x05], 0x7C   ; transfer buffer (0x7C00)
        pop word [byte si+0x0A]         ; lba (high word)
        pop word [byte si+0x08]         ; lba (low word)
        and dh, 0x00                    ; dl = drive number, dh = 0x00
        stc                             ; set carry flag
        mov ah, 0x42                    ; ah = 0x42 (al = 0x00), extended read
        int 0x13                        ; bios call
        sti                             ; enable interrupts
        ret

times 368 - ($ - $$) db 0
Error:
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


Msg1:   db '? ACTIVE PARTITION', 13, 0
Msg2:   db '! GEOMETRY ERROR', 13, 0
Msg3:   db '! DISK ERROR', 7, 13, 0     ; notice bell character

times 440 - ($ - $$) db 0
        db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
PartitionTable:

times 510 - ($ - $$) db 0
PartitionTableEnd:
        db 0x55, 0xAA
