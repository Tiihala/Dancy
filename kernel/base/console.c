/*
 * Copyright (c) 2021 Antti Tiihala
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
 * base/console.c
 *      Kernel console
 */

#include <dancy.h>

static mtx_t con_mtx;
static int con_ready;

static int con_column;
static int con_columns;

static int con_row;
static int con_rows;

static int con_cells;
static uint32_t *con_buffer;
static uint32_t *con_fb_start;

static uint32_t con_attribute;

static const uint32_t con_rendered_bit  = 0x80000000;
static const uint32_t con_intensity_bit = 0x40000000;
static const uint32_t con_reversed_bit  = 0x20000000;
static const uint32_t con_underline_bit = 0x10000000;

static size_t con_escape_size;
static char con_escape_buffer[128];

static char con_print_buffer[4096];

#define CON_RGB(r, g, b) (r | (g << 8) | (b << 16))

static const int con_color_table[16] = {
	CON_RGB(  0,   0,   0),
	CON_RGB(170,   0,   0),
	CON_RGB(  0, 170,   0),
	CON_RGB(170,  85,   0),
	CON_RGB(  0,   0, 170),
	CON_RGB(170,   0, 170),
	CON_RGB(  0, 170, 170),
	CON_RGB(170, 170, 170),
	CON_RGB( 85,  85,  85),
	CON_RGB(255,  85,  85),
	CON_RGB( 85, 255,  85),
	CON_RGB(255, 255,  85),
	CON_RGB( 85,  85, 255),
	CON_RGB(255,  85, 255),
	CON_RGB( 85, 255, 255),
	CON_RGB(255, 255, 255)
};

static uint32_t con_lookup_table[0x10000];

static uint32_t con_alpha_blend(int bg, int fg, int alpha)
{
	int bg_r = bg & 0xFF;
	int bg_g = (bg >> 8) & 0xFF;
	int bg_b = (bg >> 16) & 0xFF;

	int fg_r = fg & 0xFF;
	int fg_g = (fg >> 8) & 0xFF;
	int fg_b = (fg >> 16) & 0xFF;

	int r = ((alpha * fg_r) + ((255 - alpha) * bg_r)) / 255;
	int g = ((alpha * fg_g) + ((255 - alpha) * bg_g)) / 255;
	int b = ((alpha * fg_b) + ((255 - alpha) * bg_b)) / 255;

	return (uint32_t)(r | (g << 8) | (b << 16));
}

static void con_build_lookup_table(void)
{
	uint32_t *p = &con_lookup_table[0];
	int i, j, k;

	for (i = 0; i < 16; i++) {
		int bg = con_color_table[i];

		for (j = 15; j >= 0; j--) {
			int fg = con_color_table[j];

			if (bg == fg) {
				int r = fg & 0xFF;
				int g = (fg >> 8) & 0xFF;
				int b = (fg >> 16) & 0xFF;

				if (fg == 0) {
					r = 16, g = 16, b = 16;
				} else {
					r = (r * 240) / 255;
					g = (g * 240) / 255;
					b = (b * 240) / 255;
				}
				fg = (r | (g << 8) | (b << 16));
			}

			for (k = 0; k < 256; k++)
				*p++ = con_alpha_blend(bg, fg, k);
		}
	}
}

int con_init(void)
{
	static int run_once;
	size_t size;
	int i, offset;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	if (mtx_init(&con_mtx, mtx_plain) != thrd_success)
		return DE_UNEXPECTED;

	if (kernel->fb_width < 640 || kernel->fb_height < 480)
		return 0;

	if (kernel->glyph_width < 8 || kernel->glyph_height < 16)
		return 0;

	con_columns = (int)kernel->fb_width / kernel->glyph_width;
	con_rows = (int)kernel->fb_height / kernel->glyph_height;

	if (con_columns < 70 || con_rows < 25)
		return DE_UNEXPECTED;

	offset = (((int)kernel->fb_width % kernel->glyph_width) / 2);

	if (con_columns > 80) {
		offset += (kernel->glyph_width / 2);
		con_columns -= 1;
	}

	con_fb_start = (uint32_t *)kernel->fb_standard_addr + offset;
	offset = (((int)kernel->fb_height % kernel->glyph_height) / 2);
	con_fb_start += (offset * (int)kernel->fb_width);

	size = (size_t)(con_columns * con_rows) * sizeof(uint32_t);
	size = (size + 0x0FFF) & 0xFFFFF000;

	con_buffer = aligned_alloc(0x1000, size);
	if (!con_buffer)
		return DE_MEMORY;

	con_cells = con_columns * con_rows;

	for (i = 0; i < con_cells; i++)
		con_buffer[i] = 0;

	con_build_lookup_table();
	cpu_write32((uint32_t *)&con_ready, 1);

	return 0;
}

