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
 * base/fb.c
 *      Standard framebuffer
 */

#include <dancy.h>

static mtx_t fb_mtx;
static int fb_ready;

static int fb_width;
static int fb_height;
static int fb_stride;

static void *fb_physical_ptr;
static void *fb_standard_ptr;

static int fb_page_count;

/*
 * The fb.asm files use this structure. Please
 * check them if doing any kind modifications.
 */
static struct {
	void *pte;
	void *dst_a;
	void *dst_b;
	void *dst_c;
	size_t size_a;
	size_t size_b;
	size_t size_c;
} *fb_page_array;

static void *fb_blit_buffer;
static uint8_t *fb_vga_plane0;
static uint8_t *fb_vga_plane1;
static uint8_t *fb_vga_plane2;
static uint8_t *fb_vga_plane3;

extern void fb_copy(const void *fb_page, const void *fb_blit_buffer);
extern void fb_copy_vga(void *s1, const void *s2);

static void fb_blit_vga(int page)
{
	const uint32_t *s = fb_standard_ptr;
	uint8_t *d = fb_physical_ptr;
	uint32_t b0, b1, b2, b3;
	int i, j;

	s += (page * 1024);
	d += (page * 128);

	for (i = 0, j = 0; i < 1024; i += 8, j += 1) {
		uint32_t p0 = s[i + 0];
		uint32_t p1 = s[i + 1];
		uint32_t p2 = s[i + 2];
		uint32_t p3 = s[i + 3];
		uint32_t p4 = s[i + 4];
		uint32_t p5 = s[i + 5];
		uint32_t p6 = s[i + 6];
		uint32_t p7 = s[i + 7];

		p0 = (p0 & 0xFF) + ((p0 >> 8) & 0xFF) + ((p0 >> 16) & 0xFF);
		p1 = (p1 & 0xFF) + ((p1 >> 8) & 0xFF) + ((p1 >> 16) & 0xFF);
		p2 = (p2 & 0xFF) + ((p2 >> 8) & 0xFF) + ((p2 >> 16) & 0xFF);
		p3 = (p3 & 0xFF) + ((p3 >> 8) & 0xFF) + ((p3 >> 16) & 0xFF);
		p4 = (p4 & 0xFF) + ((p4 >> 8) & 0xFF) + ((p4 >> 16) & 0xFF);
		p5 = (p5 & 0xFF) + ((p5 >> 8) & 0xFF) + ((p5 >> 16) & 0xFF);
		p6 = (p6 & 0xFF) + ((p6 >> 8) & 0xFF) + ((p6 >> 16) & 0xFF);
		p7 = (p7 & 0xFF) + ((p7 >> 8) & 0xFF) + ((p7 >> 16) & 0xFF);

		p0 = (p0 / 3) >> 4;
		p1 = (p1 / 3) >> 4;
		p2 = (p2 / 3) >> 4;
		p3 = (p3 / 3) >> 4;
		p4 = (p4 / 3) >> 4;
		p5 = (p5 / 3) >> 4;
		p6 = (p6 / 3) >> 4;
		p7 = (p7 / 3) >> 4;

		b0  = (p0 & 1) << 7;
		b0 |= (p1 & 1) << 6;
		b0 |= (p2 & 1) << 5;
		b0 |= (p3 & 1) << 4;
		b0 |= (p4 & 1) << 3;
		b0 |= (p5 & 1) << 2;
		b0 |= (p6 & 1) << 1;
		b0 |= (p7 & 1);

		b1  = (p0 & 2) << 6;
		b1 |= (p1 & 2) << 5;
		b1 |= (p2 & 2) << 4;
		b1 |= (p3 & 2) << 3;
		b1 |= (p4 & 2) << 2;
		b1 |= (p5 & 2) << 1;
		b1 |= (p6 & 2);
		b1 |= (p7 & 2) >> 1;

		b2  = (p0 & 4) << 5;
		b2 |= (p1 & 4) << 4;
		b2 |= (p2 & 4) << 3;
		b2 |= (p3 & 4) << 2;
		b2 |= (p4 & 4) << 1;
		b2 |= (p5 & 4);
		b2 |= (p6 & 4) >> 1;
		b2 |= (p7 & 4) >> 2;

		b3  = (p0 & 8) << 4;
		b3 |= (p1 & 8) << 3;
		b3 |= (p2 & 8) << 2;
		b3 |= (p3 & 8) << 1;
		b3 |= (p4 & 8);
		b3 |= (p5 & 8) >> 1;
		b3 |= (p6 & 8) >> 2;
		b3 |= (p7 & 8) >> 3;

		fb_vga_plane0[j] = (uint8_t)b0;
		fb_vga_plane1[j] = (uint8_t)b1;
		fb_vga_plane2[j] = (uint8_t)b2;
		fb_vga_plane3[j] = (uint8_t)b3;
	}

	cpu_out8(0x03C4, 0x02);
	cpu_out8(0x03C5, 0x01);

	fb_copy_vga(d, fb_vga_plane0);

	cpu_out8(0x03C4, 0x02);
	cpu_out8(0x03C5, 0x02);

	fb_copy_vga(d, fb_vga_plane1);

	cpu_out8(0x03C4, 0x02);
	cpu_out8(0x03C5, 0x04);

	fb_copy_vga(d, fb_vga_plane2);

	cpu_out8(0x03C4, 0x02);
	cpu_out8(0x03C5, 0x08);

	fb_copy_vga(d, fb_vga_plane3);
}

