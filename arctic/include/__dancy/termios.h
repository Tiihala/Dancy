/*
 * Copyright (c) 2023 Antti Tiihala
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * include/__dancy/termios.h
 *      The Arctic Dancy Header
 */

#ifndef __DANCY_INTERNAL_TERMIOS_H
#define __DANCY_INTERNAL_TERMIOS_H

#include <__dancy/core.h>

__Dancy_Header_Begin

typedef unsigned char __dancy_cc_t;
typedef unsigned int __dancy_speed_t;
typedef unsigned int __dancy_tcflag_t;

#define __DANCY_TERMIOS_NCCS        (32)

#define __DANCY_TERMIOS_VEOF        (4)
#define __DANCY_TERMIOS_VEOL        (11)
#define __DANCY_TERMIOS_VERASE      (2)
#define __DANCY_TERMIOS_VINTR       (0)
#define __DANCY_TERMIOS_VKILL       (3)
#define __DANCY_TERMIOS_VMIN        (6)
#define __DANCY_TERMIOS_VQUIT       (1)
#define __DANCY_TERMIOS_VSTART      (8)
#define __DANCY_TERMIOS_VSTOP       (9)
#define __DANCY_TERMIOS_VSUSP       (10)
#define __DANCY_TERMIOS_VTIME       (5)

#define __DANCY_TERMIOS_BRKINT      (0x0002)
#define __DANCY_TERMIOS_ICRNL       (0x0100)
#define __DANCY_TERMIOS_IGNBRK      (0x0001)
#define __DANCY_TERMIOS_IGNCR       (0x0080)
#define __DANCY_TERMIOS_IGNPAR      (0x0004)
#define __DANCY_TERMIOS_INLCR       (0x0040)
#define __DANCY_TERMIOS_INPCK       (0x0010)
#define __DANCY_TERMIOS_ISTRIP      (0x0020)
#define __DANCY_TERMIOS_IUCLC       (0x0200)
#define __DANCY_TERMIOS_IXANY       (0x0800)
#define __DANCY_TERMIOS_IXOFF       (0x1000)
#define __DANCY_TERMIOS_IXON        (0x0400)
#define __DANCY_TERMIOS_PARMRK      (0x0008)

#define __DANCY_TERMIOS_BS0         (0x0000)
#define __DANCY_TERMIOS_BS1         (0x2000)
#define __DANCY_TERMIOS_BSDLY       (0x2000)
#define __DANCY_TERMIOS_CR0         (0x0000)
#define __DANCY_TERMIOS_CR1         (0x0200)
#define __DANCY_TERMIOS_CR2         (0x0400)
#define __DANCY_TERMIOS_CR3         (0x0600)
#define __DANCY_TERMIOS_CRDLY       (0x0600)
#define __DANCY_TERMIOS_FF0         (0x0000)
#define __DANCY_TERMIOS_FF1         (0x8000)
#define __DANCY_TERMIOS_FFDLY       (0x8000)
#define __DANCY_TERMIOS_NL0         (0x0000)
#define __DANCY_TERMIOS_NL1         (0x0100)
#define __DANCY_TERMIOS_NLDLY       (0x0100)
#define __DANCY_TERMIOS_OCRNL       (0x0008)
#define __DANCY_TERMIOS_OFDEL       (0x0080)
#define __DANCY_TERMIOS_OFILL       (0x0040)
#define __DANCY_TERMIOS_OLCUC       (0x0002)
#define __DANCY_TERMIOS_ONLCR       (0x0004)
#define __DANCY_TERMIOS_ONLRET      (0x0020)
#define __DANCY_TERMIOS_ONOCR       (0x0010)
#define __DANCY_TERMIOS_OPOST       (0x0001)
#define __DANCY_TERMIOS_TAB0        (0x0000)
#define __DANCY_TERMIOS_TAB1        (0x0800)
#define __DANCY_TERMIOS_TAB2        (0x1000)
#define __DANCY_TERMIOS_TAB3        (0x1800)
#define __DANCY_TERMIOS_TABDLY      (0x1800)
#define __DANCY_TERMIOS_VT0         (0x0000)
#define __DANCY_TERMIOS_VT1         (0x4000)
#define __DANCY_TERMIOS_VTDLY       (0x4000)

