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
 * uefi/video.c
 *      Graphics Output Protocol
 */

#include <uefi.h>

uint32_t video_active;

uint32_t video_column;
uint32_t video_row;

uint32_t video_columns = 90;
uint32_t video_rows = 30;

extern uint64_t font7x9[];
static unsigned char video_buffer[90 * 30];

static EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;

void video_init(void)
{
	EFI_HANDLE_PROTOCOL HandleProtocol;
	EFI_LOCATE_HANDLE LocateHandle;
	EFI_HANDLE handles[128];
	EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *mode;
	EFI_STATUS s;

	HandleProtocol = gSystemTable->BootServices->HandleProtocol;
	LocateHandle = gSystemTable->BootServices->LocateHandle;

	u_log("Graphics Output Protocol\n");

	/*
	 * Find all the handles that support Graphics Output Protocol.
	 */
	{
		EFI_GUID guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
		uint64_t size = (uint64_t)(sizeof(handles));

		s = LocateHandle(ByProtocol, &guid, NULL, &size, &handles[0]);

		if (s != EFI_SUCCESS || size < sizeof(EFI_HANDLE)) {
			u_log("\tNone\n\n");
			return;
		}

		u_log("\tFound %llu device%s\n", (size / sizeof(EFI_HANDLE)),
			((size / sizeof(EFI_HANDLE)) > 1) ? "s" : "");
	}

	/*
	 * Try to use the device of ConsoleOutHandle.
	 */
	{
		EFI_GUID guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
		EFI_GRAPHICS_OUTPUT_PROTOCOL *graphics_output;
		EFI_HANDLE handle = gSystemTable->ConsoleOutHandle;

		s = HandleProtocol(handle, &guid, (void **)&graphics_output);

		if (s == EFI_SUCCESS) {
			u_log("\tUsing ConsoleOutHandle\n");
			gop = graphics_output;
		}
	}

	/*
	 * If the device of ConsoleOutHandle did not work, use the first one.
	 */
	if (gop == NULL) {
		EFI_GUID guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
		EFI_GRAPHICS_OUTPUT_PROTOCOL *graphics;

		s = HandleProtocol(handles[0], &guid, (void **)&graphics);

		if (s != EFI_SUCCESS) {
			/*
			 * This should be unlikely. LocateHandle already said
			 * that the protocol is supported.
			 */
			u_log("\tError %016llX\n\n", s);
			return;
		}
		gop = graphics;
	}

	mode = gop->Mode;
	u_log("\n");
	u_log("\tMaxMode             %u\n", mode->MaxMode);
	u_log("\tMode                %u\n", mode->Mode);
	u_log("\tHorizontal          %u\n", mode->Info->HorizontalResolution);
	u_log("\tVertical            %u\n", mode->Info->VerticalResolution);

	if (mode->Info->PixelFormat == 0) {
		const char *name = "PixelRedGreenBlueReserved8BitPerColor";
		u_log("\tPixelFormat         %s\n", name);
	} else if (mode->Info->PixelFormat == 1) {
		const char *name = "PixelBlueGreenRedReserved8BitPerColor";
		u_log("\tPixelFormat         %s\n", name);
	} else if (mode->Info->PixelFormat == 2) {
		const char *name = "PixelBitMask";
		u_log("\tPixelFormat         %s\n", name);
	}
	u_log("\tPixelsPerScanLine   %u\n", mode->Info->PixelsPerScanLine);
	u_log("\tFrameBufferBase     %016llX\n", mode->FrameBufferBase);
	u_log("\n");

	/*
	 * Check whether the current mode is suitable for "video_active".
	 */
	{
		uint32_t h = gop->Mode->Info->HorizontalResolution;
		uint32_t v = gop->Mode->Info->VerticalResolution;

		if (h <= (video_columns * 7) || v <= (video_rows * 14))
			return;
		if (gop->Mode->Info->PixelFormat >= 2)
			return;
		video_active = 1;
		video_clear(0);
	}
}

