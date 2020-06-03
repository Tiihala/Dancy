/*
 * Copyright (c) 2019, 2020 Antti Tiihala
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
 * dy-png/convert.c
 *      Convert .bmp files to Dancy-compatible .png files
 */

#include "program.h"

static unsigned long raw_width;
static unsigned long raw_height;

static size_t raw_size;
static unsigned char *raw_data;

static unsigned long adler32(unsigned char *obj, size_t size)
{
	unsigned long t1 = 1, t2 = 0;
	size_t i;

	for (i = 0; i < size; i++) {
		t1 = ((t1 + (unsigned long)obj[i]) & 0xFFFFFFFFul) % 65521ul;
		t2 = ((t2 + t1) & 0xFFFFFFFFul) % 65521ul;
	}
	return ((t2 << 16) | t1) & 0xFFFFFFFFul;
}

static int get_mask_shift(unsigned long mask)
{
	if (mask == 0x00000000ul)
		return 0;
	if (mask == 0xFF000000ul)
		return 24;
	if (mask == 0x00FF0000ul)
		return 16;
	if (mask == 0x0000FF00ul)
		return 8;
	if (mask == 0x000000FFul)
		return 0;

	return 31;
}

static int validate_bmp(const unsigned char *bmp_file, size_t bmp_size)
{
	unsigned long width, height, bpp;
	unsigned long compression, row_size;
	unsigned long data_offset, data_size;

	if (bmp_size < 54 || bmp_size >= 0x10000000ul)
		return fputs("Error: bmp file size\n", stderr), 1;

	if ((int)bmp_file[0] != 'B' || (int)bmp_file[1] != 'M')
		return fputs("Error: bmp signature\n", stderr), 1;

	data_offset = LE32(&bmp_file[0x0A]);

	if (LE32(&bmp_file[0x0E]) < 40)
		return fputs("Error: unknown header size\n", stderr), 1;

	width = LE32(&bmp_file[0x12]);
	height = LE32(&bmp_file[0x16]);

	if (width > 8192 || height > 8192)
		return fputs("Error: image size overflow\n", stderr), 1;

	if (width * height == 0)
		return fputs("Error: zero-sized image\n", stderr), 1;

	if (LE16(&bmp_file[0x1A]) != 1)
		return fputs("Error: too many color planes\n", stderr), 1;

	bpp = LE16(&bmp_file[0x1C]);

	if (bpp != 24 && bpp != 32)
		return fputs("Error: unknown bpp\n", stderr), 1;

	row_size = (((bpp * width) + 31) / 32) * 4;
	data_size = row_size * height;

	if (!(data_offset < ULONG_MAX - data_size))
		return fputs("Error: image size overflow\n", stderr), 1;

	if (data_offset + data_size > bmp_size)
		return fputs("Error: image size overflow\n", stderr), 1;

	compression = LE32(&bmp_file[0x1E]);

	if (compression != 0 && compression != 3)
		return fputs("Error: unknown compression\n", stderr), 1;

	if (bpp == 24 && compression != 0)
		return fputs("Error: compression (24-bit)\n", stderr), 1;

	if (bpp == 32 && compression != 3)
		return fputs("Error: compression (32-bit)\n", stderr), 1;

	if (compression == 3 && bmp_size < 70)
		return fputs("Error: bit field masks\n", stderr), 1;

	if (compression == 3) {
		if (get_mask_shift(LE32(&bmp_file[0x36])) > 24)
			return fputs("Error: red mask\n", stderr), 1;
		if (get_mask_shift(LE32(&bmp_file[0x3A])) > 24)
			return fputs("Error: green mask\n", stderr), 1;
		if (get_mask_shift(LE32(&bmp_file[0x3E])) > 24)
			return fputs("Error: blue mask\n", stderr), 1;
		if (get_mask_shift(LE32(&bmp_file[0x42])) > 24)
			return fputs("Error: alpha mask\n", stderr), 1;
	}

	return 0;
}

