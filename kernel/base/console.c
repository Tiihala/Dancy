/*
 * Copyright (c) 2021, 2026 Antti Tiihala
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

struct con_state {
	int x;
	int y;

	int scroll_first;
	int scroll_last;
	int utf8_state;
	int wrap_delay;
	int cursor_visible;

	uint32_t attribute;

	size_t esc_size;
	char esc_buffer[128];

	uint32_t *buffer;
	uint32_t *buffer_main;
	uint32_t *buffer_alt;
	uint8_t *tabs_array;

	int cmd[8];
};

static struct con_state con_array[6];
static struct con_state *con;

static int con_columns;
static int con_rows;

static int con_cells;
static void *con_switch_state;

static uint32_t *con_fb_start;

static const uint32_t con_rendered_bit  = 0x80000000;
static const uint32_t con_intensity_bit = 0x40000000;
static const uint32_t con_reversed_bit  = 0x20000000;
static const uint32_t con_underline_bit = 0x10000000;

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

enum con_state_command {
	con_clear_state,
	con_save_cursor,
	con_save_main_state,
	con_restore_cursor,
	con_restore_main_state
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

static void con_handle_state(int cmd)
{
	int i;

	if (cmd == con_clear_state) {
		for (i = 0; i < 8; i++)
			con->cmd[i] = 0;

	} else if (cmd == con_save_cursor) {
		con->cmd[0] = (int)(con->attribute & 0x7FFFFFFF);
		con->cmd[1] = con->x;
		con->cmd[2] = con->y;

	} else if (cmd == con_save_main_state) {
		con->cmd[3] = (int)(con->attribute & 0x7FFFFFFF);
		con->cmd[4] = con->x;
		con->cmd[5] = con->y;
		con->cmd[6] = con->scroll_first;
		con->cmd[7] = con->scroll_last;

	} else if (cmd == con_restore_cursor) {
		con->attribute = (uint32_t)con->cmd[0];
		con->x = con->cmd[1];
		con->y = con->cmd[2];
		con->wrap_delay = 0;

	} else if (cmd == con_restore_main_state) {
		con->attribute = (uint32_t)con->cmd[3];
		con->x = con->cmd[4];
		con->y = con->cmd[5];
		con->scroll_first = con->cmd[6];
		con->scroll_last = con->cmd[7];
		con->wrap_delay = 0;
	}
}

static void con_init_variables(void)
{
	int i;

	con->attribute = 0x00800000;
	con->esc_size = 0;

	con->x = 0;
	con->y = 0;

	con->scroll_first = 0;
	con->scroll_last = con_rows - 1;

	con->utf8_state = 0;
	con->wrap_delay = 0;
	con->cursor_visible = 1;

	for (i = 0; i < con_columns; i++)
		con->tabs_array[i] = (uint8_t)(((i % 8) == 0) ? 1 : 0);

	con->tabs_array[con_columns - 1] = 1;

	con_handle_state(con_clear_state);
}

int con_init(void)
{
	static int run_once;
	size_t size;
	uint8_t *buffer;
	int i, j, offset;

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

	while (con_columns > 128) {
		offset += (kernel->glyph_width / 2);
		con_columns -= 1;
	}

	con_fb_start = (uint32_t *)kernel->fb_standard_addr + offset;
	offset = (((int)kernel->fb_height % kernel->glyph_height) / 2);
	con_fb_start += (offset * (int)kernel->fb_width);

	con_build_lookup_table();
	con_cells = con_columns * con_rows;
	con_switch_state = &con_array[0];

	for (i = 0; i < 6; i++) {
		con = &con_array[i];

		size = (size_t)(con_columns * con_rows) * sizeof(uint32_t);
		size *= 2;
		size += (size_t)con_columns * sizeof(con->tabs_array[0]);

		size = (size + 0x0FFF) & 0xFFFFF000;
		buffer = aligned_alloc(0x1000, size);

		if (!buffer)
			return DE_MEMORY;

		size = (size_t)(con_columns * con_rows) * sizeof(uint32_t);

		con->buffer_main = (void *)(buffer);
		con->buffer_alt = (void *)(buffer + size);
		con->tabs_array = (void *)(buffer + size + size);

		con->buffer = con->buffer_main;

		for (j = 0; j < con_cells; j++)
			con->buffer[j] = 0;

		con_init_variables();
	}

	con = &con_array[0];

	kernel->con_columns = con_columns;
	kernel->con_rows = con_rows;

	cpu_write32((uint32_t *)&con_ready, 1);

	return 0;
}

int con_switch(int i)
{
	int j;

	if (!con_ready)
		return 0;

	if (mtx_lock(&con_mtx) != thrd_success)
		return DE_UNEXPECTED;

	if (i >= 1 && i <= 6) {
		if (con_switch_state == (con = &con_array[i - 1]))
			return mtx_unlock(&con_mtx), 0;

		if (con_switch_state == NULL) {
			fb_enter();
			memset((void *)kernel->fb_standard_addr, 0,
				kernel->fb_standard_size);
			fb_leave();
		}

		con_switch_state = con;

		for (j = 0; j < con_cells; j++)
			con->buffer_main[j] &= (~con_rendered_bit);

		for (j = 0; j < con_cells; j++)
			con->buffer_alt[j] &= (~con_rendered_bit);

		mtx_unlock(&con_mtx);
		con_write(i, "", 0);

		return 0;
	}

	if (con_switch_state != NULL) {
		fb_enter();
		memset((void *)kernel->fb_standard_addr, 0,
			kernel->fb_standard_size);
		fb_leave();
	}

	con_switch_state = NULL;
	mtx_unlock(&con_mtx);

	return 0;
}

static void con_render(void)
{
	int glyph_size = kernel->glyph_width * kernel->glyph_height;
	volatile uint32_t *fb_ptr;
	int fb_off, x, y;
	int i = 0, j;

	while (i < con_cells) {
		uint32_t c = con->buffer[i];
		uint8_t *data = NULL;
		int underline = 0;
		int table_i;

		if ((c & con_rendered_bit) != 0) {
			i += 1;
			continue;
		}

		if ((c & con_intensity_bit) != 0)
			c &= 0xFF7FFFFF;

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
			con->buffer[i] = (con->buffer[i] & 0xFFF00000) | 0x7F;
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

		con->buffer[i] |= con_rendered_bit;
		i += 1;
	}

	if (con->cursor_visible) {
		int bg, fg = 0x00FFFFFF;
		uint32_t pixel;

		fb_ptr = (volatile uint32_t *)con_fb_start;
		fb_off = con->x * kernel->glyph_width;
		fb_ptr += fb_off;

		fb_off = con->y * kernel->glyph_height;
		fb_off *= (int)kernel->fb_width;
		fb_ptr += fb_off;

		for (y = 0; y < kernel->glyph_height; y++) {
			for (x = 0; x < kernel->glyph_width; x++) {
				bg = (int)(fb_ptr[x] & 0x00FFFFFF);
				pixel = con_alpha_blend(bg, fg, 192);
				fb_ptr[x] = pixel;
			}
			fb_ptr += kernel->fb_width;
		}
	}
}

static void con_scroll_up(void)
{
	uint32_t *dst = &con->buffer[con->scroll_first * con_columns];
	uint32_t *src = dst + con_columns;
	int movs = (con->scroll_last - con->scroll_first) * con_columns;
	int i;

	for (i = 0; i < movs; i++) {
		uint32_t t0 = *dst & (~con_rendered_bit);
		uint32_t t1 = *src & (~con_rendered_bit);

		if (t0 != t1)
			*dst = t1;
		dst += 1, src += 1;
	}

	for (i = 0; i < con_columns; i++) {
		uint32_t t = *dst & (~con_rendered_bit);

		if (t != con->attribute)
			*dst = con->attribute;
		dst += 1;
	}
}

static void con_scroll_down(void)
{
	uint32_t *dst = &con->buffer[con->scroll_last * con_columns];
	uint32_t *src = dst - con_columns;
	int movs = (con->scroll_last - con->scroll_first) * con_columns;
	int i;

	dst += (con_columns - 1);
	src += (con_columns - 1);

	for (i = 0; i < movs; i++) {
		uint32_t t0 = *dst & (~con_rendered_bit);
		uint32_t t1 = *src & (~con_rendered_bit);

		if (t0 != t1)
			*dst = t1;
		dst -= 1, src -= 1;
	}

	dst = &con->buffer[con->scroll_first * con_columns];

	for (i = 0; i < con_columns; i++) {
		uint32_t t = *dst & (~con_rendered_bit);

		if (t != con->attribute)
			*dst = con->attribute;
		dst += 1;
	}
}

static void con_handle_escape(void)
{
	char *p = &con->esc_buffer[0];
	char *e = &con->esc_buffer[con->esc_size];
	int type = (con->esc_size > 1) ? (int)(*(e - 1)) : 0;
	int csi = (p[1] == '['), csi_question = 0;

	int parameters[16];
	int count = 0;
	int i;

	if (csi) {
		const int count_limit = 16;
		const int value_limit = 0x10000;
		int c;

		if (p[2] == '?') {
			csi_question = 1;
			p += 1;
		}

		for (p = &p[2]; p < e; p++) {
			int parameter = 0;

			if ((c = (int)*p) == type)
				break;

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
		if (count == 0)
			parameters[count++] = 0;

		for (i = 0; i < count; i++) {
			int parameter = parameters[i];
			int fg_color = -1, bg_color = -1;

			switch (parameter) {
			case 0:
				con->attribute = 0x00800000;
				break;
			case 1:
				con->attribute |= con_intensity_bit;
				break;
			case 4:
				con->attribute |= con_underline_bit;
				break;
			case 7:
				con->attribute |= con_reversed_bit;
				break;
			case 22:
				con->attribute &= (~con_intensity_bit);
				break;
			case 24:
				con->attribute &= (~con_underline_bit);
				break;
			case 27:
				con->attribute &= (~con_reversed_bit);
				break;
			case 39:
				fg_color = 7;
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
				fg_color = 15 - (fg_color & 15);
				con->attribute &= 0xFF0FFFFF;
				con->attribute |= (uint32_t)(fg_color << 20);
			}

			/*
			 * Set the background color.
			 */
			if (bg_color >= 0) {
				bg_color = (bg_color & 15);
				con->attribute &= 0xF0FFFFFF;
				con->attribute |= (uint32_t)(bg_color << 24);
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
	 * Handle the plain ESC sequences.
	 */
	if (csi == 0) {
		/*
		 * ESC c - Reset.
		 */
		if (type == 'c') {
			con->buffer = con->buffer_main;

			for (i = 0; i < con_cells; i++) {
				con->buffer[i] = 0;
				con->buffer_alt[i] = 0;
			}

			con_init_variables();
			return;
		}

		/*
		 * ESC H - Horizontal Tab Set.
		 */
		if (type == 'H') {
			con->tabs_array[con->x] = 1;
			return;
		}

		/*
		 * ESC M - Reverse Index.
		 */
		if (type == 'M') {
			if (con->y == con->scroll_first)
				con_scroll_down();
			else if (con->y > 0)
				con->y -= 1;
			return;
		}

		/*
		 * ESC 7 - Save Cursor.
		 */
		if (type == '7') {
			con_handle_state(con_save_cursor);
			return;
		}

		/*
		 * ESC 8 - Restore Cursor.
		 */
		if (type == '8') {
			con_handle_state(con_restore_cursor);
			return;
		}
	}

	/*
	 * The rest of the function is for CSI sequences.
	 */
	if (csi == 0)
		return;

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

		t0 = &con->buffer[con->x + (con->y * con_columns)];
		t1 = &con->buffer[(con_columns - 1) + (con->y * con_columns)];

		for (i = 0; i < n; i++) {
			uint32_t *t3 = t1;

			while (t0 < t3) {
				uint32_t t4 = *(t3 - 0) & (~con_rendered_bit);
				uint32_t t5 = *(t3 - 1) & (~con_rendered_bit);

				if (t4 != t5)
					*t3 = t5;
				t3 -= 1;
			}
			if ((*t0 & (~con_rendered_bit)) != con->attribute)
				*t0 = con->attribute;
		}
	} break;

	/*
	 * CSI <n> A - Cursor Up.
	 */
	case 'A': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];

		con->wrap_delay = 0;

		if (con->y - n >= 0)
			con->y -= n;
		else
			con->y = 0;
	} break;

	/*
	 * CSI <n> B - Cursor Down.
	 */
	case 'B': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];

		con->wrap_delay = 0;

		if (con->y + n < con_rows)
			con->y += n;
		else
			con->y = con_rows - 1;
	} break;

	/*
	 * CSI <n> C - Cursor Forward.
	 */
	case 'C': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];

		con->wrap_delay = 0;

		if (con->x + n < con_columns)
			con->x += n;
		else
			con->x = con_columns - 1;
	} break;

	/*
	 * CSI <n> D - Cursor Back.
	 */
	case 'D': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];

		con->wrap_delay = 0;

		if (con->x - n >= 0)
			con->x -= n;
		else
			con->x = 0;
	} break;

	/*
	 * CSI <n> E - Cursor Next Line.
	 */
	case 'E': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];

		con->x = 0;
		con->wrap_delay = 0;

		if (con->y + n < con_rows)
			con->y += n;
		else
			con->y = con_rows - 1;
	} break;

	/*
	 * CSI <n> F - Cursor Previous Line.
	 */
	case 'F': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];

		con->x = 0;
		con->wrap_delay = 0;

		if (con->y - n >= 0)
			con->y -= n;
		else
			con->y = 0;
	} break;

	/*
	 * CSI <n> G - Cursor Horizontal Absolute.
	 */
	case 'G': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];

		con->x = (n - 1 < con_columns) ? n - 1 : con_columns - 1;
		con->wrap_delay = 0;
	} break;

	/*
	 * CSI <n> ; <m> H - Cursor Position.
	 */
	case 'H': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];
		int m = (count < 2 || parameters[1] < 1) ? 1 : parameters[1];

		con->y = (n - 1 < con_rows) ? n - 1 : con_rows - 1;
		con->x = (m - 1 < con_columns) ? m - 1 : con_columns - 1;
		con->wrap_delay = 0;
	} break;

	/*
	 * CSI <n> I - Cursor Forward Tabulation.
	 */
	case 'I': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];
		int t = con_columns - 1;

		for (i = con->x + 1; i < con_columns && n > 0; i++) {
			if (con->tabs_array[i] != 0)
				t = i, n -= 1;
		}
		con->x = t;
		con->wrap_delay = 0;
	} break;

	/*
	 * CSI <n> J - Erase in Display.
	 */
	case 'J': {
		int n = (count < 1) ? 0 : parameters[0];
		int t0 = 0, t1 = 0;

		if (n == 0) {
			t0 = con->x + (con->y * con_columns);
			t1 = con_cells;

		} else if (n == 1) {
			t1 = con->x + (con->y * con_columns) + 1;

		} else if (n == 2 || n == 3) {
			t1 = con_cells;
		}

		for (i = t0; i < t1; i++)
			con->buffer[i] = con->attribute;
	} break;

	/*
	 * CSI <n> K - Erase in Line.
	 */
	case 'K': {
		int n = (count < 1) ? 0 : parameters[0];
		int t0 = 0, t1 = 0;

		if (n == 0) {
			t0 = con->x + (con->y * con_columns);
			t1 = con_columns + (con->y * con_columns);

		} else if (n == 1) {
			t0 = con->y * con_columns;
			t1 = con->x + (con->y * con_columns) + 1;

		} else if (n == 2) {
			t0 = con->y * con_columns;
			t1 = con_columns + (con->y * con_columns);
		}

		for (i = t0; i < t1; i++)
			con->buffer[i] = con->attribute;
	} break;

	/*
	 * CSI <n> L - Insert Line.
	 */
	case 'L': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];
		int saved_val = con->scroll_first;

		if (n > con_rows)
			n = con_rows;

		if (con->y < con->scroll_first)
			n = 0;
		if (con->y > con->scroll_last)
			n = 0;

		con->scroll_first = con->y;

		for (i = 0; i < n; i++)
			con_scroll_down();

		con->scroll_first = saved_val;
	} break;

	/*
	 * CSI <n> M - Delete Line.
	 */
	case 'M': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];
		int saved_val = con->scroll_first;

		if (n > con_rows)
			n = con_rows;

		if (con->y < con->scroll_first)
			n = 0;
		if (con->y > con->scroll_last)
			n = 0;

		con->scroll_first = con->y;

		for (i = 0; i < n; i++)
			con_scroll_up();

		con->scroll_first = saved_val;
	} break;

	/*
	 * CSI <n> P - Delete Character.
	 */
	case 'P': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];
		uint32_t *t0, *t1;

		t0 = &con->buffer[con->x + (con->y * con_columns)];
		t1 = &con->buffer[(con_columns - 1) + (con->y * con_columns)];

		for (i = 0; i < n; i++) {
			uint32_t *t3 = t0;

			while (t1 > t3) {
				uint32_t t4 = *(t3 + 0) & (~con_rendered_bit);
				uint32_t t5 = *(t3 + 1) & (~con_rendered_bit);

				if (t4 != t5)
					*t3 = t5;
				t3 += 1;
			}
			if ((*t1 & (~con_rendered_bit)) != con->attribute)
				*t1 = con->attribute;
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

		t0 = &con->buffer[con->x + (con->y * con_columns)];
		t1 = &con->buffer[con_columns + (con->y * con_columns)];

		for (i = 0; i < n && t0 < t1; i++, t0++) {
			if ((*t0 & (~con_rendered_bit)) != con->attribute)
				*t0 = con->attribute;
		}
	} break;

	/*
	 * CSI <n> Z - Cursor Backward Tabulation.
	 */
	case 'Z': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];

		con->wrap_delay = 0;

		for (i = con->x - 1; i >= 0 && n > 0; i--) {
			if (i == 0 || con->tabs_array[i] != 0)
				con->x = i, n -= 1;
		}
	} break;

	/*
	 * CSI <n> d - Cursor Vertical Absolute.
	 */
	case 'd': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];

		con->y = (n - 1 < con_rows) ? n - 1 : con_rows - 1;
		con->wrap_delay = 0;
	} break;

	/*
	 * CSI <n> ; <m> f - Cursor Position.
	 */
	case 'f': {
		int n = (count < 1 || parameters[0] < 1) ? 1 : parameters[0];
		int m = (count < 2 || parameters[1] < 1) ? 1 : parameters[1];

		con->y = (n - 1 < con_rows) ? n - 1 : con_rows - 1;
		con->x = (m - 1 < con_columns) ? m - 1 : con_columns - 1;
		con->wrap_delay = 0;
	} break;

	/*
	 * CSI <n> g - Tab Clear.
	 */
	case 'g': {
		int n = (count < 1 || parameters[0] < 1) ? 0 : parameters[0];
		int t = con_columns - 1;

		if (n == 0) {
			if (con->x < t)
				con->tabs_array[con->x] = 0;

		} else if (n == 3) {
			for (i = 0; i < t; i++)
				con->tabs_array[i] = 0;
		}
	} break;

	/*
	 * CSI ? 25 h   - Make Cursor Visible.
	 * CSI ? 1049 h - Enable Alternate Screen Buffer.
	 */
	case 'h': {
		for (i = 0; i < count && csi_question != 0; i++) {
			int n = parameters[i];

			if (n == 25)
				con->cursor_visible = 1;

			if (n == 1049) {
				if (con->buffer == con->buffer_main)
					con_handle_state(con_save_main_state);

				con->buffer = con->buffer_alt;

				for (i = 0; i < con_cells; i++)
					con->buffer[i] = 0;
			}
		}
	} break;

	/*
	 * CSI ? 25 l   - Make Cursor Invisible.
	 * CSI ? 1049 l - Disable Alternate Screen Buffer.
	 */
	case 'l': {
		for (i = 0; i < count && csi_question != 0; i++) {
			int n = parameters[i];

			if (n == 25)
				con->cursor_visible = 0;

			if (n == 1049 && con->buffer == con->buffer_alt) {
				for (i = 0; i < con_cells; i++)
					con->buffer[i] = 0;

				con->buffer = con->buffer_main;
				con_handle_state(con_restore_main_state);

				for (i = 0; i < con_cells; i++)
					con->buffer[i] &= (~con_rendered_bit);
			}
		}
	} break;

	/*
	 * CSI <n> ; <m> r - Set Scrolling Region.
	 */
	case 'r': {
		int n = (count < 1) ? 0 : parameters[0];
		int m = (count < 2) ? 0 : parameters[1];

		n = (n >= 1) ? n : 1;
		m = (m >= 1) ? m : con_rows;

		m = (m <= con_rows) ? m : con_rows;
		n = (n <= m) ? n : m;

		con->scroll_first = n - 1;
		con->scroll_last = m - 1;
	} break;

	/*
	 * CSI s - Save Cursor.
	 */
	case 's': {
		if (count == 0)
			con_handle_state(con_save_cursor);
	} break;

	/*
	 * CSI u - Restore Cursor.
	 */
	case 'u': {
		if (count == 0)
			con_handle_state(con_restore_cursor);
	} break;

	/*
	 * Unknown CSI sequences are ignored.
	 */
	default:
		break;
	}
}