static void con_render(void)
{
	int glyph_size = kernel->glyph_width * kernel->glyph_height;
	int i = 0, j;

	while (i < con_cells) {
		uint32_t c = con_buffer[i];
		uint8_t *data = NULL;
		int underline = 0;
		volatile uint32_t *fb_ptr;
		int fb_off, x, y;
		int table_i;

		if ((c & con_rendered_bit) != 0) {
			i += 1;
			continue;
		}

		if ((c & con_reversed_bit) == 0) {
			table_i = (int)((c >> 12) & 0xFF00);
		} else {
			int t0 = 15 - (int)((c >> 20) & 15);
			int t1 = 15 - (int)((c >> 24) & 15);

			table_i = (t0 << 12) | (t1 << 8);
		}

		if ((c & con_underline_bit) != 0)
			underline = 1;

		c &= 0x000FFFFF;

		if (c == 0)
			c = 0x20;

		for (j = 0; j < kernel->glyph_count; j++) {
			int count = kernel->glyph[j].unicode_count;
			uint32_t range_start = kernel->glyph[j].unicode;
			uint32_t range_end = range_start + (uint32_t)count;

			if (c >= range_start && c < range_end) {
				data =  kernel->glyph[j].data;
				data += (glyph_size * (int)(c - range_start));
				break;
			}
		}

		if (!data) {
			if (c == 0x7F)
				break;
			con_buffer[i] = (con_buffer[i] & 0xFFF00000) | 0x7F;
			continue;
		}

		fb_ptr = (volatile uint32_t *)con_fb_start;
		fb_off = (i % con_columns) * kernel->glyph_width;
		fb_ptr += fb_off;

		fb_off = (i / con_columns) * kernel->glyph_height;
		fb_off *= (int)kernel->fb_width;
		fb_ptr += fb_off;

		for (y = 0; y < kernel->glyph_height; y++) {
			for (x = 0; x < kernel->glyph_width; x++) {
				int off = (table_i | (int)data[x]);
				uint32_t pixel = con_lookup_table[off];

				/*
				 * Avoid writing to the standard framebuffer,
				 * which is in normal memory, if the pixel
				 * value does not change. Paging is used for
				 * monitoring the modified framebuffer areas.
				 *
				 * The volatile qualifier is for suppressing
				 * compiler optimizations. Normally this kind
				 * of "if statement" would be redundant.
				 */
				if (fb_ptr[x] != pixel)
					fb_ptr[x] = pixel;
			}
			data += kernel->glyph_width;
			fb_ptr += kernel->fb_width;
		}

		if (underline) {
			int off = (table_i | 192);
			uint32_t pixel = con_lookup_table[off];

			for (y = 0; y < kernel->glyph_height / 8; y++) {
				fb_ptr -= kernel->fb_width;

				for (x = 0; x < kernel->glyph_width; x++) {
					if (fb_ptr[x] != pixel)
						fb_ptr[x] = pixel;
				}
			}
		}

		con_buffer[i] |= con_rendered_bit;
		i += 1;
	}
}

static void con_scroll_up(void)
{
	uint32_t *dst = &con_buffer[0];
	uint32_t *src = &con_buffer[con_columns];
	int i;

	for (i = con_columns; i < con_cells; i++) {
		uint32_t t0 = *dst & (~con_rendered_bit);
		uint32_t t1 = *src & (~con_rendered_bit);

		if (t0 != t1)
			*dst = t1;
		dst += 1, src += 1;
	}

	for (i = 0; i < con_columns; i++) {
		uint32_t t = *dst & (~con_rendered_bit);

		if (t != con_attribute)
			*dst = con_attribute;
		dst += 1;
	}
}

static void con_scroll_down(void)
{
	uint32_t *dst = &con_buffer[(con_rows - 1) * con_columns];
	uint32_t *src = &con_buffer[(con_rows - 2) * con_columns];
	int i;

	dst += (con_columns - 1);
	src += (con_columns - 1);

	for (i = con_columns; i < con_cells; i++) {
		uint32_t t0 = *dst & (~con_rendered_bit);
		uint32_t t1 = *src & (~con_rendered_bit);

		if (t0 != t1)
			*dst = t1;
		dst -= 1, src -= 1;
	}

	dst = &con_buffer[0];

	for (i = 0; i < con_columns; i++) {
		uint32_t t = *dst & (~con_rendered_bit);

		if (t != con_attribute)
			*dst = con_attribute;
		dst += 1;
	}
}

