/*
 * Copyright (c) 2019 Antti Tiihala
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
 * uefi/key.c
 *      Simple Text Input Ex Protocol
 */

#include <uefi.h>

static EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *input;

void key_init(void)
{
	EFI_HANDLE_PROTOCOL HandleProtocol;
	EFI_STATUS s;

	HandleProtocol = gSystemTable->BootServices->HandleProtocol;

	u_log("Simple Text Input Ex Protocol\n");

	/*
	 * Get the EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL.
	 */
	{
		EFI_GUID guid = EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL_GUID;
		EFI_HANDLE handle = gSystemTable->ConsoleInHandle;

		s = HandleProtocol(handle, &guid, (void **)&input);

		if (s != EFI_SUCCESS) {
			u_log("\tNot found\n");
			input = NULL;
			return;
		}
		u_log("\tFound\n");
	}

	/*
	 * Set EFI_TOGGLE_STATE_VALID and EFI_NUM_LOCK_ACTIVE.
	 */
	{
		uint8_t state = (uint8_t)(0x80u | 0x02u);

		s = input->SetState(input, &state);

		if (s != EFI_SUCCESS)
			u_log("\tCould not set the state\n");
	}

	u_log("\n");
}

static const struct {
	unsigned shift;
	enum dancy_key key;
} table_code[128] = {
	{ 0, DANCY_KEY_NULL },          /* 0x00 */
	{ 0, DANCY_KEY_NULL },          /* 0x01 */
	{ 0, DANCY_KEY_NULL },          /* 0x02 */
	{ 0, DANCY_KEY_NULL },          /* 0x03 */
	{ 0, DANCY_KEY_NULL },          /* 0x04 */
	{ 0, DANCY_KEY_NULL },          /* 0x05 */
	{ 0, DANCY_KEY_NULL },          /* 0x06 */
	{ 0, DANCY_KEY_NULL },          /* 0x07 */
	{ 0, DANCY_KEY_BACKSLASH },     /* 0x08 */
	{ 0, DANCY_KEY_TAB },           /* 0x09 */
	{ 0, DANCY_KEY_NULL },          /* 0x0A */
	{ 0, DANCY_KEY_NULL },          /* 0x0B */
	{ 0, DANCY_KEY_NULL },          /* 0x0C */
	{ 0, DANCY_KEY_ENTER },         /* 0x0D */
	{ 0, DANCY_KEY_NULL },          /* 0x0E */
	{ 0, DANCY_KEY_NULL },          /* 0x0F */
	{ 0, DANCY_KEY_NULL },          /* 0x10 */
	{ 0, DANCY_KEY_NULL },          /* 0x11 */
	{ 0, DANCY_KEY_NULL },          /* 0x12 */
	{ 0, DANCY_KEY_NULL },          /* 0x13 */
	{ 0, DANCY_KEY_NULL },          /* 0x14 */
	{ 0, DANCY_KEY_NULL },          /* 0x15 */
	{ 0, DANCY_KEY_NULL },          /* 0x16 */
	{ 0, DANCY_KEY_NULL },          /* 0x17 */
	{ 0, DANCY_KEY_NULL },          /* 0x18 */
	{ 0, DANCY_KEY_NULL },          /* 0x19 */
	{ 0, DANCY_KEY_NULL },          /* 0x1A */
	{ 0, DANCY_KEY_NULL },          /* 0x1B */
	{ 0, DANCY_KEY_NULL },          /* 0x1C */
	{ 0, DANCY_KEY_NULL },          /* 0x1D */
	{ 0, DANCY_KEY_NULL },          /* 0x1E */
	{ 0, DANCY_KEY_NULL },          /* 0x1F */
	{ 0, DANCY_KEY_SPACE },         /* 0x20 */
	{ 3, DANCY_KEY_1 },             /* 0x21 */
	{ 3, DANCY_KEY_APOSTROPHE },    /* 0x22 */
	{ 3, DANCY_KEY_3 },             /* 0x23 */
	{ 3, DANCY_KEY_4 },             /* 0x24 */
	{ 3, DANCY_KEY_5 },             /* 0x25 */
	{ 3, DANCY_KEY_7 },             /* 0x26 */
	{ 0, DANCY_KEY_APOSTROPHE },    /* 0x27 */
	{ 3, DANCY_KEY_9 },             /* 0x28 */
	{ 3, DANCY_KEY_0 },             /* 0x29 */
	{ 3, DANCY_KEY_8 },             /* 0x2A */
	{ 3, DANCY_KEY_EQUALS },        /* 0x2B */
	{ 0, DANCY_KEY_COMMA },         /* 0x2C */
	{ 0, DANCY_KEY_UNDERSCORE },    /* 0x2D */
	{ 0, DANCY_KEY_PERIOD },        /* 0x2E */
	{ 0, DANCY_KEY_SLASH },         /* 0x2F */
	{ 0, DANCY_KEY_0 },             /* 0x30 */
	{ 0, DANCY_KEY_1 },             /* 0x31 */
	{ 0, DANCY_KEY_2 },             /* 0x32 */
	{ 0, DANCY_KEY_3 },             /* 0x33 */
	{ 0, DANCY_KEY_4 },             /* 0x34 */
	{ 0, DANCY_KEY_5 },             /* 0x35 */
	{ 0, DANCY_KEY_6 },             /* 0x36 */
	{ 0, DANCY_KEY_7 },             /* 0x37 */
	{ 0, DANCY_KEY_8 },             /* 0x38 */
	{ 0, DANCY_KEY_9 },             /* 0x39 */
	{ 3, DANCY_KEY_SEMICOLON },     /* 0x3A */
	{ 0, DANCY_KEY_SEMICOLON },     /* 0x3B */
	{ 3, DANCY_KEY_COMMA },         /* 0x3C */
	{ 0, DANCY_KEY_EQUALS },        /* 0x3D */
	{ 3, DANCY_KEY_PERIOD },        /* 0x3E */
	{ 3, DANCY_KEY_SLASH },         /* 0x3F */
	{ 3, DANCY_KEY_2 },             /* 0x40 */
	{ 2, DANCY_KEY_A },             /* 0x41 */
	{ 2, DANCY_KEY_B },             /* 0x42 */
	{ 2, DANCY_KEY_C },             /* 0x43 */
	{ 2, DANCY_KEY_D },             /* 0x44 */
	{ 2, DANCY_KEY_E },             /* 0x45 */
	{ 2, DANCY_KEY_F },             /* 0x46 */
	{ 2, DANCY_KEY_G },             /* 0x47 */
	{ 2, DANCY_KEY_H },             /* 0x48 */
	{ 2, DANCY_KEY_I },             /* 0x49 */
	{ 2, DANCY_KEY_J },             /* 0x4A */
	{ 2, DANCY_KEY_K },             /* 0x4B */
	{ 2, DANCY_KEY_L },             /* 0x4C */
	{ 2, DANCY_KEY_M },             /* 0x4D */
	{ 2, DANCY_KEY_N },             /* 0x4E */
	{ 2, DANCY_KEY_O },             /* 0x4F */
	{ 2, DANCY_KEY_P },             /* 0x50 */
	{ 2, DANCY_KEY_Q },             /* 0x51 */
	{ 2, DANCY_KEY_R },             /* 0x52 */
	{ 2, DANCY_KEY_S },             /* 0x53 */
	{ 2, DANCY_KEY_T },             /* 0x54 */
	{ 2, DANCY_KEY_U },             /* 0x55 */
	{ 2, DANCY_KEY_V },             /* 0x56 */
	{ 2, DANCY_KEY_W },             /* 0x57 */
	{ 2, DANCY_KEY_X },             /* 0x58 */
	{ 2, DANCY_KEY_Y },             /* 0x59 */
	{ 2, DANCY_KEY_Z },             /* 0x5A */
	{ 0, DANCY_KEY_LEFTBRACKET },   /* 0x5B */
	{ 0, DANCY_KEY_HASH },          /* 0x5C */
	{ 0, DANCY_KEY_RIGHTBRACKET },  /* 0x5D */
	{ 3, DANCY_KEY_6 },             /* 0x5E */
	{ 3, DANCY_KEY_UNDERSCORE },    /* 0x5F */
	{ 0, DANCY_KEY_GRAVE },         /* 0x60 */
	{ 1, DANCY_KEY_A },             /* 0x61 */
	{ 1, DANCY_KEY_B },             /* 0x62 */
	{ 1, DANCY_KEY_C },             /* 0x63 */
	{ 1, DANCY_KEY_D },             /* 0x64 */
	{ 1, DANCY_KEY_E },             /* 0x65 */
	{ 1, DANCY_KEY_F },             /* 0x66 */
	{ 1, DANCY_KEY_G },             /* 0x67 */
	{ 1, DANCY_KEY_H },             /* 0x68 */
	{ 1, DANCY_KEY_I },             /* 0x69 */
	{ 1, DANCY_KEY_J },             /* 0x6A */
	{ 1, DANCY_KEY_K },             /* 0x6B */
	{ 1, DANCY_KEY_L },             /* 0x6C */
	{ 1, DANCY_KEY_M },             /* 0x6D */
	{ 1, DANCY_KEY_N },             /* 0x6E */
	{ 1, DANCY_KEY_O },             /* 0x6F */
	{ 1, DANCY_KEY_P },             /* 0x70 */
	{ 1, DANCY_KEY_Q },             /* 0x71 */
	{ 1, DANCY_KEY_R },             /* 0x72 */
	{ 1, DANCY_KEY_S },             /* 0x73 */
	{ 1, DANCY_KEY_T },             /* 0x74 */
	{ 1, DANCY_KEY_U },             /* 0x75 */
	{ 1, DANCY_KEY_V },             /* 0x76 */
	{ 1, DANCY_KEY_W },             /* 0x77 */
	{ 1, DANCY_KEY_X },             /* 0x78 */
	{ 1, DANCY_KEY_Y },             /* 0x79 */
	{ 1, DANCY_KEY_Z },             /* 0x7A */
	{ 3, DANCY_KEY_LEFTBRACKET },   /* 0x7B */
	{ 3, DANCY_KEY_HASH },          /* 0x7C */
	{ 3, DANCY_KEY_RIGHTBRACKET },  /* 0x7D */
	{ 3, DANCY_KEY_GRAVE },         /* 0x7E */
	{ 0, DANCY_KEY_NULL }           /* 0x7F */
};