static void fb_blit_palette(int page)
{
	const uint8_t *s = fb_standard_ptr;
	uint8_t *d = fb_blit_buffer;
	int i, j;

	s += (page * 4096);

	for (i = 0, j = 0; i < 4096; i += 4, j += 1) {
		uint32_t r = s[i + 0];
		uint32_t g = s[i + 1];
		uint32_t b = s[i + 2];

		d[j] = (uint8_t)(((r + g + b) / 3) & 0xFC);
	}

	fb_copy(&fb_page_array[page], fb_blit_buffer);
}

static void fb_blit_15bit(int page)
{
	const uint8_t *s = fb_standard_ptr;
	uint32_t *d = fb_blit_buffer;
	int i, j;

	s += (page * 4096);

	for (i = 0, j = 0; i < 4096; i += 8, j += 1) {
		uint32_t r0 = s[i + 0] >> 3;
		uint32_t g0 = s[i + 1] >> 3;
		uint32_t b0 = s[i + 2] >> 3;

		uint32_t r1 = s[i + 4] >> 3;
		uint32_t g1 = s[i + 5] >> 3;
		uint32_t b1 = s[i + 6] >> 3;

		uint32_t p0 = ((r0 << 10) | (g0 << 5) | b0);
		uint32_t p1 = ((r1 << 10) | (g1 << 5) | b1) << 16;

		d[j] = p0 | p1;
	}

	fb_copy(&fb_page_array[page], fb_blit_buffer);
}

static void fb_blit_16bit(int page)
{
	const uint8_t *s = fb_standard_ptr;
	uint32_t *d = fb_blit_buffer;
	int i, j;

	s += (page * 4096);

	for (i = 0, j = 0; i < 4096; i += 8, j += 1) {
		uint32_t r0 = s[i + 0] >> 3;
		uint32_t g0 = s[i + 1] >> 2;
		uint32_t b0 = s[i + 2] >> 3;

		uint32_t r1 = s[i + 4] >> 3;
		uint32_t g1 = s[i + 5] >> 2;
		uint32_t b1 = s[i + 6] >> 3;

		uint32_t p0 = ((r0 << 11) | (g0 << 5) | b0);
		uint32_t p1 = ((r1 << 11) | (g1 << 5) | b1) << 16;

		d[j] = p0 | p1;
	}

	fb_copy(&fb_page_array[page], fb_blit_buffer);
}

static void fb_blit_rgb(int page)
{
	const uint8_t *s = fb_standard_ptr;
	uint8_t *d = fb_blit_buffer;
	int i, j;

	s += (page * 4096);

	for (i = 0, j = 0; i < 4096; i += 4, j += 3) {
		uint8_t r = s[i + 0];
		uint8_t g = s[i + 1];
		uint8_t b = s[i + 2];

		d[j + 0] = r;
		d[j + 1] = g;
		d[j + 2] = b;
	}

	fb_copy(&fb_page_array[page], fb_blit_buffer);
}

