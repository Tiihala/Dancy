/*
 * Copyright (c) 2025 Antti Tiihala
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
 * lsusb/operate.c
 *      List USB devices
 */

#include "main.h"

struct lsusb_record {
	char path[4096];
	int bus;
	int port;
	int device;
};

#define B8(a,b,c) (((unsigned int)((a)[(b)]) & 0xFF) << (c))
#define LE16(a) (B8((a),0,0) | B8((a),1,8))

static void *find_descriptor(void *buffer, size_t size, int type, int i)
{
	unsigned char *p = buffer;
	size_t offset = 0;
	int count = 0;

	while (offset + 2 < size) {
		int bLength = (int)p[offset + 0];
		int bDescriptorType = (int)p[offset + 1];

		if (bLength == 0)
			break;

		if (bDescriptorType == type) {
			if (count == i) {
				if (offset + (size_t)bLength > size)
					return NULL;
				return &p[offset + 0];
			}
			count += 1;
		}

		offset += (size_t)bLength;
	}

	return NULL;
}

static void print_string_descriptor(void *buffer)
{
	unsigned char *p = buffer;
	int i, bLength = (int)p[0];

	for (i = 2; i < bLength; i += 2) {
		int c0 = (int)p[i + 0];
		int c1 = (int)p[i + 1];

		if (c0 == 0x00 && c1 == 0x00)
			continue;

		if (c0 >= 0x20 && c0 <= 0x7E && c1 == 0x00)
			printf("%c", c0);
		else
			printf("?");
	}
}

static int print_device(struct options *opt, struct lsusb_record *r)
{
	const char *path = &r->path[0];
	unsigned char buffer[4096], *p, *s;
	int fd = open(path, O_RDONLY);
	ssize_t size;

	if (fd < 0) {
		fprintf(stderr, "lsusb: \'%s\': %s\n", path, strerror(errno));
		return EXIT_FAILURE;
	}

	if ((size = read(fd, &buffer[0], sizeof(buffer))) < 0) {
		fprintf(stderr, "lsusb: \'%s\': %s\n", path, strerror(errno));
		return close(fd), EXIT_FAILURE;
	}

	close(fd);

	if ((p = find_descriptor(&buffer[0], (size_t)size, 1, 0)) != NULL) {
		int bLength = (int)p[0];

		if (bLength < 18) {
			fprintf(stderr, "lsusb: descriptor format error\n");
			return EXIT_FAILURE;
		}

		printf("Bus %02d Port %03d Device %06d: ID %04X:%04X",
			r->bus, r->port, r->device,
			LE16(&p[8]), LE16(&p[10]));

		if (p[15] > 0) {
			s = find_descriptor(&buffer[0],
				(size_t)size, 3, (int)p[15]);

			if (s != NULL) {
				printf(" ");
				print_string_descriptor(s);
			}
		}

		printf("\n");

		if (opt->verbose) {
			printf("Device Descriptor:\n");

			printf("  bLength            %6d\n",      (int)p[0]);
			printf("  bDescriptorType    %6d\n",      (int)p[1]);

			printf("  bcdUSB          %6d.%02d\n",
				(int)p[3], (int)p[2]);

			printf("  bDeviceClass       %6d\n",      (int)p[4]);
			printf("  bDeviceSubClass    %6d\n",      (int)p[5]);
			printf("  bDeviceProtocol    %6d\n",      (int)p[6]);
			printf("  bMaxPacketSize0    %6d\n",      (int)p[7]);

			printf("  idVendor           0x%04X\n", LE16(&p[8]));
			printf("  idProduct          0x%04X\n", LE16(&p[10]));

			printf("  bcdDevice       %6d.%02d\n",
				(int)p[13], (int)p[12]);

			printf("  iManufacturer      %6d", (int)p[14]);
			if (p[14] > 0) {
				s = find_descriptor(&buffer[0],
					(size_t)size, 3, (int)p[14]);

				if (s != NULL) {
					printf(" ");
					print_string_descriptor(s);
				}
			}
			printf("\n");

			printf("  iProduct           %6d", (int)p[15]);
			if (p[15] > 0) {
				s = find_descriptor(&buffer[0],
					(size_t)size, 3, (int)p[15]);

				if (s != NULL) {
					printf(" ");
					print_string_descriptor(s);
				}
			}
			printf("\n");

			printf("  iSerialNumber      %6d", (int)p[16]);
			if (p[16] > 0) {
				s = find_descriptor(&buffer[0],
					(size_t)size, 3, (int)p[16]);

				if (s != NULL) {
					printf(" ");
					print_string_descriptor(s);
				}
			}
			printf("\n");

			printf("  bNumConfigurations %6d\n", (int)p[17]);
			printf("\n");
		}
	}

	if (!opt->verbose)
		return 0;

	if ((p = find_descriptor(&buffer[0], (size_t)size, 2, 0)) != NULL) {
		int bLength = (int)p[0];

		if (bLength < 9)
			return 0;

		printf("  Configuration Descriptor:\n");

		printf("    bLength             %6d\n",      (int)p[0]);
		printf("    bDescriptorType     %6d\n",      (int)p[1]);
		printf("    wTotalLength        0x%04X\n",  LE16(&p[2]));
		printf("    bNumInterfaces      %6d\n",      (int)p[4]);
		printf("    bConfigurationValue %6d\n",      (int)p[5]);
		printf("    iConfiguration      %6d\n",      (int)p[6]);
		printf("    bmAttributes          0x%2X\n",  (int)p[7]);
		printf("    bMaxPower           %6d\n",      (int)p[8]);

		printf("\n");
	}

	return 0;
}