static void read_bmp_pixels(const unsigned char *bmp_file)
{
	const unsigned char *s = bmp_file + LE32(&bmp_file[0x0A]);
	unsigned char *d = raw_data;

	unsigned long bpp = LE16(&bmp_file[0x1C]);
	unsigned long row_size = (((bpp * raw_width) + 31) / 32) * 4;
	unsigned long i;

	if (bpp == 24) {
		for (i = 0; i < raw_height; i++) {
			const unsigned char *row = s + (i * row_size);
			unsigned long w = raw_width;

			while (w--) {
				unsigned char blue = *row++;
				unsigned char green = *row++;
				unsigned char red = *row++;

				*d++ = red;
				*d++ = green;
				*d++ = blue;
				*d++ = 0xFF;
			}
		}

	} else if (bpp == 32) {
		unsigned long red_mask   = LE32(&bmp_file[0x36]);
		unsigned long green_mask = LE32(&bmp_file[0x3A]);
		unsigned long blue_mask  = LE32(&bmp_file[0x3E]);
		unsigned long alpha_mask = LE32(&bmp_file[0x42]);

		int red_shift   = get_mask_shift(LE32(&bmp_file[0x36]));
		int green_shift = get_mask_shift(LE32(&bmp_file[0x3A]));
		int blue_shift  = get_mask_shift(LE32(&bmp_file[0x3E]));
		int alpha_shift = get_mask_shift(LE32(&bmp_file[0x42]));

		for (i = 0; i < raw_height; i++) {
			const unsigned char *row = s + (i * row_size);
			unsigned long w = raw_width;

			while (w--) {
				unsigned long pix = LE32(&row[0]);

				unsigned long red   = pix & red_mask;
				unsigned long green = pix & green_mask;
				unsigned long blue  = pix & blue_mask;
				unsigned long alpha = pix & alpha_mask;

				*d++ = (unsigned char)(red   >> red_shift);
				*d++ = (unsigned char)(green >> green_shift);
				*d++ = (unsigned char)(blue  >> blue_shift);
				*d++ = (unsigned char)(alpha >> alpha_shift);

				if (!alpha_mask) {
					*(d - 1) = 0xFF;
				} else if (!alpha) {
					*(d - 4) = 0x00;
					*(d - 3) = 0x00;
					*(d - 2) = 0x00;
					*(d - 1) = 0x00;
				}
				row += 4;
			}
		}
	}
}

static int write_bmp_file(struct options *opt)
{
	size_t size = raw_size + 138;
	unsigned char *out;

	out = malloc(size);
	if (!out)
		return fputs("Error: not enough memory\n", stderr), 1;

	memset(out, 0, size);

	W_LE16(&out[0x00], 0x4D42ul);
	W_LE32(&out[0x02], (unsigned long)size);
	W_LE32(&out[0x0A], 0x0000008Aul);
	W_LE32(&out[0x0E], 0x0000007Cul);
	W_LE32(&out[0x12], (unsigned long)raw_width);
	W_LE32(&out[0x16], (unsigned long)raw_height);
	W_LE16(&out[0x1A], 0x0001ul);
	W_LE16(&out[0x1C], 0x0020ul);
	W_LE32(&out[0x1E], 0x00000003ul);
	W_LE32(&out[0x22], (unsigned long)raw_size);
	W_LE32(&out[0x36], 0x000000FFul);
	W_LE32(&out[0x3A], 0x0000FF00ul);
	W_LE32(&out[0x3E], 0x00FF0000ul);
	W_LE32(&out[0x42], 0xFF000000ul);
	W_LE32(&out[0x46], 0x73524742ul);
	W_LE32(&out[0x7A], 0x00000002ul);

	memcpy(&out[138], raw_data, raw_size);

	if (write_file(opt, out, size))
		return free(out), 1;

	return free(out), 0;
}