static void con_handle_escape(void)
{
	char *p = &con_escape_buffer[0];
	char *e = &con_escape_buffer[con_escape_size];
	int type = (con_escape_size > 1) ? (int)(*(e - 1)) : 0;
	int csi = (p[1] == '[');

	int parameters[16];
	int count = 0;
	int i;

	if (csi) {
		const int count_limit = 16;
		const int value_limit = 0x10000;

		for (p = &p[2]; p < e; p++) {
			int parameter = 0;
			int c = (int)*p;

			while (c >= '0' && c <= '9') {
				parameter *= 10;
				parameter += (c - '0');

				c = (int)*++p;

				if (parameter > value_limit)
					parameter = value_limit;
			}

			if (c != ';' && c != type)
				return;

			if (count == count_limit)
				return;

			parameters[count++] = parameter;
		}
	}

	/*
	 * CSI <n> m - Select Graphics Rendition.
	 */
	if (csi && type == 'm') {
		for (i = 0; i < count; i++) {
			int parameter = parameters[i];
			int fg_color = -1, bg_color = -1;

			switch (parameter) {
			case 0:
				con_attribute = 0;
				break;
			case 1:
				con_attribute |= con_intensity_bit;
				con_attribute &= 0xFEFFFFFF;
				break;
			case 4:
				con_attribute |= con_underline_bit;
				break;
			case 7:
				con_attribute |= con_reversed_bit;
				break;
			case 22:
				con_attribute &= (~con_intensity_bit);
				con_attribute |= 0x01000000;
				break;
			case 24:
				con_attribute &= (~con_underline_bit);
				break;
			case 27:
				con_attribute &= (~con_reversed_bit);
				break;
			case 39:
				fg_color = 15;
				break;
			case 49:
				bg_color = 0;
				break;
			default:
				break;
			}

			/*
			 * Standard foreground colors.
			 */
			if (parameter >= 30 && parameter <= 37)
				fg_color = parameter - 30;

			/*
			 * Standard background colors.
			 */
			if (parameter >= 40 && parameter <= 47)
				bg_color = parameter - 40;

			/*
			 * Bright foreground colors.
			 */
			if (parameter >= 90 && parameter <= 97)
				fg_color = parameter - 82;

			/*
			 * Bright background colors.
			 */
			if (parameter >= 100 && parameter <= 107)
				bg_color = parameter - 92;

			/*
			 * Set the foreground color.
			 */
			if (fg_color >= 0) {
				if ((con_attribute & con_intensity_bit) != 0)
					fg_color |= 8;
				fg_color = 15 - (fg_color & 15);
				con_attribute &= 0xFF0FFFFF;
				con_attribute |= (uint32_t)(fg_color << 20);
			}

			/*
			 * Set the background color.
			 */
			if (bg_color >= 0) {
				bg_color = (bg_color & 15);
				con_attribute &= 0xF0FFFFFF;
				con_attribute |= (uint32_t)(bg_color << 24);
			}

			/*
			 * Parse extended colors, but ignore them.
			 */
			if (parameter == 38 || parameter == 48) {
				if (i + 1 < count) {
					if (parameters[i + 1] == 2)
						i += 4;
					if (parameters[i + 1] == 5)
						i += 2;
				}
			}
		}
		return;
	}

	/*
	 * The rest of the function is for CSI sequences.
	 */
	if (csi == 0)
		return;

	/*
	 * Horizontal Vertical Position is the same as Cursor Position.
	 */
	if (type == 'f')
		type = 'H';

	/*
	 * Handle other CSI sequences.
	 */
	switch (type) {

	/*
	 * CSI <n> @ - Insert Character.
	 */
	case '@': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];
		uint32_t *t0, *t1;

		t0 = &con_buffer[con_column + (con_row * con_columns)];
		t1 = &con_buffer[(con_columns - 1) + (con_row * con_columns)];

		for (i = 0; i < n; i++) {
			uint32_t *t3 = t1;

			while (t0 < t3) {
				uint32_t t4 = *(t3 - 0) & (~con_rendered_bit);
				uint32_t t5 = *(t3 - 1) & (~con_rendered_bit);

				if (t4 != t5)
					*t3 = t5;
				t3 -= 1;
			}
			if ((*t0 & (~con_rendered_bit)) != con_attribute)
				*t0 = con_attribute;
		}
	} break;

	/*
	 * CSI <n> A - Cursor Up.
	 */
	case 'A': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];

		if (con_row - n >= 0)
			con_row -= n;
		else
			con_row = 0;
	} break;

	/*
	 * CSI <n> B - Cursor Down.
	 */
	case 'B': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];

		if (con_row + n < con_rows)
			con_row += n;
		else
			con_row = con_rows - 1;
	} break;

	/*
	 * CSI <n> C - Cursor Forward.
	 */
	case 'C': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];

		if (con_column + n < con_columns)
			con_column += n;
		else
			con_column = con_columns - 1;
	} break;

	/*
	 * CSI <n> D - Cursor Back.
	 */
	case 'D': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];

		if (con_column - n >= 0)
			con_column -= n;
		else
			con_column = 0;
	} break;

	/*
	 * CSI <n> E - Cursor Next Line.
	 */
	case 'E': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];

		con_column = 0;

		if (con_row + n < con_rows)
			con_row += n;
		else
			con_row = con_rows - 1;
	} break;

	/*
	 * CSI <n> F - Cursor Previous Line.
	 */
	case 'F': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];

		con_column = 0;

		if (con_row - n >= 0)
			con_row -= n;
		else
			con_row = 0;
	} break;

	/*
	 * CSI <n> G - Cursor Horizontal Absolute.
	 */
	case 'G': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];

		con_column = (n - 1 < con_columns) ? n - 1 : con_columns - 1;
	} break;

	/*
	 * CSI <n> ; <m> H - Cursor Position.
	 */
	case 'H': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];
		int m = (count < 2 || parameters[1] < 1) ? 1 : parameters[1];

		con_row = (n - 1 < con_rows) ? n - 1 : con_rows - 1;
		con_column = (m - 1 < con_columns) ? m - 1 : con_columns - 1;
	} break;

	/*
	 * CSI <n> J - Erase in Display.
	 */
	case 'J': {
		int n = (count < 1) ? 0 : parameters[0];
		int t0 = 0, t1 = 0;

		if (n == 0) {
			t0 = con_column + (con_row * con_columns);
			t1 = con_cells;

		} else if (n == 1) {
			t1 = con_column + (con_row * con_columns) + 1;

		} else if (n == 2 || n == 3) {
			t1 = con_cells;
		}

		for (i = t0; i < t1; i++) {
			con_buffer[i] &= 0x7FF00000;
			con_buffer[i] |= con_attribute;
		}
	} break;

	/*
	 * CSI <n> K - Erase in Line.
	 */
	case 'K': {
		int n = (count < 1) ? 0 : parameters[0];
		int t0 = 0, t1 = 0;

		if (n == 0) {
			t0 = con_column + (con_row * con_columns);
			t1 = con_columns + (con_row * con_columns);

		} else if (n == 1) {
			t0 = con_row * con_columns;
			t1 = con_column + (con_row * con_columns) + 1;

		} else if (n == 2) {
			t0 = con_row * con_columns;
			t1 = con_columns + (con_row * con_columns);
		}

		for (i = t0; i < t1; i++) {
			con_buffer[i] &= 0x7FF00000;
			con_buffer[i] |= con_attribute;
		}
	} break;

	/*
	 * CSI <n> P - Delete Character.
	 */
	case 'P': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];
		uint32_t *t0, *t1;

		t0 = &con_buffer[con_column + (con_row * con_columns)];
		t1 = &con_buffer[(con_columns - 1) + (con_row * con_columns)];

		for (i = 0; i < n; i++) {
			uint32_t *t3 = t0;

			while (t1 > t3) {
				uint32_t t4 = *(t3 + 0) & (~con_rendered_bit);
				uint32_t t5 = *(t3 + 1) & (~con_rendered_bit);

				if (t4 != t5)
					*t3 = t5;
				t3 += 1;
			}
			if ((*t1 & (~con_rendered_bit)) != con_attribute)
				*t1 = con_attribute;
		}
	} break;

	/*
	 * CSI <n> S - Scroll Up.
	 */
	case 'S': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];

		if (n > con_rows)
			n = con_rows;

		for (i = 0; i < n; i++)
			con_scroll_up();
	} break;

	/*
	 * CSI <n> T - Scroll Down.
	 */
	case 'T': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];

		if (n > con_rows)
			n = con_rows;

		for (i = 0; i < n; i++)
			con_scroll_down();
	} break;

	/*
	 * CSI <n> X - Erase Character.
	 */
	case 'X': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];
		uint32_t *t0, *t1;

		t0 = &con_buffer[con_column + (con_row * con_columns)];
		t1 = &con_buffer[con_columns + (con_row * con_columns)];

		for (i = 0; i < n && t0 < t1; i++, t0++) {
			if ((*t0 & (~con_rendered_bit)) != con_attribute)
				*t0 = con_attribute;
		}
	} break;

	/*
	 * Unknown CSI sequences are ignored.
	 */
	default:
		break;
	}
}

