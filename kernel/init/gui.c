/*
 * Copyright (c) 2020 Antti Tiihala
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
 * init/gui.c
 *      Graphical user interface for pre-kernel environment
 */

#include <init.h>

static unsigned char *back_buffer;
static struct b_video_info vi;

extern void *ttf;
static size_t ttf_height = 16;

static size_t ttf_bitmap_size;
static unsigned char *ttf_bitmap;

static const uint32_t colors[16] = {
	DANCY_PALETTE_0,
	DANCY_PALETTE_1,
	DANCY_PALETTE_2,
	DANCY_PALETTE_3,
	DANCY_PALETTE_4,
	DANCY_PALETTE_5,
	DANCY_PALETTE_6,
	DANCY_PALETTE_7,
	DANCY_PALETTE_8,
	DANCY_PALETTE_9,
	DANCY_PALETTE_A,
	DANCY_PALETTE_B,
	DANCY_PALETTE_C,
	DANCY_PALETTE_D,
	DANCY_PALETTE_E,
	DANCY_PALETTE_F
};

static const unsigned char ttf_colors[8] = {
	0x07, /* 0x00 */
	0x06, /* 0x01 */
	0x05, /* 0x02 */
	0x04, /* 0x03 */
	0x03, /* 0x04 */
	0x02, /* 0x05 */
	0x01, /* 0x06 */
	0x00  /* 0x07 */
};

static const unsigned char ttf_colors_window[8] = {
	0x00, /* 0x00 */
	0x01, /* 0x01 */
	0x02, /* 0x02 */
	0x03, /* 0x03 */
	0x04, /* 0x04 */
	0x05, /* 0x05 */
	0x06, /* 0x06 */
	0x07  /* 0x07 */
};

static uint16_t palette16[256];
static uint8_t  palette24[768];
static uint32_t palette32[256];

struct gui_window {
	struct gui_window *prev;
	unsigned char *win_behind;
	unsigned char *win_buffer;
	unsigned win_width;
	unsigned win_height;
	unsigned x;
	unsigned y;
};

static struct gui_window *gui_window_stack;

static void (*blit)(void);

static void blit_vga(void)
{
	const unsigned char *src = back_buffer;
	unsigned char *dst = (unsigned char *)vi.framebuffer;
	unsigned b0, b1, b2, b3;
	unsigned x, y;

	unsigned char row_plane0[128];
	unsigned char row_plane1[128];
	unsigned char row_plane2[128];
	unsigned char row_plane3[128];

	for (y = 0; y < 480; y++) {
		unsigned char *ptr_plane0 = &row_plane0[0];
		unsigned char *ptr_plane1 = &row_plane1[0];
		unsigned char *ptr_plane2 = &row_plane2[0];
		unsigned char *ptr_plane3 = &row_plane3[0];

		for (x = 0; x < 640; x += 8) {
			unsigned src0 = src[x + 0];
			unsigned src1 = src[x + 1];
			unsigned src2 = src[x + 2];
			unsigned src3 = src[x + 3];
			unsigned src4 = src[x + 4];
			unsigned src5 = src[x + 5];
			unsigned src6 = src[x + 6];
			unsigned src7 = src[x + 7];

			b0  = (src0 & 1) << 7;
			b0 |= (src1 & 1) << 6;
			b0 |= (src2 & 1) << 5;
			b0 |= (src3 & 1) << 4;
			b0 |= (src4 & 1) << 3;
			b0 |= (src5 & 1) << 2;
			b0 |= (src6 & 1) << 1;
			b0 |= (src7 & 1);

			b1  = (src0 & 2) << 6;
			b1 |= (src1 & 2) << 5;
			b1 |= (src2 & 2) << 4;
			b1 |= (src3 & 2) << 3;
			b1 |= (src4 & 2) << 2;
			b1 |= (src5 & 2) << 1;
			b1 |= (src6 & 2);
			b1 |= (src7 & 2) >> 1;

			b2  = (src0 & 4) << 5;
			b2 |= (src1 & 4) << 4;
			b2 |= (src2 & 4) << 3;
			b2 |= (src3 & 4) << 2;
			b2 |= (src4 & 4) << 1;
			b2 |= (src5 & 4);
			b2 |= (src6 & 4) >> 1;
			b2 |= (src7 & 4) >> 2;

			b3  = (src0 & 8) << 4;
			b3 |= (src1 & 8) << 3;
			b3 |= (src2 & 8) << 2;
			b3 |= (src3 & 8) << 1;
			b3 |= (src4 & 8);
			b3 |= (src5 & 8) >> 1;
			b3 |= (src6 & 8) >> 2;
			b3 |= (src7 & 8) >> 3;

			*ptr_plane0++ = (unsigned char)b0;
			*ptr_plane1++ = (unsigned char)b1;
			*ptr_plane2++ = (unsigned char)b2;
			*ptr_plane3++ = (unsigned char)b3;
		}

		cpu_out8(0x03C4, 0x02);
		cpu_out8(0x03C5, 0x01);

		for (x = 0; x < 80; x++)
			dst[x] = row_plane0[x];

		cpu_out8(0x03C4, 0x02);
		cpu_out8(0x03C5, 0x02);

		for (x = 0; x < 80; x++)
			dst[x] = row_plane1[x];

		cpu_out8(0x03C4, 0x02);
		cpu_out8(0x03C5, 0x04);

		for (x = 0; x < 80; x++)
			dst[x] = row_plane2[x];

		cpu_out8(0x03C4, 0x02);
		cpu_out8(0x03C5, 0x08);

		for (x = 0; x < 80; x++)
			dst[x] = row_plane3[x];

		src += 640;
		dst += 80;
	}

	cpu_out8(0x03C4, 0x02);
	cpu_out8(0x03C5, 0x0F);
}