static int get_number(const char *s, int count)
{
	int i, ret = 0;

	for (i = 0; i < count; i++) {
		char c = s[i];

		if (c < '0' || c > '9')
			return -1;

		ret *= 10;
		ret += (int)(c - '0');
	}

	return ret;
}

static int lsusb(struct options *opt, struct lsusb_record *r, int recursion)
{
	const char *path = &r->path[0];
	struct stat status;
	int ret = 0;
	DIR *dir;

	if (recursion > 8)
		return ret;

	if ((lstat(path, &status)) != 0) {
		fprintf(stderr, "lsusb: \'%s\': %s\n", path, strerror(errno));
		return EXIT_FAILURE;
	}

	if (!S_ISDIR(status.st_mode))
		return print_device(opt, r);

	if ((dir = opendir(path)) == NULL) {
		fprintf(stderr, "lsusb: \'%s\': %s\n", path, strerror(errno));
		return EXIT_FAILURE;
	}

	if (opt->verbose)
		printf("\n");

	for (;;) {
		struct lsusb_record record;
		struct dirent *d;

		memset(&record, 0, sizeof(record));

		errno = 0;
		d = readdir(dir);

		if (d == NULL && errno == 0)
			break;

		if (d == NULL) {
			fprintf(stderr, "lsusb: \'%s\': %s\n",
				path, strerror(errno));
			return EXIT_FAILURE;
		}

		/*
		 * Example: "port-001-device-000001"
		 */
		if (strlen(&d->d_name[0]) != 22)
			continue;

		if (strncmp(&d->d_name[0], "port-", 5))
			continue;

		if (strncmp(&d->d_name[8], "-device-", 8))
			continue;

		record.bus = r->bus;
		record.port = get_number(&d->d_name[5], 3);

		if (record.port < 0 || record.port > 999)
			continue;

		record.device = get_number(&d->d_name[16], 6);

		if (record.device < 0 || record.device > 999999)
			continue;

		strcpy(&record.path[0], path);
		strcat(&record.path[0], "/");
		strcat(&record.path[0], &d->d_name[0]);

		ret |= lsusb(opt, &record, recursion + 1);
	}

	closedir(dir);

	return ret;
}

int operate(struct options *opt)
{
	DIR *dir;
	int ret = 0;

	if (opt->operands[0] != NULL)
		return opt->error = "operands not supported", EXIT_FAILURE;

	if ((dir = opendir("/dev")) == NULL) {
		perror("lsusb");
		return EXIT_FAILURE;
	}

	for (;;) {
		struct lsusb_record record;
		struct dirent *d;

		memset(&record, 0, sizeof(record));

		errno = 0;
		d = readdir(dir);

		if (d == NULL && errno == 0)
			break;

		if (d == NULL) {
			fprintf(stderr, "lsusb: \'%s\': %s\n",
				"/dev", strerror(errno));
			return EXIT_FAILURE;
		}

		/*
		 * Example: "dancy-usb-host-01"
		 */
		if (strlen(&d->d_name[0]) != 17)
			continue;

		if (strncmp(&d->d_name[0], "dancy-usb-host-", 15))
			continue;

		record.bus = get_number(&d->d_name[15], 2);

		if (record.bus < 1 || record.bus > 99)
			continue;

		strcpy(&record.path[0], "/dev/");
		strcat(&record.path[0], &d->d_name[0]);
		strcat(&record.path[0], "/");

		ret |= lsusb(opt, &record, 0);
	}

	closedir(dir);

	return ret;
}
