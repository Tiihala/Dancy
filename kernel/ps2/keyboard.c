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
 * ps2/keyboard.c
 *      PS/2 Keyboard
 */

#include <dancy.h>

static int probe_state = 0;
static int kbd_ready = 0;

static const int data_none = -1;
static int data_led_state = 0x00;
static int data_scan_code = 0x02;
static int data_typematic = 0x00;

static int send_command(int command, int data)
{
	uint32_t ticks;
	int resend = 0;
	int b;

	ps2_send_port1((uint8_t)command);

	ticks = timer_ticks;

	while ((b = ps2_receive_port1()) != 0xFA) {
		if (b == 0xFE && resend < 32) {
			ps2_send_port1((uint8_t)command);
			resend += 1;
			continue;
		}
		if ((timer_ticks - ticks) >= 100)
			return 1;
	}

	if (data != data_none) {
		ps2_send_port1((uint8_t)data);

		ticks = timer_ticks;

		while ((b = ps2_receive_port1()) != 0xFA) {
			if ((timer_ticks - ticks) >= 100)
				return 2;
		}
	}

	while (ps2_receive_port1() >= 0) { /* void */ }

	return 0;
}

int ps2_kbd_init(void)
{
	/*
	 * Disable keyboard scanning.
	 */
	if (send_command(0xF5, data_none))
		return 1;

	/*
	 * Set scan code set 2 (will be translated to code set 1).
	 */
	if (send_command(0xF0, data_scan_code))
		return 1;

	/*
	 * Set keyboard LEDs.
	 */
	if (send_command(0xED, data_led_state))
		return 1;

	/*
	 * Set keyboard typematic rate and delay.
	 */
	if (send_command(0xF3, data_typematic))
		return 1;

	/*
	 * Enable keyboard scanning.
	 */
	(void)send_command(0xF4, data_none);

	kbd_ready = 1;

	return 0;
}

void ps2_kbd_handler(void)
{
	int b;

	if (probe_state != 0) {
		if (probe_state == 1)
			probe_state = 0;
		if (probe_state == 2)
			probe_state = 3;
	}

	while (kbd_ready == 0) {
		if (ps2_receive_port1() < 0)
			return;
	}

	while ((b = ps2_receive_port1()) >= 0) {

	}
}

void ps2_kbd_probe(void)
{
	/*
	 * Send an echo command to the keyboard.
	 */
	if (probe_state == 0) {
		ps2_send_port1(0xEE);
		probe_state = 1;
		return;
	}

	/*
	 * Continue sending the echo commands.
	 */
	if (probe_state <= 2) {
		ps2_send_port1(0xEE);
		probe_state = 2;
		kbd_ready = 0;
		return;
	}

	/*
	 * The echo commands were successful or some other bytes
	 * were received. Try to initialize the keyboard, but go
	 * back trying the echo commands again if this fails.
	 */
	if (ps2_kbd_init()) {
		probe_state = 2;
		return;
	}

	probe_state = 0;
}