static const enum dancy_key table_scan[128] = {
	DANCY_KEY_NULL,                 /* 0x00 */
	DANCY_KEY_UPARROW,              /* 0x01 */
	DANCY_KEY_DOWNARROW,            /* 0x02 */
	DANCY_KEY_RIGHTARROW,           /* 0x03 */
	DANCY_KEY_LEFTARROW,            /* 0x04 */
	DANCY_KEY_HOME,                 /* 0x05 */
	DANCY_KEY_END,                  /* 0x06 */
	DANCY_KEY_INSERT,               /* 0x07 */
	DANCY_KEY_DELETE,               /* 0x08 */
	DANCY_KEY_PAGEUP,               /* 0x09 */
	DANCY_KEY_PAGEDOWN,             /* 0x0A */
	DANCY_KEY_F1,                   /* 0x0B */
	DANCY_KEY_F2,                   /* 0x0C */
	DANCY_KEY_F3,                   /* 0x0D */
	DANCY_KEY_F4,                   /* 0x0E */
	DANCY_KEY_F5,                   /* 0x0F */
	DANCY_KEY_F6,                   /* 0x10 */
	DANCY_KEY_F7,                   /* 0x11 */
	DANCY_KEY_F8,                   /* 0x12 */
	DANCY_KEY_F9,                   /* 0x13 */
	DANCY_KEY_F10,                  /* 0x14 */
	DANCY_KEY_F11,                  /* 0x15 */
	DANCY_KEY_F12,                  /* 0x16 */
	DANCY_KEY_ESCAPE,               /* 0x17 */
	DANCY_KEY_NULL,                 /* 0x18 */
	DANCY_KEY_NULL,                 /* 0x19 */
	DANCY_KEY_NULL,                 /* 0x1A */
	DANCY_KEY_NULL,                 /* 0x1B */
	DANCY_KEY_NULL,                 /* 0x1C */
	DANCY_KEY_NULL,                 /* 0x1D */
	DANCY_KEY_NULL,                 /* 0x1E */
	DANCY_KEY_NULL,                 /* 0x1F */
	DANCY_KEY_NULL,                 /* 0x20 */
	DANCY_KEY_NULL,                 /* 0x21 */
	DANCY_KEY_NULL,                 /* 0x22 */
	DANCY_KEY_NULL,                 /* 0x23 */
	DANCY_KEY_NULL,                 /* 0x24 */
	DANCY_KEY_NULL,                 /* 0x25 */
	DANCY_KEY_NULL,                 /* 0x26 */
	DANCY_KEY_NULL,                 /* 0x27 */
	DANCY_KEY_NULL,                 /* 0x28 */
	DANCY_KEY_NULL,                 /* 0x29 */
	DANCY_KEY_NULL,                 /* 0x2A */
	DANCY_KEY_NULL,                 /* 0x2B */
	DANCY_KEY_NULL,                 /* 0x2C */
	DANCY_KEY_NULL,                 /* 0x2D */
	DANCY_KEY_NULL,                 /* 0x2E */
	DANCY_KEY_NULL,                 /* 0x2F */
	DANCY_KEY_NULL,                 /* 0x30 */
	DANCY_KEY_NULL,                 /* 0x31 */
	DANCY_KEY_NULL,                 /* 0x32 */
	DANCY_KEY_NULL,                 /* 0x33 */
	DANCY_KEY_NULL,                 /* 0x34 */
	DANCY_KEY_NULL,                 /* 0x35 */
	DANCY_KEY_NULL,                 /* 0x36 */
	DANCY_KEY_NULL,                 /* 0x37 */
	DANCY_KEY_NULL,                 /* 0x38 */
	DANCY_KEY_NULL,                 /* 0x39 */
	DANCY_KEY_NULL,                 /* 0x3A */
	DANCY_KEY_NULL,                 /* 0x3B */
	DANCY_KEY_NULL,                 /* 0x3C */
	DANCY_KEY_NULL,                 /* 0x3D */
	DANCY_KEY_NULL,                 /* 0x3E */
	DANCY_KEY_NULL,                 /* 0x3F */
	DANCY_KEY_NULL,                 /* 0x40 */
	DANCY_KEY_NULL,                 /* 0x41 */
	DANCY_KEY_NULL,                 /* 0x42 */
	DANCY_KEY_NULL,                 /* 0x43 */
	DANCY_KEY_NULL,                 /* 0x44 */
	DANCY_KEY_NULL,                 /* 0x45 */
	DANCY_KEY_NULL,                 /* 0x46 */
	DANCY_KEY_NULL,                 /* 0x47 */
	DANCY_KEY_PAUSE,                /* 0x48 */
	DANCY_KEY_NULL,                 /* 0x49 */
	DANCY_KEY_NULL,                 /* 0x4A */
	DANCY_KEY_NULL,                 /* 0x4B */
	DANCY_KEY_NULL,                 /* 0x4C */
	DANCY_KEY_NULL,                 /* 0x4D */
	DANCY_KEY_NULL,                 /* 0x4E */
	DANCY_KEY_NULL,                 /* 0x4F */
	DANCY_KEY_NULL,                 /* 0x50 */
	DANCY_KEY_NULL,                 /* 0x51 */
	DANCY_KEY_NULL,                 /* 0x52 */
	DANCY_KEY_NULL,                 /* 0x53 */
	DANCY_KEY_NULL,                 /* 0x54 */
	DANCY_KEY_NULL,                 /* 0x55 */
	DANCY_KEY_NULL,                 /* 0x56 */
	DANCY_KEY_NULL,                 /* 0x57 */
	DANCY_KEY_NULL,                 /* 0x58 */
	DANCY_KEY_NULL,                 /* 0x59 */
	DANCY_KEY_NULL,                 /* 0x5A */
	DANCY_KEY_NULL,                 /* 0x5B */
	DANCY_KEY_NULL,                 /* 0x5C */
	DANCY_KEY_NULL,                 /* 0x5D */
	DANCY_KEY_NULL,                 /* 0x5E */
	DANCY_KEY_NULL,                 /* 0x5F */
	DANCY_KEY_NULL,                 /* 0x60 */
	DANCY_KEY_NULL,                 /* 0x61 */
	DANCY_KEY_NULL,                 /* 0x62 */
	DANCY_KEY_NULL,                 /* 0x63 */
	DANCY_KEY_NULL,                 /* 0x64 */
	DANCY_KEY_NULL,                 /* 0x65 */
	DANCY_KEY_NULL,                 /* 0x66 */
	DANCY_KEY_NULL,                 /* 0x67 */
	DANCY_KEY_NULL,                 /* 0x68 */
	DANCY_KEY_NULL,                 /* 0x69 */
	DANCY_KEY_NULL,                 /* 0x6A */
	DANCY_KEY_NULL,                 /* 0x6B */
	DANCY_KEY_NULL,                 /* 0x6C */
	DANCY_KEY_NULL,                 /* 0x6D */
	DANCY_KEY_NULL,                 /* 0x6E */
	DANCY_KEY_NULL,                 /* 0x6F */
	DANCY_KEY_NULL,                 /* 0x70 */
	DANCY_KEY_NULL,                 /* 0x71 */
	DANCY_KEY_NULL,                 /* 0x72 */
	DANCY_KEY_NULL,                 /* 0x73 */
	DANCY_KEY_NULL,                 /* 0x74 */
	DANCY_KEY_NULL,                 /* 0x75 */
	DANCY_KEY_NULL,                 /* 0x76 */
	DANCY_KEY_NULL,                 /* 0x77 */
	DANCY_KEY_NULL,                 /* 0x78 */
	DANCY_KEY_NULL,                 /* 0x79 */
	DANCY_KEY_NULL,                 /* 0x7A */
	DANCY_KEY_NULL,                 /* 0x7B */
	DANCY_KEY_NULL,                 /* 0x7C */
	DANCY_KEY_NULL,                 /* 0x7D */
	DANCY_KEY_NULL,                 /* 0x7E */
	DANCY_KEY_NULL                  /* 0x7F */
};