static void blit_1byte(void)
{
	const unsigned char *src = back_buffer;
	unsigned char *dst = (unsigned char *)vi.framebuffer;
	unsigned src_add = (unsigned)vi.width;
	unsigned dst_add = (unsigned)vi.stride;
	unsigned x, y;

	for (y = 0; y < vi.height; y++) {
		for (x = 0; x < vi.width; x++)
			dst[x] = src[x];
		src += src_add;
		dst += dst_add;
	}
}

static void blit_2byte(void)
{
	const unsigned char *src = back_buffer;
	uint16_t *dst = (uint16_t *)vi.framebuffer;
	unsigned src_add = (unsigned)vi.width;
	unsigned dst_add = (unsigned)vi.stride / 2;
	unsigned x, y;

	for (y = 0; y < vi.height; y++) {
		for (x = 0; x < vi.width; x++)
			dst[x] = palette16[src[x]];
		src += src_add;
		dst += dst_add;
	}
}

static void blit_3byte(void)
{
	const unsigned char *src = back_buffer;
	unsigned char *dst = (unsigned char *)vi.framebuffer;
	unsigned src_add = (unsigned)vi.width;
	unsigned dst_add = (unsigned)(vi.stride - (vi.width * 3));
	unsigned i, x, y;

	for (y = 0; y < vi.height; y++) {
		for (x = 0; x < vi.width; x++) {
			i = src[x];
			*dst++ = palette24[i + 0];
			*dst++ = palette24[i + 256];
			*dst++ = palette24[i + 512];
		}
		src += src_add;
		dst += dst_add;
	}
}

static void blit_4byte(void)
{
	const unsigned char *src = back_buffer;
	uint32_t *dst = (uint32_t *)vi.framebuffer;
	unsigned src_add = (unsigned)vi.width;
	unsigned dst_add = (unsigned)vi.stride / 4;
	unsigned x, y;

	for (y = 0; y < vi.height; y++) {
		for (x = 0; x < vi.width; x++)
			dst[x] = palette32[src[x]];
		src += src_add;
		dst += dst_add;
	}
}

static void blit_15bit_init(void)
{
	size_t i;

	for (i = 0; i < sizeof(colors) / sizeof(colors[0]); i++) {
		uint32_t color = colors[i];
		uint32_t e = 0;

		e |= ((color >> 0x13) & 0x1Fu);
		e |= ((color >> 0x0B) & 0x1Fu) << 0x05;
		e |= ((color >> 0x03) & 0x1Fu) << 0x0A;

		palette16[i] = (uint16_t)e;
	}
}

