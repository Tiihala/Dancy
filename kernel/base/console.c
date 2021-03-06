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
		volatile uint32_t *fb_ptr;
		int fb_off, x, y;
		int table_i;

		if ((c & 0x80000000) != 0) {
			i += 1;
			continue;
		}

		table_i = (int)((c >> 13) & 0xFF00);
		c &= 0x001FFFFF;

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
			con_buffer[i] = (con_buffer[i] & 0xFFE00000) | 0x7F;
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

		con_buffer[i] |= 0x80000000;
		i += 1;
	}
}

static void con_write_locked(const unsigned char *data, int size)
{
	int i, j;

	for (i = 0; i < size; i++) {
		int c = (int)data[i];
		int offset, spaces;

		switch (c) {
		case '\b':
			con_column -= ((con_column > 0) ? 1 : 0);
			break;
		case '\t':
			spaces = 8 - (con_column & 7);
			offset = con_column + (con_row * con_columns);
			for (j = 0; j < spaces; j++)
				con_buffer[offset + j] = 0x20;
			con_column = (con_column + 8) & 0x7FFFFFF8;
			break;
		case '\n':
			con_column = 0;
			con_row += 1;
			break;
		case '\r':
			con_column = 0;
			break;
		default:
			offset = con_column + (con_row * con_columns);
			con_buffer[offset] = (uint32_t)c;
			con_column += 1;
			break;
		}

		if (con_column >= con_columns) {
			con_column = 0;
			con_row += 1;
		}

		if (con_row >= con_rows) {
			uint32_t *dst = &con_buffer[0];
			uint32_t *src = &con_buffer[con_columns];

			for (j = con_columns; j < con_cells; j++) {
				uint32_t t0 = *dst & 0x7FFFFFFF;
				uint32_t t1 = *src & 0x7FFFFFFF;

				if (t0 != t1)
					*dst = t1;
				dst += 1, src += 1;
			}

			for (j = 0; j < con_columns; j++) {
				uint32_t t = *dst & 0x7FFFFFFF;

				if (t != 0)
					*dst = 0;
				dst += 1;
			}

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
		con_buffer[i] = 0x80000000;

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