void video_clear(int mode)
{
	if (video_active != 0) {
		uint32_t h = gop->Mode->Info->HorizontalResolution;
		uint32_t v = gop->Mode->Info->VerticalResolution;
		uint32_t *fb = (uint32_t *)gop->Mode->FrameBufferBase;
		uint32_t stride = gop->Mode->Info->PixelsPerScanLine;
		uint32_t i, j;

		for (i = 0; i < v; i++) {
			for (j = 0; j < h; j++)
				fb[j] = 0x00101010;
			fb += stride;
		}
	}

	if (mode != 0) {
		video_column = 0;
		video_row = 0;
		memset(&video_buffer[0], 0, sizeof(video_buffer));
	}
}

size_t video_get_edid(struct b_video_edid *out)
{
	memset(out, 0, sizeof(struct b_video_edid));
	return 0;
}

size_t video_get_info(struct b_video_info *out)
{
	memset(out, 0, sizeof(struct b_video_info));
	if (video_active == 0)
		return 0;

	if (gop->Mode->Info->PixelFormat == 0)
		out->mode = B_MODE_RED_GREEN_BLUE_RESERVED;
	else
		out->mode = B_MODE_BLUE_GREEN_RED_RESERVED;

	out->width = gop->Mode->Info->HorizontalResolution;
	out->height = gop->Mode->Info->VerticalResolution;
	out->stride = gop->Mode->Info->PixelsPerScanLine * 4;
	out->framebuffer = (phys_addr_t)gop->Mode->FrameBufferBase;

	return (sizeof(struct b_video_info));
}

static void render_1x(uint32_t h, uint32_t v)
{
	uint32_t *fb = (uint32_t *)gop->Mode->FrameBufferBase;
	uint32_t stride = gop->Mode->Info->PixelsPerScanLine;
	uint32_t x, y;
	uint64_t font;

	h -= (video_columns * 7);
	v -= (video_rows * 14);

	if (((h & 0x80000000u) | (v & 0x80000000u)) != 0)
		return;

	fb += ((h / 4) + (stride * (v / 4)));

	for (y = 0; y < video_rows; y++) {
		for (x = 0; x < video_columns; x++) {
			uint32_t *ptr = fb + (x * 7);
			unsigned char u;
			uint32_t i, j, bc;
			int hl, offset;

			u = video_buffer[x + (y * video_columns)];
			offset = (int)(u & 0x7Fu) - 0x20;
			font = (offset >= 0) ? font7x9[offset] : 0;

			hl = (int)(u >> 7);
			bc = hl ? 0x00AAAAAA : 0;

			for (i = 0; i < 2; i++) {
				for (j = 0; j < 7; j++)
					ptr[j] = bc;
				ptr += stride;
			}
			if ((font & 0x8000000000000000ull) != 0) {
				for (i = 0; i < 2; i++) {
					for (j = 0; j < 7; j++)
						ptr[j] = bc;
					ptr += stride;
				}
				offset = 1;
			} else {
				offset = 3;
			}
			for (i = 0; i < 9; i++) {
				for (j = 0; j < 7; j++) {
					if ((int)(font & 1ull) != hl)
						ptr[j] = 0x00AAAAAA;
					else
						ptr[j] = 0;
					font >>= 1;
				}
				ptr += stride;
			}
			for (i = 0; i < (uint32_t)offset; i++) {
				for (j = 0; j < 7; j++)
					ptr[j] = bc;
				ptr += stride;
			}
		}
		fb += (stride * 14);
	}
}

