/*
 * Copyright (c) 2015, 2018 Antti Tiihala
 *
 * This work is licensed under the Creative Commons Attribution-
 * NoDerivatives 4.0 International License. To view a copy of this
 * license, visit http://creativecommons.org/licenses/by-nd/4.0/
 * or send a letter to Creative Commons, PO Box 1866, Mountain View,
 * CA 94042, USA.
 *
 * fat/ldr512.c
 *      Second Stage Loader for FAT12/FAT16 (LOADER.512)
 */

#include <dancy.h>

/*
 * VERSION 1.0 (CRC-32C: 0x03AC2A3A)
 */
const unsigned char ldr512_bin[512] = {
	DANCY_BLOB(0xB900,0x02EB,0x02EB,0xF9E8,0x0600,0x81FE,0x0A80,0xEB08),
	DANCY_BLOB(0x558B,0xEC8B,0x7602,0x5DC3,0x75E6,0xBE00,0x80BF,0x007E),
	DANCY_BLOB(0x3BE5,0x75DC,0x81FD,0x007C,0x75D6,0x8CC8,0x061E,0x5AFC),
	DANCY_BLOB(0x0BC2,0x5A0B,0xC275,0xC9F3,0xA4EB,0x00EA,0x407E,0x0000),
	DANCY_BLOB(0x53F6,0x060C,0x7CFE,0x746F,0x3B4E,0x1374,0x6A0B,0x5622),
	DANCY_BLOB(0x7565,0xB105,0x8B46,0x11D3,0xE0F7,0x760B,0x85C0,0x7457),
	DANCY_BLOB(0x85D2,0x7553,0x508A,0x4610,0xB400,0xF766,0x1603,0x460E),
	DANCY_BLOB(0x7245,0x5001,0x06FC,0x7B50,0x33D2,0xE863,0x01BA,0xA583),
	DANCY_BLOB(0xA10B,0x7CBF,0x0080,0x03C7,0xB90B,0x00BE,0xF47F,0x3805),
	DANCY_BLOB(0x7425,0x57F3,0xA65F,0x7414,0x83C7,0x203B,0xF872,0xE958),
	DANCY_BLOB(0x0501,0x0050,0x3B06,0xFC7B,0x72CE,0xEB0B,0xF645,0x0B18),
	DANCY_BLOB(0x7505,0x0B4D,0x1A75,0x03E9,0x0601,0x5181,0x7D1C,0x0080),
	DANCY_BLOB(0x754C,0x817D,0x1E00,0x0075,0x45A1,0x137C,0x2B06,0xFC7B),
	DANCY_BLOB(0x763C,0x33D2,0x5E52,0x33C9,0x0A0E,0x0D7C,0x7430,0xF7F1),
	DANCY_BLOB(0x3DF5,0x0FBB,0xF67B,0xC707,0x107F,0x7204,0xC707,0x187F),
	DANCY_BLOB(0x33D2,0x52B8,0x0080,0x52F7,0x360B,0x7C5A,0xF7F1,0xD1E0),
	DANCY_BLOB(0x5F2B,0xF857,0x83C7,0x0279,0x0589,0x75FE,0xFF27,0xEB7E),
	DANCY_BLOB(0xB803,0x00B9,0xFF0F,0xEB06,0xB804,0x00B9,0xFFFF,0xF7E6),
	DANCY_BLOB(0xD1EA,0xD1D8,0x519C,0xE867,0x0088,0x1DE8,0x6200,0xB104),
	DANCY_BLOB(0x885D,0x018B,0x359D,0x7302,0xD3EE,0x5923,0xF1B9,0x0200),
	DANCY_BLOB(0x8935,0x03F9,0xBBF6,0x7B75,0xC35D,0x8B46,0x002D,0x0200),
	DANCY_BLOB(0x8A0E,0x0D7C,0xF7E1,0x0306,0xFC7B,0x5051,0x55BA,0x0000),
	DANCY_BLOB(0x06E8,0x7C00,0xB104,0xA10B,0x7CD3,0xE85B,0x03C3,0x8EC0),
	DANCY_BLOB(0x5D59,0x5840,0xE2E4,0x83C5,0x0275,0xCFB5,0x40BE,0x0080),
	DANCY_BLOB(0xAD03,0xE8E2,0xFBBC,0x007C,0x8EC1,0x3BEC,0x7472,0xEB30),
	DANCY_BLOB(0x5052,0x5780,0xFE01,0x7728,0xF736,0x0B7C,0x0306,0x0E7C),
	DANCY_BLOB(0x3B06,0xF87B,0x5274,0x09A3,0xF87B,0xBA00,0x00E8,0x3000),
	DANCY_BLOB(0x5E33,0xDB5F,0x5A58,0x0501,0x0013,0xD38A,0x9C00,0x80C3),
	DANCY_BLOB(0xB40F,0xCD10,0xB307,0x33F6,0x2E8A,0x84F0,0x7F83,0xC601),
	DANCY_BLOB(0x5356,0xB40E,0xCD10,0x5E5B,0x83FE,0x1072,0xEBF4,0xEBFD),
	DANCY_BLOB(0x0306,0x1C7C,0x1316,0x1E7C,0xBD48,0x02FF,0x16FE,0x7BC3),
	DANCY_BLOB(0x0D0A,0x2120,0x4C4F,0x4144,0x4552,0x2020,0x4154,0x200D)
};