static void blit_16bit_init(void)
{
	size_t i;

	for (i = 0; i < sizeof(colors) / sizeof(colors[0]); i++) {
		uint32_t color = colors[i];
		uint32_t e = 0;

		e |= ((color >> 0x13) & 0x1Fu);
		e |= ((color >> 0x0A) & 0x3Fu) << 0x05;
		e |= ((color >> 0x03) & 0x1Fu) << 0x0B;

		palette16[i] = (uint16_t)e;
	}
}

static void blit_rgb_init(void)
{
	size_t i;

	for (i = 0; i < sizeof(colors) / sizeof(colors[0]); i++) {
		uint32_t color = colors[i];
		palette24[i + 0x000] = (uint8_t)((color) & 0xFFu);
		palette24[i + 0x100] = (uint8_t)((color >> 0x08) & 0xFFu);
		palette24[i + 0x200] = (uint8_t)((color >> 0x10) & 0xFFu);
	}
}

static void blit_bgr_init(void)
{
	size_t i;

	for (i = 0; i < sizeof(colors) / sizeof(colors[0]); i++) {
		uint32_t color = colors[i];
		palette24[i + 0x200] = (uint8_t)((color) & 0xFFu);
		palette24[i + 0x100] = (uint8_t)((color >> 0x08) & 0xFFu);
		palette24[i + 0x000] = (uint8_t)((color >> 0x10) & 0xFFu);
	}
}

static void blit_rgbx_init(void)
{
	size_t i;

	for (i = 0; i < sizeof(colors) / sizeof(colors[0]); i++) {
		uint32_t color = colors[i];

		palette32[i] = 0;
		palette32[i] |= ((color) & 0xFFu);
		palette32[i] |= ((color >> 0x08) & 0xFFu) << 0x08;
		palette32[i] |= ((color >> 0x10) & 0xFFu) << 0x10;
	}
}

static void blit_bgrx_init(void)
{
	size_t i;

	for (i = 0; i < sizeof(colors) / sizeof(colors[0]); i++) {
		uint32_t color = colors[i];

		palette32[i] = 0;
		palette32[i] |= ((color >> 0x10) & 0xFFu);
		palette32[i] |= ((color >> 0x08) & 0xFFu) << 0x08;
		palette32[i] |= ((color) & 0xFFu) << 0x10;
	}
}

static void ttf_set_colors(const unsigned char *indices)
{
	unsigned height = (unsigned)ttf_height;
	unsigned p, x, y;

	for (y = 0; y < height; y++) {
		unsigned char *row = &ttf_bitmap[y * height];

		for (x = 0; x < height; x++) {
			p = row[x];
			row[x] = indices[p & 0x07u];
		}
	}
}

int gui_init(void)
{
	size_t size;

	/*
	 * This gui_init function is the only one in this module
	 * that uses boot loader services.
	 */
	if (!b_get_structure(&vi, B_VIDEO_INFO))
		return 1;
	if (!ttf)
		return 1;

	size = ((vi.width * vi.height) + 0xFFFu) & 0xFFFFF000u;
	if (!size)
		return 1;

	ttf_bitmap_size = ttf_height * ttf_height;
	ttf_bitmap = malloc(ttf_bitmap_size);

	if (!ttf_bitmap) {
		memset(&vi, 0, sizeof(vi));
		return 1;
	}

	back_buffer = aligned_alloc(4096, size);

	switch (vi.mode) {
	case B_MODE_VGA:
		blit = blit_vga;
		break;
	case B_MODE_PALETTE:
		blit = blit_1byte;
		break;
	case B_MODE_15_BIT:
		blit_15bit_init();
		blit = blit_2byte;
		break;
	case B_MODE_16_BIT:
		blit_16bit_init();
		blit = blit_2byte;
		break;
	case B_MODE_RED_GREEN_BLUE:
		blit_rgb_init();
		blit = blit_3byte;
		break;
	case B_MODE_BLUE_GREEN_RED:
		blit_bgr_init();
		blit = blit_3byte;
		break;
	case B_MODE_RED_GREEN_BLUE_RESERVED:
		blit_rgbx_init();
		blit = blit_4byte;
		break;
	case B_MODE_BLUE_GREEN_RED_RESERVED:
		blit_bgrx_init();
		blit = blit_4byte;
		break;
	default:
		free(back_buffer), back_buffer = NULL;
		break;
	}

	if (!back_buffer) {
		free(ttf_bitmap), ttf_bitmap = NULL;
		memset(&vi, 0, sizeof(vi));
		return 1;
	}

	memset(back_buffer, 0x02, size);

	b_log("Graphical User Interface\n");
	b_log("\tBack buffer set at %p\n", back_buffer);
	b_log("\n");

	return 0;
}