#define __DANCY_TERMIOS_B0          (0)
#define __DANCY_TERMIOS_B50         (1)
#define __DANCY_TERMIOS_B75         (2)
#define __DANCY_TERMIOS_B110        (3)
#define __DANCY_TERMIOS_B134        (4)
#define __DANCY_TERMIOS_B150        (5)
#define __DANCY_TERMIOS_B200        (6)
#define __DANCY_TERMIOS_B300        (7)
#define __DANCY_TERMIOS_B600        (8)
#define __DANCY_TERMIOS_B1200       (9)
#define __DANCY_TERMIOS_B1800       (10)
#define __DANCY_TERMIOS_B2400       (11)
#define __DANCY_TERMIOS_B4800       (12)
#define __DANCY_TERMIOS_B9600       (13)
#define __DANCY_TERMIOS_B19200      (14)
#define __DANCY_TERMIOS_B38400      (15)
#define __DANCY_TERMIOS_B57600      (4097)
#define __DANCY_TERMIOS_B115200     (4098)
#define __DANCY_TERMIOS_B230400     (4099)
#define __DANCY_TERMIOS_B460800     (4100)
#define __DANCY_TERMIOS_B500000     (4101)
#define __DANCY_TERMIOS_B576000     (4102)
#define __DANCY_TERMIOS_B921600     (4103)
#define __DANCY_TERMIOS_B1000000    (4104)
#define __DANCY_TERMIOS_B1152000    (4105)
#define __DANCY_TERMIOS_B1500000    (4106)
#define __DANCY_TERMIOS_B2000000    (4107)
#define __DANCY_TERMIOS_B2500000    (4108)
#define __DANCY_TERMIOS_B3000000    (4109)
#define __DANCY_TERMIOS_B3500000    (4110)
#define __DANCY_TERMIOS_B4000000    (4111)

#define __DANCY_TERMIOS_CLOCAL      (0x0800)
#define __DANCY_TERMIOS_CREAD       (0x0080)
#define __DANCY_TERMIOS_CS5         (0x0000)
#define __DANCY_TERMIOS_CS6         (0x0010)
#define __DANCY_TERMIOS_CS7         (0x0020)
#define __DANCY_TERMIOS_CS8         (0x0030)
#define __DANCY_TERMIOS_CSIZE       (0x0030)
#define __DANCY_TERMIOS_CSTOPB      (0x0040)
#define __DANCY_TERMIOS_HUPCL       (0x0400)
#define __DANCY_TERMIOS_PARENB      (0x0100)
#define __DANCY_TERMIOS_PARODD      (0x0200)

#define __DANCY_TERMIOS_ECHO        (0x0008)
#define __DANCY_TERMIOS_ECHOE       (0x0010)
#define __DANCY_TERMIOS_ECHOK       (0x0020)
#define __DANCY_TERMIOS_ECHONL      (0x0040)
#define __DANCY_TERMIOS_ICANON      (0x0002)
#define __DANCY_TERMIOS_IEXTEN      (0x8000)
#define __DANCY_TERMIOS_ISIG        (0x0001)
#define __DANCY_TERMIOS_NOFLSH      (0x0080)
#define __DANCY_TERMIOS_TOSTOP      (0x0100)
#define __DANCY_TERMIOS_XCASE       (0x0004)

#define __DANCY_TERMIOS_TCSADRAIN   (1)
#define __DANCY_TERMIOS_TCSAFLUSH   (2)
#define __DANCY_TERMIOS_TCSANOW     (0)

#define __DANCY_TERMIOS_TCIFLUSH    (0)
#define __DANCY_TERMIOS_TCIOFLUSH   (2)
#define __DANCY_TERMIOS_TCOFLUSH    (1)

#define __DANCY_TERMIOS_TCIOFF      (2)
#define __DANCY_TERMIOS_TCION       (3)
#define __DANCY_TERMIOS_TCOOFF      (0)
#define __DANCY_TERMIOS_TCOON       (1)

struct __dancy_termios {
	__dancy_tcflag_t c_iflag;
	__dancy_tcflag_t c_oflag;
	__dancy_tcflag_t c_cflag;
	__dancy_tcflag_t c_lflag;
	__dancy_cc_t c_cc[__DANCY_TERMIOS_NCCS];
};

__Dancy_Header_End

#endif