static int render_2x(uint32_t h, uint32_t v)
{
	uint32_t *fb = (uint32_t *)gop->Mode->FrameBufferBase;
	uint32_t stride = gop->Mode->Info->PixelsPerScanLine;
	uint32_t x, y;
	uint64_t font;

	if (h < 1920 || v < 1080)
		return 1;

	h -= (video_columns * 14);
	v -= (video_rows * 28);

	if (((h & 0x80000000u) | (v & 0x80000000u)) != 0)
		return 1;

	fb += ((h / 4) + (stride * (v / 4)));

	for (y = 0; y < video_rows; y++) {
		for (x = 0; x < video_columns; x++) {
			uint32_t *ptr = fb + (x * 14);
			unsigned char u;
			uint32_t i, j, bc;
			int hl, offset;

			u = video_buffer[x + (y * video_columns)];
			offset = (int)(u & 0x7Fu) - 0x20;
			font = (offset >= 0) ? font7x9[offset] : 0;

			hl = (int)(u >> 7);
			bc = hl ? 0x00AAAAAA : 0;

			for (i = 0; i < 4; i++) {
				for (j = 0; j < 14; j++)
					ptr[j] = bc;
				ptr += stride;
			}
			if ((font & 0x8000000000000000ull) != 0) {
				for (i = 0; i < 4; i++) {
					for (j = 0; j < 14; j++)
						ptr[j] = bc;
					ptr += stride;
				}
				offset = 2;
			} else {
				offset = 6;
			}
			for (i = 0; i < 18; i += 2) {
				for (j = 0; j < 14; j += 2) {
					uint32_t color = 0x00AAAAAA;
					if ((int)(font & 1ull) != hl) {
						ptr[j + 0] = color;
						ptr[j + 1] = color;
						ptr[j + stride + 0] = color;
						ptr[j + stride + 1] = color;
					} else {
						ptr[j + 0] = 0;
						ptr[j + 1] = 0;
						ptr[j + stride + 0] = 0;
						ptr[j + stride + 1] = 0;
					}
					font >>= 1;
				}
				ptr += stride * 2;
			}
			for (i = 0; i < (uint32_t)offset; i++) {
				for (j = 0; j < 14; j++)
					ptr[j] = bc;
				ptr += stride;
			}
		}
		fb += (stride * 28);
	}
	return 0;
}

void video_output_string(const char *str, unsigned int len, int hl, int cr)
{
	char c;

	while ((c = *str) != '\0') {
		unsigned char u = *((unsigned char *)(str++));
		uint32_t offset;

		if (u > 0x7E)
			u = 0x7F;
		if (hl != 0)
			u = (unsigned char)(u | 0x80u);

		switch (c) {
		case '\b':
			video_column -= ((video_column > 0) ? 1 : 0);
			break;
		case '\t':
			video_column += 1;
			break;
		case '\n':
			if (cr != 0)
				video_column = 0;
			video_row += 1;
			break;
		case '\r':
			video_column = 0;
			break;
		default:
			offset = video_column + (video_row * video_columns);
			video_buffer[offset] = u;
			video_column += 1;
			break;
		}

		if (video_column >= video_columns) {
			video_column = 0;
			video_row += 1;
		}

		if (video_row >= video_rows) {
			void *dst = &video_buffer[0];
			void *src = &video_buffer[video_columns];
			uint32_t size = video_columns * (video_rows - 1);

			memmove(dst, src, (size_t)size);
			memset(&video_buffer[size], 0, (size_t)video_columns);
			video_row = video_rows - 1;
		}

		if (len == 1)
			break;
		if (len > 1)
			len -= 1;
	}

	if (video_active != 0) {
		uint32_t h = gop->Mode->Info->HorizontalResolution;
		uint32_t v = gop->Mode->Info->VerticalResolution;

		if (!render_2x(h, v))
			return;
		render_1x(h, v);
	}
}