static void con_write_locked(const unsigned char *data, int size)
{
	int i, j;

	for (i = 0; i < size; i++) {
		int c = (int)data[i];
		int offset, spaces;

		if (con_escape_size) {
			char *p = &con_escape_buffer[0];

			if (con_escape_size + 2 > sizeof(con_escape_buffer)) {
				memset(p, 0, sizeof(con_escape_buffer));
				con_escape_size = 0;

			} else if ((c >= 0x20 && c <= 0x3F) || c == '[') {
				p[con_escape_size++] = (char)c;
				continue;

			} else if (c >= 0x40 && c <= 0x7E) {
				p[con_escape_size++] = (char)c;
				con_handle_escape();

				memset(p, 0, sizeof(con_escape_buffer));
				con_escape_size = 0;
				continue;
			}
		}

		switch (c) {
		case '\b':
			con_column -= ((con_column > 0) ? 1 : 0);
			break;
		case '\t':
			spaces = 8 - (con_column & 7);
			offset = con_column + (con_row * con_columns);
			for (j = 0; j < spaces; j++)
				con_buffer[offset + j] = con_attribute;
			con_column = (con_column + 8) & 0x7FFFFFF8;
			break;
		case '\n':
			con_column = 0;
			con_row += 1;
			break;
		case '\r':
			con_column = 0;
			break;
		case 0x1B:
			if (con_escape_size) {
				char *p = &con_escape_buffer[0];
				memset(p, 0, sizeof(con_escape_buffer));
				con_escape_size = 0;
			}
			con_escape_buffer[con_escape_size++] = 0x1B;
			break;
		default:
			offset = con_column + (con_row * con_columns);
			c = (c != ' ') ? c : 0;
			con_buffer[offset] = (uint32_t)c | con_attribute;
			con_column += 1;
			break;
		}

		if (con_column >= con_columns) {
			con_column = 0;
			con_row += 1;
		}

		if (con_row >= con_rows) {
			con_scroll_up();
			con_row = con_rows - 1;
		}
	}
}