static void con_increment_row(void)
{
	con->y += 1;

	if (con->y == con->scroll_last + 1) {
		con_scroll_up();
		con->y -= 1;

	} else if (con->y >= con_rows) {
		con->y = con_rows - 1;
	}
}

static void con_write_locked(const unsigned char *data, int size)
{
	int i = 0, j;

	if (con->cursor_visible) {
		int offset = con->x + (con->y * con_columns);

		con->buffer[offset] &= (~con_rendered_bit);
	}

	while (i < size) {
		int c = (int)data[i++];
		int normal = 0;
		int offset;

		if (c == '\0')
			continue;

		c = utf8_decode(&con->utf8_state, (unsigned char)c);

		if (c == UTF8_WAITING_NEXT)
			continue;

		if (c == UTF8_ERROR_RETRY)
			i -= 1;

		if (c < 0 || c > 0xFFFFF)
			c = 0xFFFD;

		if (c == '\f' || c == '\v')
			c = '\n';

		if (c == 0x18 || c == 0x1A) {
			char *p = &con->esc_buffer[0];

			memset(p, 0, sizeof(con->esc_buffer));
			con->esc_size = 0;
			continue;
		}

		if (con->esc_size) {
			char *p = &con->esc_buffer[0];

			if (con->esc_size == 1 && (c == '7' || c == '8')) {
				p[con->esc_size++] = (char)c;
				con_handle_escape();

				p[0] = 0, p[1] = 0;
				con->esc_size = 0;
				continue;
			}

			if (con->esc_size + 2 > sizeof(con->esc_buffer)) {
				memset(p, 0, sizeof(con->esc_buffer));
				con->esc_size = 0;

			} else if ((c >= 0x20 && c <= 0x3F) || c == '[') {
				p[con->esc_size++] = (char)c;
				continue;

			} else if (c >= 0x40 && c <= 0x7E) {
				p[con->esc_size++] = (char)c;
				con_handle_escape();

				memset(p, 0, sizeof(con->esc_buffer));
				con->esc_size = 0;
				continue;
			}
		}

		switch (c) {
		case '\a':
			break;
		case '\b':
			con->x -= ((con->x > 0) ? 1 : 0);
			break;
		case '\t':
			offset = INT_MAX;
			for (j = con->x + 1; j < con_columns; j++) {
				if (con->tabs_array[j] != 0) {
					offset = j;
					break;
				}
			}
			if (offset >= con_columns) {
				con_increment_row();
				offset = 0;
			}
			con->x = offset;
			break;
		case '\n':
			con->x = 0;
			con_increment_row();
			break;
		case '\r':
			con->x = 0;
			break;
		case 0x0E:
			break;
		case 0x0F:
			break;
		case 0x1B:
			if (con->esc_size) {
				char *p = &con->esc_buffer[0];
				memset(p, 0, sizeof(con->esc_buffer));
				con->esc_size = 0;
			}
			con->esc_buffer[con->esc_size++] = 0x1B;
			break;
		case 0x7F:
			break;
		case 0x9B:
			if (con->esc_size) {
				char *p = &con->esc_buffer[0];
				memset(p, 0, sizeof(con->esc_buffer));
				con->esc_size = 0;
			}
			con->esc_buffer[con->esc_size++] = 0x1B;
			con->esc_buffer[con->esc_size++] = '[';
			break;
		default:
			c = (c != ' ') ? c : 0;
			normal = 1;
			break;
		}

		if (normal) {
			if (con->wrap_delay && con->x == con_columns - 1) {
				con->x = 0;
				con_increment_row();
			}

			offset = con->x + (con->y * con_columns);
			con->buffer[offset] = (uint32_t)c | con->attribute;
			con->x += 1;

			if (con->x >= con_columns) {
				con->x = con_columns - 1;
				con->wrap_delay = 1;
			}
		}

		if (con->x != con_columns - 1)
			con->wrap_delay = 0;
	}
}

