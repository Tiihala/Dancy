/*
 * Copyright (c) 2021, 2023, 2025 Antti Tiihala
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
 * ps2/mouse.c
 *      PS/2 Mouse
 */

#include <dancy.h>

static int probe_state = 0;

static int mse_ready = 0;
static int mse_mode = 0;

static const int data_none = -1;

static int send_command(int command, int data, int count, int *response)
{
	uint32_t ticks_timeout = (uint32_t)((command == 0xFF) ? 2000 : 100);
	uint32_t ticks;
	int resend = 0;
	int b, i;

	for (i = 0; i < count; i++)
		response[i] = 0;

	ps2_send_port2((uint8_t)command);

	ticks = timer_ticks;

	while ((b = ps2_receive_port2()) != 0xFA) {
		if (b == 0xFE && resend < 32) {
			ps2_send_port2((uint8_t)command);
			resend += 1;
			continue;
		}
		if ((timer_ticks - ticks) >= ticks_timeout)
			return 1;
	}

	if (data != data_none) {
		ps2_send_port2((uint8_t)data);
		ticks = timer_ticks;

		while ((b = ps2_receive_port2()) != 0xFA) {
			if ((timer_ticks - ticks) >= ticks_timeout)
				return 2;
		}
	}

	for (i = 0; i < count; i++) {
		ticks = timer_ticks;

		while ((b = ps2_receive_port2()) < 0) {
			if ((timer_ticks - ticks) >= ticks_timeout)
				return 3;
		}
		response[i] = b;
	}

	while (ps2_receive_port2() >= 0) { /* void */ }

	return 0;
}

int ps2_mse_init(void)
{
	int response[2];

	mse_ready = 0;
	mse_mode = 0;

	/*
	 * Try first with a faster command before doing the
	 * full initialization sequence.
	 */
	if (send_command(0xE6, data_none, 0, NULL))
		return 1;

	/*
	 * Reset the device.
	 */
	(void)send_command(0xFF, data_none, 2, &response[0]);

	/*
	 * Try to enable the "scrolling wheel" mode.
	 */
	{
		(void)send_command(0xF3, 200, 0, NULL);
		(void)send_command(0xF3, 100, 0, NULL);
		(void)send_command(0xF3, 80, 0, NULL);

		if (send_command(0xF2, data_none, 1, &response[0]))
			return 1;

		mse_mode = response[0];
	}

	/*
	 * Set the resolution.
	 */
	if (send_command(0xE8, 3, 0, NULL))
		return 1;

	/*
	 * Set the scale.
	 */
	if (send_command(0xE6, data_none, 0, NULL))
		return 1;

	/*
	 * Set the sample rate.
	 */
	if (send_command(0xF3, 40, 0, NULL))
		return 1;

	/*
	 * Enable data reporting.
	 */
	(void)send_command(0xF4, data_none, 0, NULL);

	mse_ready = 1;

	return 0;
}

void ps2_mse_handler(void)
{
	static int data = 0;
	static int state = 0;
	int b;

	while (mse_ready == 0) {
		if (ps2_receive_port2() < 0)
			return;
	}

	dancy_mse_clear();

	while ((b = ps2_receive_port2()) >= 0) {
		switch (state) {
		case 0:
			if ((b & (1 << 3)) != 0) {
				data = b;
				state = 1;
			}
			break;
		case 1:
			data |= (b << 8);
			state = 2;
			break;
		case 2:
			data |= (b << 16);
			state = (mse_mode == 3) ? 3 : 4;
			break;
		case 3:
			data |= ((b & 0x0F) << 24);
			state = 4;
			break;
		default:
			state = 0;
			break;
		}

		while (state == 4) {
			size_t size = sizeof(int);
			int r = dancy_mse_write(&size, &data);

			if (size == sizeof(int) || r != DE_RETRY)
				state = 0;
			else
				task_yield();
		}
	}

	dancy_mse_clear();
}

void ps2_mse_probe(void)
{
	int response;

	if (mse_ready == 0) {
		ps2_mse_init();
		return;
	}

	if (probe_state < 2) {
		probe_state += 1;
		return;
	}

	mse_ready = 0;

	/*
	 * Compare the device ID with the previous value.
	 */
	if (!send_command(0xF2, data_none, 1, &response)) {
		if (mse_mode == response)
			mse_ready = 1;
	}

	probe_state = 0;
}
