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
 * init/rtc.c
 *      Get current time and date
 */

#include <init.h>

struct rtc_regs {
	unsigned second;
	unsigned minute;
	unsigned hour;
	unsigned day;
	unsigned month;
	unsigned year;
	unsigned century;
	unsigned status_a;
	unsigned status_b;
};

#define BCD_1(a) ((uint8_t)(((a) & 0x0Fu) + ((a) >> 4) * 10u))
#define BCD_2(a) ((uint16_t)(((a) & 0x0Fu) + ((a) >> 4) * 10u))
#define BIN_1(a) ((uint8_t)(a))
#define BIN_2(a) ((uint16_t)(a))

static int translate(struct rtc_regs *regs, struct b_time *bt)
{
	int h12 = ((regs->status_b & 2u) == 0);
	int bcd = ((regs->status_b & 4u) == 0);
	unsigned hour_flag = regs->hour & 0x80u;
	unsigned century, year;

	regs->hour &= 0x7Fu;

	if (bcd) {
		bt->month  = BCD_1(regs->month);
		bt->day    = BCD_1(regs->day);
		bt->hour   = BCD_1(regs->hour);
		bt->minute = BCD_1(regs->minute);
		bt->second = BCD_1(regs->second);
		century    = BCD_2(regs->century);
		year       = BCD_2(regs->year);
	} else {
		bt->month  = BIN_1(regs->month);
		bt->day    = BIN_1(regs->day);
		bt->hour   = BIN_1(regs->hour);
		bt->minute = BIN_1(regs->minute);
		bt->second = BIN_1(regs->second);
		century    = BIN_2(regs->century);
		year       = BIN_2(regs->year);
	}

	if (century >= 19)
		bt->year = (uint16_t)(year + (century * 100));
	else
		bt->year = (uint16_t)(year + ((year >= 80) ? 1900 : 2000));

	if (h12 && hour_flag)
		bt->hour = (uint8_t)(((int)bt->hour + 12) % 24);

	if (!(bt->month >= 1 && bt->month <= 12))
		return 1;
	if (!(bt->day >= 1 && bt->day <= 31))
		return 1;
	if (!(bt->hour <= 59))
		return 1;
	if (!(bt->minute <= 59))
		return 1;
	return 0;
}

static int compare_regs(struct rtc_regs *regs1, struct rtc_regs *regs2)
{
	if (regs1->second != regs2->second || regs1->minute != regs2->minute)
		return 1;
	if (regs1->hour != regs2->hour || regs1->day != regs2->day)
		return 1;
	if (regs1->month != regs2->month || regs1->year != regs2->year)
		return 1;
	return 0;
}

static unsigned cmos(uint8_t reg, uint32_t delay_low, uint32_t delay_high)
{
	uint8_t reg_with_nmi_enabled = (uint8_t)(reg & 0x7Fu);

	cpu_out8(0x70, reg_with_nmi_enabled);
	cpu_rdtsc_delay(delay_low, delay_high);
	return (unsigned)cpu_in8(0x71);
}

int rtc_read(struct b_time *bt)
{
	static int first_run = 1;
	struct acpi_information *acpi;
	uint32_t delay_low, delay_high;
	struct rtc_regs regs1, regs2;
	uint8_t pic1, pic2, century_idx;
	int i, j;

	/*
	 * For boot loaders other than "BIOS", use loader services.
	 */
	if (boot_loader_type != BOOT_LOADER_TYPE_BIOS)
		return b_get_time(bt) ? 1 : 0;

	memset(bt, 0, sizeof(*bt));
	memset(&regs1, 0, sizeof(regs1));
	memset(&regs2, 0, sizeof(regs2));
	century_idx = 0;

	acpi = acpi_get_information();
	if (acpi != NULL) {
		unsigned iapc_boot_arch = acpi->iapc_boot_arch;
		if (iapc_boot_arch & INIT_ARCH_CMOS_RTC_NOT_PRESENT)
			return 1;
		century_idx = (uint8_t)acpi->rtc_century_idx;
	}

	/*
	 * Disable IRQs.
	 */
	pic1 = cpu_in8(0x21);
	pic2 = cpu_in8(0xA1);
	cpu_out8(0xA1, 0xFF);
	cpu_out8(0x21, 0xFF);

	/*
	 * Calibrate IO delay.
	 */
	cpu_rdtsc(&delay_low, &delay_high);
	cpu_out8(0xA1, 0xFF), cpu_out8(0x21, 0xFF);
	cpu_rdtsc_diff(&delay_low, &delay_high);

	if (century_idx)
		regs1.century = cmos(century_idx, delay_low, delay_high);

	/*
	 * Read CMOS RTC date and time registers.
	 */
	for (i = 0; i < 8192; i++) {
		for (j = 0; j < 8192; j++) {
			regs1.status_a = cmos(0x0A, delay_low, delay_high);
			regs1.status_b = cmos(0x0B, delay_low, delay_high);
			if (!(regs1.status_a & 0x80u))
				break;
		}

		regs1.second = cmos(0x00, delay_low, delay_high);
		regs1.minute = cmos(0x02, delay_low, delay_high);
		regs1.hour   = cmos(0x04, delay_low, delay_high);
		regs1.day    = cmos(0x07, delay_low, delay_high);
		regs1.month  = cmos(0x08, delay_low, delay_high);
		regs1.year   = cmos(0x09, delay_low, delay_high);

		regs2.second = cmos(0x00, delay_low, delay_high);
		regs2.minute = cmos(0x02, delay_low, delay_high);
		regs2.hour   = cmos(0x04, delay_low, delay_high);
		regs2.day    = cmos(0x07, delay_low, delay_high);
		regs2.month  = cmos(0x08, delay_low, delay_high);
		regs2.year   = cmos(0x09, delay_low, delay_high);

		if (!compare_regs(&regs1, &regs2))
			break;
		regs1.year = UINT_MAX;
	}

	/*
	 * Enable IRQs.
	 */
	cpu_out8(0x21, pic1);
	cpu_out8(0xA1, pic2);

	/*
	 * Decipher the CMOS RTC data.
	 */
	if (regs1.year == UINT_MAX || translate(&regs1, bt))
		return memset(bt, 0, sizeof(*bt)), 1;

	if (first_run) {
		b_log("Real Time Clock (RTC)\n");
		b_log("\tTSC delay is %.0X%08X\n", delay_high, delay_low);
		b_log("\t%d-hour clock\n", (regs1.status_b & 4u) ? 12 : 24);
		b_log("\t%s mode\n",
			(regs1.status_b & 2u) ? "BCD" : "Binary");
		b_log("\t%04u-%02u-%02u %02u:%02u:%02u\n\n",
			bt->year, bt->month, bt->day,
			bt->hour, bt->minute, bt->second);
		first_run = 0;
	}
	return 0;
}
