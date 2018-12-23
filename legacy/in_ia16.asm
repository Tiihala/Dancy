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
;; legacy/in_ia16.asm
;;      Dancy operating system for legacy hardware
;;

%include "legacy/inc/bsyscall.asm"

        BITS 16
        CPU 8086
        ORG 0x0000

times 32 - ($ - $$) db 0
in_ia16:
        mov cx, 0x0000                  ; length of string (0 is "unlimited")
        mov bx, hello_world             ; address of string
        mov ah, b_output_string         ; syscall number
        int 0x20                        ; syscall

.stop:  mov ah, b_get_keycode           ; syscall number
        int 0x20                        ; syscall
        mov ah, b_pause                 ; syscall number
        int 0x20                        ; syscall
        jmp short .stop

hello_world:
db 13, 10, 13, 10
db 'Welcome to the legacy edition of Dancy Operating System!'
db 13, 10, 13, 10
db 'This version is not implemented at this point but it will be '    , 13, 10
db 'like a client for other Dancy sessions that are run on modern '   , 13, 10
db 'hardware. The data is tranferred via COM1 and COM2 ports.'        , 13, 10
db 0