static int qsort_info_array(const void *a, const void *b)
{
	const EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info1 = a;
	const EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info2 = b;
	uint32_t t1, t2;

	t1 = info1->HorizontalResolution;
	t2 = info2->HorizontalResolution;

	if (t1 != t2)
		return (t1 < t2) ? 1 : -1;

	t1 = info1->VerticalResolution;
	t2 = info2->VerticalResolution;

	if (t1 != t2)
		return (t1 < t2) ? 1 : -1;

	t1 = (uint32_t)info1->PixelFormat;
	t2 = (uint32_t)info2->PixelFormat;

	if (t1 != t2)
		return (t1 < t2) ? -1 : 1;

	return (info1->Version < info2->Version) ? -1 : 1;
}

void video_show_menu(void)
{
	static EFI_GRAPHICS_OUTPUT_MODE_INFORMATION info_array[1024];
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
	uint32_t i, modes;
	uint32_t f1_state, state = 0;
	unsigned rows = 13;
	EFI_STATUS s;

	uint32_t no_menu = 0;
	uint32_t preferred_width = 1024;
	uint32_t preferred_height = 768;

	/*
	 * Get settings from CONFIG.AT file.
	 */
	if (config_file_size >= 0x28) {
		const unsigned char *c = config_file;
		unsigned long w = LE16(&c[0x24]);
		unsigned long h = LE16(&c[0x26]) >> 4;

		no_menu = ((unsigned)c[0x20] & 0x02) ? 1 : 0;

		if (w != 0 && h != 0) {
			preferred_width = (uint32_t)w;
			preferred_height = (uint32_t)h;
		}
	}

	if (gop == NULL)
		return;

	modes = gop->Mode->MaxMode;

	if (modes == 0 || modes > 1024)
		return;

	for (i = 0; i < modes; i++) {
		uint64_t size = 0;

		s = gop->QueryMode(gop, i, &size, &info);

		if (s == EFI_SUCCESS && (size_t)size >= sizeof(*info)) {
			memcpy(&info_array[i], info, sizeof(*info));

			if (info->HorizontalResolution > 3840)
				info_array[i].HorizontalResolution = 0;

			if (info->VerticalResolution > 2160)
				info_array[i].HorizontalResolution = 0;

			if (info->PixelFormat >= 2)
				info_array[i].HorizontalResolution = 0;
		}

		/*
		 * Use "Version" member for saving the mode number.
		 */
		info_array[i].Version = i;
	}

	qsort(&info_array[0], (size_t)modes, sizeof(*info), qsort_info_array);

	if (modes > rows * 2)
		modes = rows * 2;

	for (i = 0; i < modes; i++) {
		info = &info_array[i];

		if (info->HorizontalResolution < 800) {
			modes = i;
			break;
		}
		if (info->VerticalResolution < 600) {
			modes = i;
			break;
		}
		if (info->HorizontalResolution == preferred_width) {
			if (info->VerticalResolution == preferred_height)
				state = i + 1;
		}
	}

	while (modes > 4 && (rows * 2 - modes) >= 2)
		rows -= 1;

	if (rows > modes)
		rows = modes;

	if (!state)
		state = modes;
	f1_state = state;

	while (state) {
		static char buf[4096];
		const uint32_t wait_loops = 300;
		uint32_t new_state = state;
		unsigned hl_start = 0;
		int first_run, n = 0;

		first_run = (buf[0] == '\0') ? 1 : 0;

		n += snprintf(&buf[n], 64, "Video Modes (GOP)\r\n");
		n = n > 0 ? n : 0;

		for (i = 0; i < modes && i < rows; i++) {
			info = &info_array[i];

			if (state == i + 1)
				hl_start = (unsigned)n + 2;

			n += snprintf(&buf[n], 64,
				"  %4dx%-4d 32-bit",
				info->HorizontalResolution,
				info->VerticalResolution);
			n = n > 0 ? n : 0;

			if (i + rows < modes) {
				info = &info_array[i + rows];

				if (state == i + rows + 1)
					hl_start = (unsigned)n + 4;

				n += snprintf(&buf[n], 64,
					"    %4dx%-4d 32-bit",
					info->HorizontalResolution,
					info->VerticalResolution);
				n = n > 0 ? n : 0;
			}

			n += snprintf(&buf[n], 64, "\r\n");
			n = n > 0 ? n : 0;
		}

		if (hl_start == 0)
			break;

		if (first_run != 0) {
			snprintf(&buf[n], 128, "\r\n"
				"  Waiting keyboard input for a few seconds"
				"...\r", 0);
		} else {
			snprintf(&buf[n], 128, "\r\n"
				"  [ESC] Close/Continue    [F1] Default"
				"    [ENTER] Set Mode\r", 0);
		}

		if (no_menu == 0) {
			b_output_string(&buf[0], hl_start);
			b_output_string_hl(&buf[hl_start], 16);
			b_output_string(&buf[hl_start + 16], 0);
		} else {
			new_state = 0xFFFFFFFFu;
		}

		for (i = 0; i < wait_loops && no_menu == 0; i++) {
			unsigned long keycode = b_get_keycode();;
			int shift = keycode & DANCY_KEYMOD_SHIFT;
			int alt = keycode & DANCY_KEYMOD_ALT;
			int key = (int)(keycode & 0xFFFFul);

			switch (key) {
			case DANCY_KEY_TAB:
				if (shift)
					new_state = state - 1;
				else
					new_state = state + 1;
				break;
			case DANCY_KEY_HOME:
				new_state = 1;
				break;
			case DANCY_KEY_PAGEUP:
				new_state = state - 1;
				break;
			case DANCY_KEY_END:
				new_state = modes;
				break;
			case DANCY_KEY_PAGEDOWN:
				new_state = state + 1;
				break;
			case DANCY_KEY_RIGHTARROW:
				new_state = state + rows;
				break;
			case DANCY_KEY_LEFTARROW:
				new_state = state - rows;
				break;
			case DANCY_KEY_DOWNARROW:
				new_state = state + 1;
				break;
			case DANCY_KEY_UPARROW:
				new_state = state - 1;
				break;
			default:
				break;
			}

			if (new_state == 0 || new_state > modes)
				new_state = state;

			if (state != new_state) {
				state = new_state;
				break;
			}

			if (key == DANCY_KEY_ESCAPE) {
				state = 0;
				break;
			}

			if (key == DANCY_KEY_F1) {
				state = f1_state;
				new_state = alt ? 0xFFFFFFFEu :  0xFFFFFFFFu;
				break;
			}

			if (key == DANCY_KEY_ENTER) {
				new_state = 0xFFFFFFFFu;
				break;
			}

			u_stall(10);
		}

		if (first_run && i >= wait_loops) {
			new_state = 0xFFFFFFFFu;
			no_menu = 1;
		}

		if (state != 0 && new_state == 0xFFFFFFFEu) {
			info = &info_array[state - 1];
			uint32_t new_mode = info->Version;

			video_clear(1);
			video_active = 0;
			gop->SetMode(gop, new_mode);
			u_clear_screen();
		}

		if (state != 0 && new_state == 0xFFFFFFFFu) {
			info = &info_array[state - 1];
			uint32_t new_mode = info->Version;

			if (!video_active || new_mode != gop->Mode->Mode) {
				s = gop->SetMode(gop, new_mode);
				if (s == EFI_SUCCESS) {
					if (!video_active) {
						video_active = 1;
						video_clear(1);
					} else {
						video_clear(0);
					}
				}
			}
		}

		if (no_menu != 0 || state == 0) {
			buf[0] = '\r', memset(&buf[1], ' ', 78);
			b_output_string(&buf[0], 79);
			state = 0;
		}

		if (state != 0) {
			unsigned cursor;
			unsigned offset = ((rows << 8) + 0x0200);

			cursor = (unsigned)b_output_control(0, B_GET_CURSOR);

			if (cursor >= offset)
				cursor -= offset;
			else
				cursor = 0;

			b_output_control(cursor, B_SET_CURSOR);
		}
	}

	u_print("\n");
}