void con_clear(void)
{
	uint32_t *p = (uint32_t *)kernel->fb_standard_addr;
	int i;

	if (!con_ready || mtx_lock(&con_mtx) != thrd_success)
		return;

	memset(p, 0, kernel->fb_standard_size);
	fb_render();

	for (i = 0; i < con_cells; i++)
		con_buffer[i] = con_rendered_bit;

	con_attribute = 0;
	con_escape_size = 0;

	con_column = 0;
	con_row = 0;

	mtx_unlock(&con_mtx);
}

void con_panic(const char *message)
{
	static int run_once;
	uint32_t *p = (uint32_t *)kernel->fb_standard_addr;
	const void *data = message;
	int size = (int)strlen(message);
	int i;

	if (!con_ready || !spin_trylock(&run_once))
		return;

	/*
	 * The caller is responsible for stopping other
	 * running threads of execution. Try to lock the
	 * mutex but ignore the return value. The mutex
	 * is not unlocked in this con_panic function.
	 */
	(void)mtx_trylock(&con_mtx);

	memset(p, 0, kernel->fb_standard_size);

	for (i = 0; i < con_cells; i++)
		con_buffer[i] = 0;

	con_attribute = 0;
	con_escape_size = 0;

	con_column = 0;
	con_row = 0;

	con_write_locked(data, size);
	con_render();
	fb_panic();
}

void con_print(const char *format, ...)
{
	size_t size = sizeof(con_print_buffer);
	int locked = 0, r = 0;

	va_list va;
	va_start(va, format);

	if (con_ready && mtx_lock(&con_mtx) == thrd_success) {
		r = vsnprintf(&con_print_buffer[0], size, format, va);
		locked = 1;
	}

	va_end(va);

	if (locked && r > 0) {
		const void *data = &con_print_buffer[0];

		con_write_locked(data, r);
		memset(&con_print_buffer[0], 0, (size_t)r);

		con_render();
		fb_render();
		mtx_unlock(&con_mtx);
	}
}

void con_write(const void *data, size_t size)
{
	if (!con_ready || mtx_lock(&con_mtx) != thrd_success)
		return;

	if (size <= (size_t)(INT_MAX)) {
		con_write_locked(data, (int)size);
		con_render();
		fb_render();
	}

	mtx_unlock(&con_mtx);
}
