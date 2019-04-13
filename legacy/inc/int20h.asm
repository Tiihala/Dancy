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
;; legacy/inc/int20h.asm
;;      Dancy operating system for legacy hardware
;;

%define b_output_string         (0xA0)
%define b_output_string_hl      (0xA1)
%define b_output_control        (0xA2)
%define b_get_keycode           (0xA3)
%define b_get_byte_com1         (0xA4)
%define b_put_byte_com1         (0xA5)
%define b_get_byte_com2         (0xA6)
%define b_put_byte_com2         (0xA7)
%define b_get_parameter         (0xA8)
%define b_get_structure         (0xA9)
%define b_set_read_buffer       (0xAA)
%define b_read_blocks           (0xAB)
%define b_set_write_buffer      (0xAC)
%define b_write_blocks          (0xAD)
%define b_pause                 (0xAE)
%define b_exit                  (0xAF)
