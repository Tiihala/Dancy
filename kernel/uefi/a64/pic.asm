;;
;; Copyright (c) 2020 Antti Tiihala
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
;; uefi/a64/pic.asm
;;      Initialize 8259 Programmable Interrupt Controller (PIC)
;;

        bits 64

section .text

        extern cpu_rdtsc
        extern cpu_rdtsc_delay
        extern cpu_rdtsc_diff
        global pic_delay
        global pic_delay_beg
        global pic_delay_end
        global pic_init

align 16
        ; void pic_delay(void)
pic_delay:
        push rbp                        ; save register rbp
        mov rbp, rsp                    ; rbp = current stack pointer

        mov rcx, [delay_low]            ; rcx = value of "delay_low"
        mov rdx, [delay_high]           ; rdx = value of "delay_high"
        call cpu_rdtsc_delay            ; delay

        pop rbp                         ; restore register rbp
        ret

align 16
        ; void pic_delay_beg(void)
pic_delay_beg:
        push rbp                        ; save register rbp
        mov rbp, rsp                    ; rbp = current stack pointer

        mov rcx, delay_low              ; rcx = address of "delay_low"
        mov rdx, delay_high             ; rdx = address of "delay_high"
        call cpu_rdtsc                  ; initial value

        pop rbp                         ; restore register rbp
        ret

align 16
        ; void pic_delay_end(void)
pic_delay_end:
        push rbp                        ; save register rbp
        mov rbp, rsp                    ; rbp = current stack pointer

        mov rcx, delay_low              ; rcx = address of "delay_low"
        mov rdx, delay_high             ; rdx = address of "delay_high"
        call cpu_rdtsc_diff             ; diff value

        pop rbp                         ; restore register rbp
        ret

align 16
        ; void pic_init(void)
pic_init:
        push rbp                        ; save register rbp
        mov rbp, rsp                    ; rbp = current stack pointer

        mov al, 0xFF                    ; disable IRQs (PIC 2)
        out 0xA1, al
        call pic_delay

        out 0x21, al                    ; disable IRQs (PIC 1)
        call pic_delay

        cli                             ; disable interrupts

        mov al, 0x11                    ; start the initialization (PIC 1)
        out 0x20, al
        call pic_delay

        mov al, 0x20                    ; vector start from 0x20 (PIC 1)
        out 0x21, al
        call pic_delay

        mov al, 0x04                    ; settings (PIC 1)
        out 0x21, al
        call pic_delay

        mov al, 0x01                    ; settings (PIC 1)
        out 0x21, al
        call pic_delay

        mov al, 0x11                    ; start the initialization (PIC 2)
        out 0xA0, al
        call pic_delay

        mov al, 0x28                    ; vector start from 0x28 (PIC 2)
        out 0xA1, al
        call pic_delay

        mov al, 0x02                    ; settings (PIC 2)
        out 0xA1, al
        call pic_delay

        mov al, 0x01                    ; settings (PIC 2)
        out 0xA1, al
        call pic_delay

        mov al, 0xFF                    ; set the mask bits (PIC 2)
        out 0xA1, al
        call pic_delay

        mov al, 0xFB                    ; set the mask bits (PIC 1)
        out 0x21, al

        pop rbp                         ; restore register rbp
        ret


section .data

align 4
delay_low:
        dd 0
delay_high:
        dd 0
