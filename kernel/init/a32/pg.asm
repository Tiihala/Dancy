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
;; init/a32/pg.asm
;;      Page translation and protection
;;

        bits 32

section .text

        global _pg_get_current
        global _pg_get_fault
        global _pg_switch

align 16
        ; void pg_get_current(phys_addr_t *addr)
_pg_get_current:
        mov ecx, [esp+4]                ; ecx = addr
        mov eax, cr3                    ; eax = control register cr3
        mov [ecx], eax                  ; *addr = cr3
        ret

align 16
        ; void pg_get_fault(phys_addr_t *addr)
_pg_get_fault:
        mov ecx, [esp+4]                ; ecx = addr
        mov eax, cr2                    ; eax = control register cr2
        mov [ecx], eax                  ; *addr = cr2
        ret

align 16
        ; void pg_switch(phys_addr_t addr)
_pg_switch:
        mov eax, [esp+4]                ; eax = addr
        mov cr3, eax                    ; move addr to control register cr3
        ret
