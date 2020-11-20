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
;; uefi/a64/font7x9.asm
;;      Raster fonts for the UEFI boot loader
;;

        bits 64

section .data

        global font7x9

align 16
font7x9:
        dq 0x0000000000000000           ; ASCII 0x20
        dq 0x0400102040810204           ; ASCII 0x21
        dq 0x0000000000000912           ; ASCII 0x22
        dq 0x1224FC91224FC912           ; ASCII 0x23
        dq 0x041C54A0E0A54704           ; ASCII 0x24
        dq 0x0244902081048920           ; ASCII 0x25
        dq 0x2E2245504184890C           ; ASCII 0x26
        dq 0x0000000000008204           ; ASCII 0x27
        dq 0x0808101020410208           ; ASCII 0x28
        dq 0x0410208102020404           ; ASCII 0x29
        dq 0x000088A3F2888000           ; ASCII 0x2A
        dq 0x00001021F0810000           ; ASCII 0x2B
        dq 0x8208100000000000           ; ASCII 0x2C
        dq 0x00000001F0000000           ; ASCII 0x2D
        dq 0x0400000000000000           ; ASCII 0x2E
        dq 0x0204102081040820           ; ASCII 0x2F
        dq 0x1E428D2A9628509E           ; ASCII 0x30
        dq 0x0810204081028608           ; ASCII 0x31
        dq 0x3F0208208208109E           ; ASCII 0x32
        dq 0x1E428101C408109E           ; ASCII 0x33
        dq 0x10204083F2450C10           ; ASCII 0x34
        dq 0x1E42810203E040BF           ; ASCII 0x35
        dq 0x1E428509F020509E           ; ASCII 0x36
        dq 0x020810408204103F           ; ASCII 0x37
        dq 0x1E428509E428509E           ; ASCII 0x38
        dq 0x1E428103E428509E           ; ASCII 0x39
        dq 0x0008100000810000           ; ASCII 0x3A
        dq 0x8208100000000204           ; ASCII 0x3B
        dq 0x1010101010410410           ; ASCII 0x3C
        dq 0x000000F803E00000           ; ASCII 0x3D
        dq 0x0104104101010101           ; ASCII 0x3E
        dq 0x040010204104109E           ; ASCII 0x3F
        dq 0x3E0275AB57A8509E           ; ASCII 0x40
        dq 0x2142850BF428509E           ; ASCII 0x41
        dq 0x1F428509F428509F           ; ASCII 0x42
        dq 0x1E4204081020509E           ; ASCII 0x43
        dq 0x1F42850A1428509F           ; ASCII 0x44
        dq 0x3F020409F02040BF           ; ASCII 0x45
        dq 0x01020409F02040BF           ; ASCII 0x46
        dq 0x1E42850BD020509E           ; ASCII 0x47
        dq 0x2142850BF42850A1           ; ASCII 0x48
        dq 0x1C1020408102041C           ; ASCII 0x49
        dq 0x0E22408102040810           ; ASCII 0x4A
        dq 0x2122242830A248A1           ; ASCII 0x4B
        dq 0x3F02040810204081           ; ASCII 0x4C
        dq 0x2142850A142B59A1           ; ASCII 0x4D
        dq 0x3162A54A94A951A3           ; ASCII 0x4E
        dq 0x1E42850A1428509E           ; ASCII 0x4F
        dq 0x01020409F428509F           ; ASCII 0x50
        dq 0x3E62A50A1428509E           ; ASCII 0x51
        dq 0x21424489F428509F           ; ASCII 0x52
        dq 0x1E428101E020509E           ; ASCII 0x53
        dq 0x081020408102047F           ; ASCII 0x54
        dq 0x1E42850A142850A1           ; ASCII 0x55
        dq 0x0C24850A142850A1           ; ASCII 0x56
        dq 0x125AB56A142850A1           ; ASCII 0x57
        dq 0x21428490C24850A1           ; ASCII 0x58
        dq 0x0810204081051141           ; ASCII 0x59
        dq 0x3F0410208104083F           ; ASCII 0x5A
        dq 0x0F0204081020408F           ; ASCII 0x5B
        dq 0x1020204040808101           ; ASCII 0x5C
        dq 0x1E2040810204081E           ; ASCII 0x5D
        dq 0x0000000000044504           ; ASCII 0x5E
        dq 0x1F00000000000000           ; ASCII 0x5F
        dq 0x0000000000020204           ; ASCII 0x60
        dq 0x2E2244F103C00000           ; ASCII 0x61
        dq 0x1F44891223C08103           ; ASCII 0x62
        dq 0x0E22040911C00000           ; ASCII 0x63
        dq 0x3E22448913C40818           ; ASCII 0x64
        dq 0x0E2204F911C00000           ; ASCII 0x65
        dq 0x0E08102043E10A08           ; ASCII 0x66
        dq 0x8E2240F112244F00           ; ASCII 0x67
        dq 0x1122448911E04081           ; ASCII 0x68
        dq 0x0E08102040C00200           ; ASCII 0x69
        dq 0x8612204081030008           ; ASCII 0x6A
        dq 0x13242830A2408180           ; ASCII 0x6B
        dq 0x0C08102040810204           ; ASCII 0x6C
        dq 0x112244A951400000           ; ASCII 0x6D
        dq 0x1122448911E00000           ; ASCII 0x6E
        dq 0x0E22448911C00000           ; ASCII 0x6F
        dq 0x81023C8912244780           ; ASCII 0x70
        dq 0xB82040F112244700           ; ASCII 0x71
        dq 0x0204081223C00000           ; ASCII 0x72
        dq 0x0E22603111C00000           ; ASCII 0x73
        dq 0x1C08102043E10204           ; ASCII 0x74
        dq 0x2E22448912200000           ; ASCII 0x75
        dq 0x0414448912200000           ; ASCII 0x76
        dq 0x0A2A548912200000           ; ASCII 0x77
        dq 0x11222820A2200000           ; ASCII 0x78
        dq 0x8E2240F112244880           ; ASCII 0x79
        dq 0x1F02086103E00000           ; ASCII 0x7A
        dq 0x0C0408101040810C           ; ASCII 0x7B
        dq 0x0408102000810204           ; ASCII 0x7C
        dq 0x0610204101020406           ; ASCII 0x7D
        dq 0x0000000003298000           ; ASCII 0x7E
        dq 0x3F46954B34AA58BF           ; ASCII 0x7F
