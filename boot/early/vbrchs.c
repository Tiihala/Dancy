/*
 * Copyright (c) 2015, 2018 Antti Tiihala
 *
 * This work is licensed under the Creative Commons Attribution-
 * NoDerivatives 4.0 International License. To view a copy of this
 * license, visit http://creativecommons.org/licenses/by-nd/4.0/
 * or send a letter to Creative Commons, PO Box 1866, Mountain View,
 * CA 94042, USA.
 *
 * fat/vbrchs.c
 *      Volume Boot Record for FAT12/FAT16 (CHS)
 */

#include <dancy.h>

/*
 * VERSION 1.0 (CRC-32C: 0xD3392602)
 */
const unsigned char vbrchs_bin[512] = {
	DANCY_BLOB(0xEB3C,0x9000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000),
	DANCY_BLOB(0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x33F6),
	DANCY_BLOB(0x8EC6,0x8EDE,0xB90F,0x008C,0xC8BD,0x007C,0xEB0A,0xEBF0),
	DANCY_BLOB(0x8B7E,0xFE85,0xF9B1,0x04C3,0xFA8E,0xD68B,0xE5E8,0xF0FF),
	DANCY_BLOB(0x75DE,0xD3EF,0x03C7,0x3DC6,0x0775,0xD5EA,0x707C,0x0000),
	DANCY_BLOB(0xFB88,0x5624,0x55C7,0x46FE,0xA07D,0x33C9,0x33FF,0x0655),
	DANCY_BLOB(0xB600,0xB408,0xCD13,0xFBFC,0x5D07,0x7217,0x81E1,0x3F00),
	DANCY_BLOB(0x7411,0x8AD6,0x81E2,0xFF00,0x7409,0x83C2,0x0189,0x4E18),
	DANCY_BLOB(0x8956,0x1A33,0xDB8B,0x4613,0x2D01,0x00BA,0x0000,0x722D),
	DANCY_BLOB(0x0346,0x1C13,0x561E,0x7225,0xBE80,0x7DBF,0x0000,0x9CFA),
	DANCY_BLOB(0x8977,0x0089,0x7F02,0x9D90,0xF776,0x1833,0xD2F7,0x761A),
	DANCY_BLOB(0x0AE6,0x0A66,0x0B75,0x0680,0x7E0C,0x0273,0x03E9,0xA000),
	DANCY_BLOB(0xB105,0xA111,0x7CD3,0xE033,0xD2F7,0x760B,0x85C0,0x744A),
	DANCY_BLOB(0x85D2,0x7546,0x508A,0x4610,0xB400,0xF766,0x1603,0x460E),
	DANCY_BLOB(0x5001,0x46FC,0x5033,0xD2BD,0x4802,0xE88B,0x00BD,0x4802),
	DANCY_BLOB(0xA10B,0x7CBF,0x0080,0x03C7,0xB90B,0x00BE,0xF07D,0x3805),
	DANCY_BLOB(0x745E,0x57F3,0xA65F,0x7414,0x83C7,0x203B,0xF872,0xE958),
	DANCY_BLOB(0x0501,0x0050,0x3B06,0xFC7B,0x72CB,0xEB44,0xF645,0x0B18),
	DANCY_BLOB(0x753E,0x8B45,0x1A2D,0x0200,0x7236,0x020E,0x0D7C,0x7430),
	DANCY_BLOB(0xF7E1,0x0306,0xFC7B,0x817D,0x1C00,0x0275,0x230B,0x551E),
	DANCY_BLOB(0x751E,0xE833,0x00BC,0xFE7B,0x33ED,0xB900,0x01BE,0x0080),
	DANCY_BLOB(0xAD03,0xE849,0x75FA,0x5B3B,0xEC75,0x05EA,0x0080,0x0000),
	DANCY_BLOB(0xB800,0x0533,0xDB53,0xCD10,0x5B2E,0x8A87,0xDE7D,0xB40E),
	DANCY_BLOB(0x4384,0xC075,0xF0F4,0xEBFD,0x0306,0x1C7C,0x1316,0x1E7C),
	DANCY_BLOB(0x5052,0xF736,0x187C,0x8BDA,0xB108,0x33D2,0xF736,0x1A7C),
	DANCY_BLOB(0xD3E0,0xD3E2,0x8D4F,0x018A,0xECBB,0x0080,0x8A16,0x247C),
	DANCY_BLOB(0xD1ED,0x5574,0xBB72,0x0DF9,0xB801,0x02CD,0x13FB,0x7209),
	DANCY_BLOB(0x5D5A,0x58C3,0xB400,0xCD13,0xFB5D,0x5A58,0xEBC2,0x0D0A),
	DANCY_BLOB(0x2120,0x424F,0x4F54,0x2045,0x5252,0x4F52,0x0707,0x0D00),
	DANCY_BLOB(0x4C4F,0x4144,0x4552,0x2020,0x3531,0x3200,0x0000,0x55AA)
};