int gui_create_window(const char *name, int x1, int y1, int x2, int y2)
{
	struct gui_window *win;
	unsigned char *win_buffer;
	unsigned win_width;
	unsigned win_height;
	unsigned x, y;

	if (!back_buffer)
		return 1;

	if (x1 < 0 || y1 < 0)
		return 1;
	if (x2 < x1 || y2 < y1)
		return 1;
	if (x2 >= (int)vi.width || y2 >= (int)vi.height)
		return 1;

	if ((x2 - x1) < 64 || (y2 - y1) < 64)
		return 1;

	win_buffer = back_buffer + (y1 * (int)vi.width) + x1;
	win_width = (unsigned)(x2 - x1);
	win_height = (unsigned)(y2 - y1);

	win = calloc(1, sizeof(*win));
	if (!win)
		return 2;

	win->prev = gui_window_stack;
	win->win_behind = malloc(win_width * win_height);

	if (!win->win_behind)
		return free(win), 2;

	win->win_buffer = win_buffer;
	win->win_width = win_width;
	win->win_height = win_height;

	gui_window_stack = win;

	for (y = 0; y < win_height; y++) {
		unsigned char *src = win->win_buffer + (y * vi.width);
		unsigned char *dst = win->win_behind + (y * win_width);

		for (x = 0; x < win_width; x++)
			dst[x] = src[x];
	}

	for (y = 0; y < ttf_height; y++) {
		for (x = 0; x < win_width; x++)
			win_buffer[x + y * vi.width] = ttf_colors_window[0];
	}

	/*
	 * Draw window title.
	 */
	{
		unsigned char *ptr = win_buffer + vi.width + 4;
		unsigned total_width = 0;
		unsigned width = 0;
		unsigned code_points[2] = { 0, 0 };

		ttf_set_bitmap(ttf, ttf_bitmap_size, ttf_bitmap);
		ttf_set_shades(ttf, 8);

		while (*name != '\0')  {
			int kerning = 0;

			code_points[0] = code_points[1];
			code_points[1] = (unsigned)*name;

			ttf_render(ttf, (unsigned)*name++, &width);
			ttf_set_colors(&ttf_colors_window[0]);

			if (code_points[0])
				ttf_get_kerning(ttf, code_points, &kerning);
			ptr += kerning;

			for (y = 0; y < ttf_height - 1; y++) {
				for (x = 0; x < width; x++) {
					unsigned char c;
					c = ttf_bitmap[x + y * ttf_height];
					if (ttf_colors_window[0] != c)
						ptr[x + y * vi.width] = c;
				}
			}
			total_width += width;
			total_width = (unsigned)((int)total_width + kerning);
			if (total_width + ttf_height >= win_width)
				break;
			ptr += width;
		}
	}

	for (y = (unsigned)ttf_height; y < win_height - 1; y++) {
		win_buffer[0 + y * vi.width] = ttf_colors_window[0];
		for (x = 1; x < win_width - 1; x++)
			win_buffer[x + y * vi.width] = ttf_colors[0];
		win_buffer[x + y * vi.width] = ttf_colors_window[0];
	}

	for (x = 0; x < win_width; x++)
		win_buffer[x + y * vi.width] = ttf_colors_window[0];

	return 0;
}

int gui_delete_window(void)
{
	struct gui_window *win;
	unsigned x, y;

	if (!gui_window_stack)
		return 1;

	win = gui_window_stack;

	for (y = 0; y < win->win_height; y++) {
		unsigned char *src = win->win_behind + (y * win->win_width);
		unsigned char *dst = win->win_buffer + (y * vi.width);

		for (x = 0; x < win->win_width; x++)
			dst[x] = src[x];
	}

	gui_window_stack = gui_window_stack->prev;

	free(win->win_behind);
	free(win);

	return 0;
}