static int write_png_file(struct options *opt)
{
	size_t size = raw_size + raw_height + 4096;
	size_t row_size = (size_t)(raw_width * 4);
	unsigned long adler, crc, i, j;
	unsigned char *out, *ptr, *idat;

	out = malloc(size);
	if (!out)
		return fputs("Error: not enough memory\n", stderr), 1;

	memset(out, 0, size);

	W_BE32(&out[0x00], 0x89504E47ul);
	W_BE32(&out[0x04], 0x0D0A1A0Aul);
	W_BE32(&out[0x08], 0x0000000Dul);
	W_BE32(&out[0x0C], 0x49484452ul);
	W_BE32(&out[0x10], (unsigned long)raw_width);
	W_BE32(&out[0x14], (unsigned long)raw_height);

	if (opt->vga)
		W_BE16(&out[0x18], 0x0803ul);
	else
		W_BE16(&out[0x18], 0x0806ul);

	crc = crc32(&out[0x0C], 17);
	W_BE32(&out[0x1D], crc);

	ptr = &out[0x21];

	if (opt->vga) {
		const unsigned long colors = 16;

		W_BE32(&ptr[0], (colors * 3));
		W_BE32(&ptr[4], 0x504C5445ul);

		for (i = 0; i < colors; i++) {
			unsigned char *p = ptr + (i * 3) + 8;
			vga_color((int)i, &p[0], &p[1], &p[2]);
		}

		crc = crc32(&ptr[4], (size_t)((colors * 3) + 4));

		ptr += ((colors * 3) + 8);
		W_BE32(&ptr[0], crc);
		ptr += 4;
	}

	idat = ptr;
	W_BE32(&idat[4], 0x49444154ul);
	W_BE16(&idat[8], 0x7801ul);

	ptr = &idat[10];

	if (opt->vga) {
		for (i = raw_height; i > 0; i--) {
			unsigned char *s = raw_data + (row_size * (i - 1));

			*ptr++ = 0x00;
			for (j = 0; j < raw_width; j++) {
				unsigned c = vga_find_color(s[0], s[1], s[2]);

				*ptr++ = (unsigned char)c;
				s += 4;
			}
		}
		size = (raw_width + 1) * raw_height;
	} else {
		for (i = raw_height; i > 0; i--) {
			unsigned char *s = raw_data + (row_size * (i - 1));

			ptr[0] = 0x00;
			memcpy(&ptr[1], s, row_size);
			ptr += (row_size + 1);
		}
		size = (row_size + 1) * raw_height;
	}

	adler = adler32(&idat[10], size);

	if (deflate_compress(&idat[10], &size)) {
		fputs("Error: deflate bigger than uncompressed\n", stderr);
		return free(out), 1;
	}

	W_BE32(&idat[0], ((unsigned long)(size + 6)));

	ptr = idat + size + 10;
	W_BE32(&ptr[0], adler);

	crc = crc32(&idat[4], (size + 10));
	W_BE32(&ptr[4], crc);

	ptr += 8;
	W_BE32(&ptr[0], 0x00000000ul);
	W_BE32(&ptr[4], 0x49454E44ul);
	W_BE32(&ptr[8], 0xAE426082ul);

	ptr += 12;
	size = (size_t)(ptr - out);

	if (write_file(opt, out, size))
		return free(out), 1;

	return free(out), 0;
}

int convert(struct options *opt, unsigned char *bmp_file, size_t bmp_size)
{
	int ret;

	if (validate_bmp(bmp_file, bmp_size))
		return 1;

	raw_width = LE32(&bmp_file[0x12]);
	raw_height = LE32(&bmp_file[0x16]);

	raw_size = (size_t)(raw_width * raw_height * 4ul);
	raw_data = malloc(raw_size);

	if (!raw_data)
		return fputs("Error: not enough memory\n", stderr), 1;

	read_bmp_pixels(bmp_file);

	if (opt->bmp)
		ret = write_bmp_file(opt);
	else
		ret = write_png_file(opt);

	return free(raw_data), ret;
}
