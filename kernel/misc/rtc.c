/*
 * Copyright (c) 2021, 2022 Antti Tiihala
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
 * misc/rtc.c
 *      Real-time clock
 */

#include <dancy.h>

static mtx_t rtc_mtx;

struct rtc_regs {
	unsigned int second;
	unsigned int minute;
	unsigned int hour;
	unsigned int day;
	unsigned int month;
	unsigned int year;
	unsigned int century;
	unsigned int status_a;
	unsigned int status_b;
};

#define BCD_1(a) ((uint8_t)(((a) & 0x0Fu) + ((a) >> 4) * 10u))
#define BCD_2(a) ((uint16_t)(((a) & 0x0Fu) + ((a) >> 4) * 10u))
#define BIN_1(a) ((uint8_t)(a))
#define BIN_2(a) ((uint16_t)(a))

static int translate(struct rtc_regs *regs, struct rtc_time *rt)
{
	int h12 = ((regs->status_b & 2u) == 0);
	int bcd = ((regs->status_b & 4u) == 0);
	unsigned int hour_flag = regs->hour & 0x80u;
	unsigned int century, year;

	regs->hour &= 0x7Fu;

	if (bcd) {
		rt->month  = BCD_1(regs->month);
		rt->day    = BCD_1(regs->day);
		rt->hour   = BCD_1(regs->hour);
		rt->minute = BCD_1(regs->minute);
		rt->second = BCD_1(regs->second);
		century    = BCD_2(regs->century);
		year       = BCD_2(regs->year);
	} else {
		rt->month  = BIN_1(regs->month);
		rt->day    = BIN_1(regs->day);
		rt->hour   = BIN_1(regs->hour);
		rt->minute = BIN_1(regs->minute);
		rt->second = BIN_1(regs->second);
		century    = BIN_2(regs->century);
		year       = BIN_2(regs->year);
	}

	if (century >= 19)
		rt->year = (uint16_t)(year + (century * 100));
	else
		rt->year = (uint16_t)(year + ((year >= 80) ? 1900 : 2000));

	if (h12 && hour_flag)
		rt->hour = (uint8_t)(((int)rt->hour + 12) % 24);

	if (!(rt->month >= 1 && rt->month <= 12))
		return 1;
	if (!(rt->day >= 1 && rt->day <= 31))
		return 1;
	if (!(rt->hour <= 59))
		return 1;
	if (!(rt->minute <= 59))
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

static unsigned int cmos(int reg)
{
	uint8_t reg_with_nmi_enabled = (uint8_t)(reg & 0x7F);

	cpu_out8(0x70, reg_with_nmi_enabled);
	delay(1000);

	return (unsigned int)cpu_in8(0x71);
}

int rtc_init(void)
{
	static int run_once;

	if (!spin_trylock(&run_once))
		return DE_UNEXPECTED;

	if (mtx_init(&rtc_mtx, mtx_plain) != thrd_success)
		return DE_UNEXPECTED;

	return 0;
}

int rtc_read(struct rtc_time *rt)
{
	struct rtc_regs regs1, regs2;
	int i, j;

	memset(rt, 0, sizeof(*rt));
	memset(&regs1, 0, sizeof(regs1));
	memset(&regs2, 0, sizeof(regs2));

	if (mtx_lock(&rtc_mtx) != thrd_success)
		return DE_UNEXPECTED;

	if (kernel->acpi && kernel->acpi->rtc_century_idx)
		regs1.century = cmos((int)kernel->acpi->rtc_century_idx);

	task_switch_disable();

	/*
	 * Read CMOS RTC date and time registers.
	 */
	for (i = 0; i < 8192; i++) {
		for (j = 0; j < 8192; j++) {
			regs1.status_a = cmos(0x0A);
			regs1.status_b = cmos(0x0B);
			if (!(regs1.status_a & 0x80u))
				break;
		}

		regs1.second = cmos(0x00);
		regs1.minute = cmos(0x02);
		regs1.hour   = cmos(0x04);
		regs1.day    = cmos(0x07);
		regs1.month  = cmos(0x08);
		regs1.year   = cmos(0x09);

		regs2.second = cmos(0x00);
		regs2.minute = cmos(0x02);
		regs2.hour   = cmos(0x04);
		regs2.day    = cmos(0x07);
		regs2.month  = cmos(0x08);
		regs2.year   = cmos(0x09);

		if (!compare_regs(&regs1, &regs2))
			break;
		regs1.year = UINT_MAX;
	}

	task_switch_enable();
	mtx_unlock(&rtc_mtx);

	/*
	 * Decipher the CMOS RTC data.
	 */
	if (regs1.year == UINT_MAX || translate(&regs1, rt))
		return memset(rt, 0, sizeof(*rt)), DE_UNEXPECTED;

	return 0;
}