int gui_move_window(int x, int y)
{
	struct gui_window *win = gui_window_stack;
	unsigned i, j;

	if (!gui_window_stack)
		return 1;

	if (x < 0 || (unsigned)x + win->win_width > vi.width)
		return 1;

	if (y < 0 || (unsigned)y + win->win_height > vi.height)
		return 1;

	for (i = 0; i < win->win_height; i++) {
		unsigned behind = i * win->win_width;
		unsigned buffer = i * vi.width;

		for (j = 0; j < win->win_width; j++) {
			unsigned char tmp = win->win_behind[behind];
			win->win_behind[behind++] = win->win_buffer[buffer];
			win->win_buffer[buffer++] = tmp;
		}
	}

	win->win_buffer = back_buffer + (y * (int)vi.width) + x;

	for (i = 0; i < win->win_height; i++) {
		unsigned behind = i * win->win_width;
		unsigned buffer = i * vi.width;

		for (j = 0; j < win->win_width; j++) {
			unsigned char tmp = win->win_behind[behind];
			win->win_behind[behind++] = win->win_buffer[buffer];
			win->win_buffer[buffer++] = tmp;
		}
	}

	return 0;
}

static void handle_newline(void)
{
	unsigned win_width = gui_window_stack->win_width - 2;
	unsigned win_height = gui_window_stack->win_height - 3;
	size_t y_limit = win_height - (2 * ttf_height);

	gui_window_stack->y += (unsigned)ttf_height;

	if (gui_window_stack->y > y_limit) {
		unsigned char *dst = gui_window_stack->win_buffer;
		unsigned char *src;
		size_t x, y;

		dst += (ttf_height + 2) * vi.width + 1;
		src = dst + (ttf_height * vi.width);

		for (y = (2 * ttf_height); y < win_height; y++) {
			for (x = 0; x < win_width; x++)
				dst[x] = src[x];
			src += vi.width;
			dst += vi.width;
		}
		for (y = 0; y < ttf_height; y++) {
			for (x = 0; x < win_width; x++)
				dst[x] = ttf_colors[0];
			dst += vi.width;
		}
		gui_window_stack->y -= (unsigned)ttf_height;
	}
	gui_window_stack->x = 0;
}

void gui_print(const char *format, ...)
{
	unsigned char *dst_buffer;
	unsigned win_width;

	unsigned code_points[2] = { 0, 0 };

	char buf[4096];
	int ret, i;

	if (!gui_window_stack)
		return;

	va_list va;
	va_start(va, format);
	ret = vsnprintf(buf, sizeof(buf), format, va);
	va_end(va);

	if (ttf_set_bitmap(ttf, ttf_bitmap_size, ttf_bitmap))
		return;
	if (ttf_set_shades(ttf, 8))
		return;

	dst_buffer = gui_window_stack->win_buffer;
	win_width = gui_window_stack->win_width - 2;

	dst_buffer += (ttf_height + 2) * vi.width + 2;

	for (i = 0; i < ret; i++) {
		unsigned char *src = ttf_bitmap;
		unsigned char *dst = dst_buffer;

		unsigned c = (unsigned)buf[i];
		unsigned width = 0;
		int kerning = 0;
		unsigned x, y;

		code_points[0] = code_points[1];
		code_points[1] = c;

		if (c == 0x0D) {
			code_points[1] = 0;
			gui_window_stack->x = 0;
			continue;
		}
		if (c == 0x0A) {
			code_points[1] = 0;
			handle_newline();
			continue;
		}

		ttf_render(ttf, c, &width);
		ttf_set_colors(&ttf_colors[0]);

		if (code_points[0])
			ttf_get_kerning(ttf, code_points, &kerning);

		dst += gui_window_stack->x;
		dst += (gui_window_stack->y * vi.width);
		dst += kerning;

		for (y = 0; y < ttf_height; y++) {
			for (x = 0; x < width; x++) {
				if (kerning == 0 || ttf_colors[0] != src[x])
					dst[x] = src[x];
			}
			src += ttf_height;
			dst += vi.width;
		}

		x = gui_window_stack->x + width;
		gui_window_stack->x = (unsigned)((int)x + kerning);

		if (gui_window_stack->x + ttf_height >= win_width)
			handle_newline();
	}
}

void gui_refresh(void)
{
	if (back_buffer)
		blit();
}
