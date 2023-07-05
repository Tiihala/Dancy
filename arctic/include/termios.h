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
 * termios.h
 *      Definitions used by the terminal I/O interfaces
 */

#ifndef __DANCY_TERMIOS_H
#define __DANCY_TERMIOS_H

#include <__dancy/core.h>
#include <__dancy/termios.h>

__Dancy_Header_Begin

#ifndef __DANCY_TYPEDEF_PID_T
#define __DANCY_TYPEDEF_PID_T
typedef __dancy_pid_t pid_t;
#endif

typedef __dancy_cc_t cc_t;
typedef __dancy_speed_t speed_t;
typedef __dancy_tcflag_t tcflag_t;

#define NCCS        __DANCY_TERMIOS_NCCS

#define VEOF        __DANCY_TERMIOS_VEOF
#define VEOL        __DANCY_TERMIOS_VEOL
#define VERASE      __DANCY_TERMIOS_VERASE
#define VINTR       __DANCY_TERMIOS_VINTR
#define VKILL       __DANCY_TERMIOS_VKILL
#define VMIN        __DANCY_TERMIOS_VMIN
#define VQUIT       __DANCY_TERMIOS_VQUIT
#define VSTART      __DANCY_TERMIOS_VSTART
#define VSTOP       __DANCY_TERMIOS_VSTOP
#define VSUSP       __DANCY_TERMIOS_VSUSP
#define VTIME       __DANCY_TERMIOS_VTIME

#define BRKINT      __DANCY_TERMIOS_BRKINT
#define ICRNL       __DANCY_TERMIOS_ICRNL
#define IGNBRK      __DANCY_TERMIOS_IGNBRK
#define IGNCR       __DANCY_TERMIOS_IGNCR
#define IGNPAR      __DANCY_TERMIOS_IGNPAR
#define INLCR       __DANCY_TERMIOS_INLCR
#define INPCK       __DANCY_TERMIOS_INPCK
#define ISTRIP      __DANCY_TERMIOS_ISTRIP
#define IUCLC       __DANCY_TERMIOS_IUCLC
#define IXANY       __DANCY_TERMIOS_IXANY
#define IXOFF       __DANCY_TERMIOS_IXOFF
#define IXON        __DANCY_TERMIOS_IXON
#define PARMRK      __DANCY_TERMIOS_PARMRK

#define BS0         __DANCY_TERMIOS_BS0
#define BS1         __DANCY_TERMIOS_BS1
#define BSDLY       __DANCY_TERMIOS_BSDLY
#define CR0         __DANCY_TERMIOS_CR0
#define CR1         __DANCY_TERMIOS_CR1
#define CR2         __DANCY_TERMIOS_CR2
#define CR3         __DANCY_TERMIOS_CR3
#define CRDLY       __DANCY_TERMIOS_CRDLY
#define FF0         __DANCY_TERMIOS_FF0
#define FF1         __DANCY_TERMIOS_FF1
#define FFDLY       __DANCY_TERMIOS_FFDLY
#define NL0         __DANCY_TERMIOS_NL0
#define NL1         __DANCY_TERMIOS_NL1
#define NLDLY       __DANCY_TERMIOS_NLDLY
#define OCRNL       __DANCY_TERMIOS_OCRNL
#define OFDEL       __DANCY_TERMIOS_OFDEL
#define OFILL       __DANCY_TERMIOS_OFILL
#define OLCUC       __DANCY_TERMIOS_OLCUC
#define ONLCR       __DANCY_TERMIOS_ONLCR
#define ONLRET      __DANCY_TERMIOS_ONLRET
#define ONOCR       __DANCY_TERMIOS_ONOCR
#define OPOST       __DANCY_TERMIOS_OPOST
#define TAB0        __DANCY_TERMIOS_TAB0
#define TAB1        __DANCY_TERMIOS_TAB1
#define TAB2        __DANCY_TERMIOS_TAB2
#define TAB3        __DANCY_TERMIOS_TAB3
#define TABDLY      __DANCY_TERMIOS_TABDLY
#define VT0         __DANCY_TERMIOS_VT0
#define VT1         __DANCY_TERMIOS_VT1
#define VTDLY       __DANCY_TERMIOS_VTDLY

