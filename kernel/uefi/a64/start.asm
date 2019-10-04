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
;; uefi/a64/start.asm
;;      Boot Loader (UEFI)
;;

        bits 64

section .text

        extern uefi_main
        global start

align 16
        ; void start(uint64_t, uint64_t, uint64_t)
start:
        push rbp                        ; save register rbp
        mov rbp, rsp                    ; rbp = current stack pointer
        lea r9, [rsp+8]                 ; r9 = "OriginalRsp"
        push r9                         ; sub rsp, 8
        push r8                         ; sub rsp, 8
        push rdx                        ; sub rsp, 8
        push rcx                        ; sub rsp, 8
        call uefi_main                  ; call uefi_main
        add rsp, 32                     ; free shadow space
        pop rbp                         ; restore register rbp
        ret