static void fb_blit_bgr(int page)
{
	const uint8_t *s = fb_standard_ptr;
	uint8_t *d = fb_blit_buffer;
	int i, j;

	s += (page * 4096);

	for (i = 0, j = 0; i < 4096; i += 4, j += 3) {
		uint8_t r = s[i + 0];
		uint8_t g = s[i + 1];
		uint8_t b = s[i + 2];

		d[j + 0] = b;
		d[j + 1] = g;
		d[j + 2] = r;
	}

	fb_copy(&fb_page_array[page], fb_blit_buffer);
}

static void fb_blit_rgbx(int page)
{
	const uint8_t *s = fb_standard_ptr;
	uint8_t *d = fb_blit_buffer;
	int i;

	s += (page * 4096);

	for (i = 0; i < 4096; i += 4) {
		uint8_t r = s[i + 0];
		uint8_t g = s[i + 1];
		uint8_t b = s[i + 2];

		d[i + 0] = r;
		d[i + 1] = g;
		d[i + 2] = b;
		d[i + 3] = 0;
	}

	fb_copy(&fb_page_array[page], fb_blit_buffer);
}

static void fb_blit_bgrx(int page)
{
	const uint8_t *s = fb_standard_ptr;
	uint8_t *d = fb_blit_buffer;
	int i;

	s += (page * 4096);

	for (i = 0; i < 4096; i += 4) {
		uint8_t r = s[i + 0];
		uint8_t g = s[i + 1];
		uint8_t b = s[i + 2];

		d[i + 0] = b;
		d[i + 1] = g;
		d[i + 2] = r;
		d[i + 3] = 0;
	}

	fb_copy(&fb_page_array[page], fb_blit_buffer);
}

static void (*fb_blit)(int page);