unsigned long key_get_code(void)
{
	EFI_KEY_DATA key_data;
	unsigned scan_code, unicode_char;
	unsigned shift = 0, caps_lock = 0;
	unsigned long key_code = 0;
	EFI_STATUS s;

	if (input == NULL)
		return 0;

	memset(&key_data, 0, sizeof(EFI_KEY_DATA));

	s = input->ReadKeyStrokeEx(input, &key_data);

	if (s != EFI_SUCCESS)
		return 0;

	scan_code = (unsigned)key_data.Key.ScanCode;
	unicode_char = (unsigned)key_data.Key.UnicodeChar;

	if (scan_code > 0 && scan_code < 128) {
		key_code = (unsigned long)table_scan[scan_code];
	} else if (unicode_char > 0 && unicode_char < 128) {
		shift = (unsigned long)table_code[unicode_char].shift;
		key_code = (unsigned long)table_code[unicode_char].key;
	}

	if ((key_data.KeyState.KeyToggleState & 0x04u) != 0)
		caps_lock = 1;

	if (shift == 1 && caps_lock == 1)
		key_code |= DANCY_KEYMOD_SHIFT;
	else if (shift == 2 && caps_lock == 0)
		key_code |= DANCY_KEYMOD_SHIFT;
	else if (shift == 3)
		key_code |= DANCY_KEYMOD_SHIFT;

	if ((key_data.KeyState.KeyShiftState & 0x80000000u) != 0) {
		uint32_t shift_state = key_data.KeyState.KeyShiftState;

		if ((shift_state & 0x01u) != 0)
			key_code |= DANCY_KEYMOD_RSHIFT;
		if ((shift_state & 0x02u) != 0)
			key_code |= DANCY_KEYMOD_LSHIFT;
		if ((shift_state & 0x04u) != 0)
			key_code |= DANCY_KEYMOD_RCTRL;
		if ((shift_state & 0x08u) != 0)
			key_code |= DANCY_KEYMOD_LCTRL;
		if ((shift_state & 0x10u) != 0)
			key_code |= DANCY_KEYMOD_RALT;
		if ((shift_state & 0x20u) != 0)
			key_code |= DANCY_KEYMOD_LALT;
		if ((shift_state & 0x40u) != 0)
			key_code |= DANCY_KEYMOD_RGUI;
		if ((shift_state & 0x80u) != 0)
			key_code |= DANCY_KEYMOD_LGUI;
	}

	return key_code;
}