#define B0          __DANCY_TERMIOS_B0
#define B50         __DANCY_TERMIOS_B50
#define B75         __DANCY_TERMIOS_B75
#define B110        __DANCY_TERMIOS_B110
#define B134        __DANCY_TERMIOS_B134
#define B150        __DANCY_TERMIOS_B150
#define B200        __DANCY_TERMIOS_B200
#define B300        __DANCY_TERMIOS_B300
#define B600        __DANCY_TERMIOS_B600
#define B1200       __DANCY_TERMIOS_B1200
#define B1800       __DANCY_TERMIOS_B1800
#define B2400       __DANCY_TERMIOS_B2400
#define B4800       __DANCY_TERMIOS_B4800
#define B9600       __DANCY_TERMIOS_B9600
#define B19200      __DANCY_TERMIOS_B19200
#define B38400      __DANCY_TERMIOS_B38400
#define B57600      __DANCY_TERMIOS_B57600
#define B115200     __DANCY_TERMIOS_B115200
#define B230400     __DANCY_TERMIOS_B230400
#define B460800     __DANCY_TERMIOS_B460800
#define B500000     __DANCY_TERMIOS_B500000
#define B576000     __DANCY_TERMIOS_B576000
#define B921600     __DANCY_TERMIOS_B921600
#define B1000000    __DANCY_TERMIOS_B1000000
#define B1152000    __DANCY_TERMIOS_B1152000
#define B1500000    __DANCY_TERMIOS_B1500000
#define B2000000    __DANCY_TERMIOS_B2000000
#define B2500000    __DANCY_TERMIOS_B2500000
#define B3000000    __DANCY_TERMIOS_B3000000
#define B3500000    __DANCY_TERMIOS_B3500000
#define B4000000    __DANCY_TERMIOS_B4000000

#define CLOCAL      __DANCY_TERMIOS_CLOCAL
#define CREAD       __DANCY_TERMIOS_CREAD
#define CS5         __DANCY_TERMIOS_CS5
#define CS6         __DANCY_TERMIOS_CS6
#define CS7         __DANCY_TERMIOS_CS7
#define CS8         __DANCY_TERMIOS_CS8
#define CSIZE       __DANCY_TERMIOS_CSIZE
#define CSTOPB      __DANCY_TERMIOS_CSTOPB
#define HUPCL       __DANCY_TERMIOS_HUPCL
#define PARENB      __DANCY_TERMIOS_PARENB
#define PARODD      __DANCY_TERMIOS_PARODD

#define ECHO        __DANCY_TERMIOS_ECHO
#define ECHOE       __DANCY_TERMIOS_ECHOE
#define ECHOK       __DANCY_TERMIOS_ECHOK
#define ECHONL      __DANCY_TERMIOS_ECHONL
#define ICANON      __DANCY_TERMIOS_ICANON
#define IEXTEN      __DANCY_TERMIOS_IEXTEN
#define ISIG        __DANCY_TERMIOS_ISIG
#define NOFLSH      __DANCY_TERMIOS_NOFLSH
#define TOSTOP      __DANCY_TERMIOS_TOSTOP
#define XCASE       __DANCY_TERMIOS_XCASE

#define TCSADRAIN   __DANCY_TERMIOS_TCSADRAIN
#define TCSAFLUSH   __DANCY_TERMIOS_TCSAFLUSH
#define TCSANOW     __DANCY_TERMIOS_TCSANOW

#define TCIFLUSH    __DANCY_TERMIOS_TCIFLUSH
#define TCIOFLUSH   __DANCY_TERMIOS_TCIOFLUSH
#define TCOFLUSH    __DANCY_TERMIOS_TCOFLUSH

#define TCIOFF      __DANCY_TERMIOS_TCIOFF
#define TCION       __DANCY_TERMIOS_TCION
#define TCOOFF      __DANCY_TERMIOS_TCOOFF
#define TCOON       __DANCY_TERMIOS_TCOON

struct termios {
	tcflag_t c_iflag;
	tcflag_t c_oflag;
	tcflag_t c_cflag;
	tcflag_t c_lflag;
	cc_t c_cc[NCCS];
};

speed_t cfgetispeed(const struct termios *termios_p);
speed_t cfgetospeed(const struct termios *termios_p);

int cfsetispeed(struct termios *termios_p, speed_t speed);
int cfsetospeed(struct termios *termios_p, speed_t speed);

int tcdrain(int fd);
int tcflow(int fd, int action);
int tcflush(int fd, int queue_selector);
int tcsendbreak(int fd, int duration);

pid_t tcgetsid(int fd);

int tcgetattr(int fd, struct termios *termios_p);
int tcsetattr(int fd, int optional_actions, const struct termios *termios_p);

__Dancy_Header_End

#endif
