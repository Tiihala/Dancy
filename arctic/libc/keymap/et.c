/*
 * Copyright (c) 2024 Antti Tiihala
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
 * libc/keymap/et.c
 *      The Estonian keyboard
 */

#include <__dancy/keys.h>

#define Dc0(i) (i)
#define Dc1(i) (__DANCY_KEYBIT_CAPS | (i))
#define Dc2(i) (__DANCY_KEYBIT_NPAD | (i))
#define Dc3(i) (__DANCY_KEYBIT_DEAD | (i))

const struct __dancy_keymap __dancy_keymap_et = {
/*
 * int map[16][128];
 */
{
	/*
	 * __DANCY_KEYMAP_NORMAL
	 */
	{
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc1('a'     ), Dc1('b'     ), Dc1('c'     ), Dc1('d'     ),
		Dc1('e'     ), Dc1('f'     ), Dc1('g'     ), Dc1('h'     ),
		Dc1('i'     ), Dc1('j'     ), Dc1('k'     ), Dc1('l'     ),
		Dc1('m'     ), Dc1('n'     ), Dc1('o'     ), Dc1('p'     ),
		Dc1('q'     ), Dc1('r'     ), Dc1('s'     ), Dc1('t'     ),
		Dc1('u'     ), Dc1('v'     ), Dc1('w'     ), Dc1('x'     ),
		Dc1('y'     ), Dc1('z'     ), Dc0('1'     ), Dc0('2'     ),
		Dc0('3'     ), Dc0('4'     ), Dc0('5'     ), Dc0('6'     ),
		Dc0('7'     ), Dc0('8'     ), Dc0('9'     ), Dc0('0'     ),
		Dc0(0x00000A), Dc0(0x000000), Dc0(0x00007F), Dc0(0x000009),
		Dc0(0x000020), Dc0('+'     ), Dc3(0x0000B4), Dc1(0x0000FC),
		Dc1(0x0000F5), Dc0('\''    ), Dc0(0x000000), Dc1(0x0000F6),
		Dc1(0x0000E4), Dc3(0x0002C7), Dc0(','     ), Dc0('.'     ),
		Dc0('-'     ), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0('/'     ), Dc0('*'     ), Dc0('-'     ), Dc0('+'     ),
		Dc0(0x00000A), Dc2(0x000000), Dc2(0x000000), Dc2(0x000000),
		Dc2(0x000000), Dc2(0x000000), Dc2(0x000000), Dc2(0x000000),
		Dc2(0x000000), Dc2(0x000000), Dc2(0x000000), Dc2(0x000000),
		Dc0('<'     ), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
	},

	/*
	 * __DANCY_KEYMAP_SHIFT
	 */
	{
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc1('A'     ), Dc1('B'     ), Dc1('C'     ), Dc1('D'     ),
		Dc1('E'     ), Dc1('F'     ), Dc1('G'     ), Dc1('H'     ),
		Dc1('I'     ), Dc1('J'     ), Dc1('K'     ), Dc1('L'     ),
		Dc1('M'     ), Dc1('N'     ), Dc1('O'     ), Dc1('P'     ),
		Dc1('Q'     ), Dc1('R'     ), Dc1('S'     ), Dc1('T'     ),
		Dc1('U'     ), Dc1('V'     ), Dc1('W'     ), Dc1('X'     ),
		Dc1('Y'     ), Dc1('Z'     ), Dc0('!'     ), Dc0('\"'    ),
		Dc0('#'     ), Dc0(0x0000A4), Dc0('%'     ), Dc0('&'     ),
		Dc0('/'     ), Dc0('('     ), Dc0(')'     ), Dc0('='     ),
		Dc0(0x00000A), Dc0(0x000000), Dc0(0x00007F), Dc0(0x000009),
		Dc0(0x000020), Dc0('?'     ), Dc3('`'     ), Dc1(0x0000DC),
		Dc1(0x0000D5), Dc0('*'     ), Dc0(0x000000), Dc1(0x0000D6),
		Dc1(0x0000C4), Dc3('~'     ), Dc0(';'     ), Dc0(':'     ),
		Dc0('_'     ), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc2('1'     ), Dc2('2'     ), Dc2('3'     ),
		Dc2('4'     ), Dc2('5'     ), Dc2('6'     ), Dc2('7'     ),
		Dc2('8'     ), Dc2('9'     ), Dc2('0'     ), Dc2(','     ),
		Dc0('>'     ), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
	},

	/*
	 * __DANCY_KEYMAP_CTRL
	 */
	{
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0('A'-'@' ), Dc0('B'-'@' ), Dc0('C'-'@' ), Dc0('D'-'@' ),
		Dc0('E'-'@' ), Dc0('F'-'@' ), Dc0('G'-'@' ), Dc0('H'-'@' ),
		Dc0('I'-'@' ), Dc0('J'-'@' ), Dc0('K'-'@' ), Dc0('L'-'@' ),
		Dc0('M'-'@' ), Dc0('N'-'@' ), Dc0('O'-'@' ), Dc0('P'-'@' ),
		Dc0('Q'-'@' ), Dc0('R'-'@' ), Dc0('S'-'@' ), Dc0('T'-'@' ),
		Dc0('U'-'@' ), Dc0('V'-'@' ), Dc0('W'-'@' ), Dc0('X'-'@' ),
		Dc0('Y'-'@' ), Dc0('Z'-'@' ), Dc0(0x000000), Dc0(0x000000),
	},

	/*
	 * __DANCY_KEYMAP_SHIFT
	 * __DANCY_KEYMAP_CTRL
	 */
	{
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
	},

	/*
	 * __DANCY_KEYMAP_ALTGR
	 */
	{
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x0020AC), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x0000B5), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc1(0x000161), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc1(0x00017E), Dc0(0x000000), Dc0('@'     ),
		Dc0(0x0000A3), Dc0('$'     ), Dc0(0x0020AC), Dc0(0x000000),
		Dc0('{'     ), Dc0('['     ), Dc0(']'     ), Dc0('}'     ),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0('\\'    ), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x0000A7), Dc0(0x0000BD), Dc0(0x000000), Dc0(0x000000),
		Dc0('^'     ), Dc0(0x000000), Dc0('<'     ), Dc0('>'     ),
		Dc0('|'     ), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0('|'     ), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
	},

	/*
	 * __DANCY_KEYMAP_SHIFT
	 * __DANCY_KEYMAP_ALTGR
	 */
	{
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc1(0x000160), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc1(0x00017D), Dc0(0x000000), Dc0(0x000000),
	},

	/*
	 * __DANCY_KEYMAP_CTRL
	 * __DANCY_KEYMAP_ALTGR
	 */
	{
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0(0x000000), Dc0(0x000000), Dc0(0x000000),
		Dc0(0x000000), Dc0('\\'-'@'), Dc0(0x000000), Dc0(0x000000),
	},
},

/*
 * int diacritic[64][3];
 */
{
	{ Dc0(0x000000), Dc0(0x000000), Dc0(0x000000) },
},

/*
 * const char *name, *description;
 */
	"et", "Estonian"
};