void con_panic(const char *message)
{
	static int run_once;
	const void *data = message;
	int size = (int)strlen(message);

	if (!con_ready || !spin_trylock(&run_once))
		return;

	/*
	 * The caller is responsible for stopping other
	 * running threads of execution. Try to lock the
	 * mutex but ignore the return value. The mutex
	 * is not unlocked in this con_panic function.
	 */
	(void)mtx_trylock(&con_mtx);

	con_switch_state = (con = &con_array[0]);

	/*
	 * Reset the screen and console buffers.
	 */
	{
		uint32_t *p = (uint32_t *)kernel->fb_standard_addr;
		int i;

		memset(p, 0, kernel->fb_standard_size);

		con->buffer = con->buffer_main;

		for (i = 0; i < con_cells; i++)
			con->buffer[i] = con_rendered_bit;

		for (i = 0; i < con_cells; i++)
			con->buffer_alt[i] = 0;

		con_init_variables();
	}

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

	if (locked) {
		void *data = &con_print_buffer[0];
		void *current = con;

		con = &con_array[0];

		if (r > 0) {
			con_write_locked(data, r);
			memset(data, 0, (size_t)r);
		}

		if (con_switch_state == con) {
			fb_enter();
			con_render();
			fb_leave();
		}

		con = current;
		mtx_unlock(&con_mtx);
	}
}

void con_write(int i, const void *data, size_t size)
{
	if (i < 1 || i > 6)
		return;

	if (!con_ready || mtx_lock(&con_mtx) != thrd_success)
		return;

	if (size <= (size_t)(INT_MAX)) {
		void *current = con;

		con = &con_array[i - 1];
		con_write_locked(data, (int)size);
		con = current;

		if (con_switch_state == con) {
			fb_enter();
			con_render();
			fb_leave();
		}
	}

	mtx_unlock(&con_mtx);
}