int fb_init(void)
{
	static int run_once;
	int fb_mode = (int)kernel->fb_mode;
	int pixel_size = 0;
	size_t size;
	int i;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	if (mtx_init(&fb_mtx, mtx_plain) != thrd_success)
		return DE_UNEXPECTED;

	switch (fb_mode) {
	case 0: /* FB_MODE_VGA */
		fb_vga_plane0 = (fb_blit_buffer = aligned_alloc(64, 512));
		fb_vga_plane1 = &fb_vga_plane0[128];
		fb_vga_plane2 = &fb_vga_plane1[128];
		fb_vga_plane3 = &fb_vga_plane2[128];
		fb_blit = fb_blit_vga;
		break;
	case 1: /* FB_MODE_PALETTE */
		fb_blit = fb_blit_palette;
		pixel_size = 1;
		break;
	case 2: /* FB_MODE_15_BIT */
		fb_blit = fb_blit_15bit;
		pixel_size = 2;
		break;
	case 3: /* FB_MODE_16_BIT */
		fb_blit = fb_blit_16bit;
		pixel_size = 2;
		break;
	case 4: /* FB_MODE_RED_GREEN_BLUE */
		fb_blit = fb_blit_rgb;
		pixel_size = 3;
		break;
	case 5: /* FB_MODE_BLUE_GREEN_RED */
		fb_blit = fb_blit_bgr;
		pixel_size = 3;
		break;
	case 6: /* FB_MODE_RED_GREEN_BLUE_RESERVED */
		fb_blit = fb_blit_rgbx;
		pixel_size = 4;
		break;
	case 7: /* FB_MODE_BLUE_GREEN_RED_RESERVED */
		fb_blit = fb_blit_bgrx;
		pixel_size = 4;
		break;
	default:
		return 0;
	}

	if ((size = (size_t)(1024 * pixel_size)) != 0)
		fb_blit_buffer = aligned_alloc(64, size);

	if (!fb_blit_buffer)
		return DE_MEMORY;

	fb_width = (int)kernel->fb_width;
	fb_height = (int)kernel->fb_height;
	fb_stride = (int)kernel->fb_stride;

	fb_physical_ptr = (void *)kernel->fb_addr;
	fb_standard_ptr = (void *)kernel->fb_standard_addr;

	if (fb_width < 640 || fb_height < 480)
		return 0;

	size = kernel->fb_standard_size;

	if ((size & 0x0FFF) != 0)
		return DE_UNEXPECTED;
	if (size < (size_t)(fb_width * fb_height * pixel_size))
		return DE_UNEXPECTED;

	fb_page_count = (int)kernel->fb_standard_size / 0x1000;
	size = (size_t)fb_page_count * sizeof(fb_page_array[0]);

	fb_page_array = malloc(size);
	if (!fb_page_array)
		return DE_MEMORY;

	memset(fb_page_array, 0, size);

	for (i = 0; i < fb_page_count; i++) {
		const uint8_t *ptr = fb_standard_ptr;
		void *pte = pg_get_entry(&ptr[i * 0x1000]);

		if (!pte)
			return DE_UNEXPECTED;

		fb_page_array[i].pte = pte;
	}

	if (pixel_size) {
		uint8_t *dst = fb_physical_ptr;
		size_t row_size = (size_t)(fb_width * pixel_size);
		int unit = 1024 * pixel_size;
		int size_diff, size_total = 0;
		size_t size_a, size_b, size_c;

		for (i = 0; i < fb_page_count; i++) {
			int t0 = 1024 * i;
			int t1 = t0 % fb_width;
			int t2 = fb_width - t1;
			int t3 = ((t0 / fb_width) * fb_stride);
			int t4 = fb_stride;

			fb_page_array[i].dst_a = &dst[t3 + (t1 * pixel_size)];
			fb_page_array[i].dst_b = NULL;
			fb_page_array[i].dst_c = NULL;

			size_a = (size_t)unit;
			size_b = 0;
			size_c = 0;

			if (t2 < 1024) {
				size_b = (size_t)(unit - (t2 * pixel_size));
				size_a = (size_t)(t2 * pixel_size);
				fb_page_array[i].dst_b = &dst[t3 + t4];
			}

			if (size_b > row_size) {
				size_c = size_b - row_size;
				size_b = row_size;
				fb_page_array[i].dst_c = &dst[t3 + (2 * t4)];
			}

			fb_page_array[i].size_a = size_a;
			fb_page_array[i].size_b = size_b;
			fb_page_array[i].size_c = size_c;

			if ((size_a + size_b + size_c) != (size_t)unit)
				return DE_UNEXPECTED;

			size_total += unit;
		}

		size_diff = (fb_width * fb_height * pixel_size) - size_total;

		if (size_diff < 0) {
			i = fb_page_count - 1;

			size = (size_t)(-size_diff);

			if (fb_page_array[i].size_c <= size) {
				size -= fb_page_array[i].size_c;
				fb_page_array[i].size_c = 0;
				fb_page_array[i].dst_c = NULL;
			} else {
				fb_page_array[i].size_c -= size;
				size = 0;
			}

			if (fb_page_array[i].size_b <= size) {
				size -= fb_page_array[i].size_b;
				fb_page_array[i].size_b = 0;
				fb_page_array[i].dst_b = NULL;
			} else {
				fb_page_array[i].size_b -= size;
				size = 0;
			}

			if (fb_page_array[i].size_a <= size)
				return DE_UNEXPECTED;

			fb_page_array[i].size_a -= size;
		}
	}

	cpu_write32((uint32_t *)&fb_ready, 1);

	return 0;
}

void fb_panic(void)
{
	int i;

	if (!fb_ready)
		return;

	for (i = 0; i < fb_page_count; i++)
		fb_blit(i);
}

void fb_render(void)
{
	const uint32_t d_bit = 0x40;
	int i;

	if (!fb_ready)
		return;

	if (mtx_lock(&fb_mtx) != thrd_success)
		return;

	for (i = 0; i < fb_page_count; i++) {
		uint32_t *p = (uint32_t *)fb_page_array[i].pte;
		uint32_t val = *p;

		if ((val & d_bit) != 0) {
			addr_t addr = (addr_t)(val & 0xFFFFF000);

			*p = val & (~d_bit);
			cpu_invlpg((const void *)addr);
			fb_blit(i);
		}
	}

	mtx_unlock(&fb_mtx);
}
