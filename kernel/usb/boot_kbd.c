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
 * usb/boot_kbd.c
 *      HID - Boot Interface Subclass - Keyboard
 */

#include <dancy.h>

struct state {
	uint8_t report_a[8];
	uint8_t report_b[8];

	struct usb_endpoint_descriptor *in;

	int lctrl;
	int lshift;
	int lalt;
	int lgui;
	int rctrl;
	int rshift;
	int ralt;
	int rgui;

	struct {
		int keycode;
		int stop;
		uint64_t timer;
	} repeat[6];

	uint8_t leds[2];
	int idle[2];
};

static int read_report(struct vfs_node *node, struct state *state)
{
	struct dancy_usb_node *data = node->internal_data;
	struct dancy_usb_device *dev = data->dev;

	size_t size = 8;
	int r = DE_MEDIA_CHANGED;

	memcpy(&state->report_b[0], &state->report_a[0], size);

	spin_lock_yield(&dev->lock);

	if (data->port == dev->port && data->device == dev->device) {
		r = dev->u_write_endpoint(dev, state->in,
			&size, &state->report_a[0]);
	}

	spin_unlock(&dev->lock);

	if (r == 0 && size != 8)
		return DE_UNEXPECTED;

	return r;
}

static int set_idle(struct vfs_node *node,
	struct dancy_usb_driver *driver, int milliseconds)
{
	struct dancy_usb_node *data = node->internal_data;
	struct dancy_usb_device *dev = data->dev;

	int r = DE_MEDIA_CHANGED;

	spin_lock_yield(&dev->lock);

	if (data->port == dev->port && data->device == dev->device) {
		int i = (int)driver->descriptor.interface->bInterfaceNumber;

		struct usb_device_request request;
		memset(&request, 0, sizeof(request));

		request.bmRequestType = 0x21;
		request.bRequest      = 0x0A;
		request.wValue        = (uint16_t)((milliseconds / 4) << 8);
		request.wIndex        = (uint16_t)i;
		request.wLength       = 0;

		r = dev->u_write_request(dev, &request, NULL);
	}

	spin_unlock(&dev->lock);

	return r;
}

static struct usb_endpoint_descriptor *get_in(struct dancy_usb_driver *driver)
{
	int i;

	for (i = 0; i < 32; i++) {
		struct usb_endpoint_descriptor *e;

		if ((e = driver->descriptor.endpoints[i]) != NULL) {
			if ((e->bEndpointAddress & 0x80) != 0)
				return e;
		}
	}

	return NULL;
}

static void process_keycode(int keycode, int release, struct state *state)
{
	int data = keycode;

	if (release)
		data |= DANCY_KEYTYP_RELEASE;

	switch (keycode) {
		case DANCY_KEY_LCTRL:
			state->lctrl = !release;
			break;
		case DANCY_KEY_LSHIFT:
			state->lshift = !release;
			break;
		case DANCY_KEY_LALT:
			state->lalt = !release;
			break;
		case DANCY_KEY_LGUI:
			state->lgui = !release;
			break;
		case DANCY_KEY_RCTRL:
			state->rctrl = !release;
			break;
		case DANCY_KEY_RSHIFT:
			state->rshift = !release;
			break;
		case DANCY_KEY_RALT:
			state->ralt = !release;
			break;
		case DANCY_KEY_RGUI:
			state->rgui = !release;
			break;
		case DANCY_KEY_NUMLOCK:
			state->leds[1] ^= ((!release) << 1);
			break;
		case DANCY_KEY_CAPSLOCK:
			state->leds[1] ^= ((!release) << 2);
			break;
		case DANCY_KEY_SCROLLLOCK:
			state->leds[1] ^= ((!release) << 0);
			break;
		default:
			break;
	}

	if ((state->lctrl || state->rctrl) && (state->lalt || state->ralt)) {
		if (keycode == DANCY_KEY_DELETE) {
			if (release)
				return;
			cpu_write32(&kernel->keyboard.ctrl_alt_del_data, 1);
			event_signal(kernel->keyboard.ctrl_alt_del_event);
			return;
		}
		if (keycode == DANCY_KEY_PADDELETE) {
			if (release)
				return;
			cpu_write32(&kernel->keyboard.ctrl_alt_del_data, 2);
			event_signal(kernel->keyboard.ctrl_alt_del_event);
			return;
		}
		if (keycode >= DANCY_KEY_F1 && keycode <= DANCY_KEY_F12) {
			uint32_t f = (uint32_t)(keycode - DANCY_KEY_F1) + 1;
			if (release || f > 2)
				return;
			cpu_write32(&kernel->keyboard.console_switch_data, f);
			event_signal(kernel->keyboard.console_switch_event);
			return;
		}
	}

	if (state->lctrl)
		data |= DANCY_KEYMOD_LCTRL;
	if (state->lshift)
		data |= DANCY_KEYMOD_LSHIFT;
	if (state->lalt)
		data |= DANCY_KEYMOD_LALT;
	if (state->lgui)
		data |= DANCY_KEYMOD_LGUI;

	if (state->rctrl)
		data |= DANCY_KEYMOD_RCTRL;
	if (state->rshift)
		data |= DANCY_KEYMOD_RSHIFT;
	if (state->ralt)
		data |= DANCY_KEYMOD_RALT;
	if (state->rgui)
		data |= DANCY_KEYMOD_RGUI;

	if ((state->leds[1] & (1 << 1)) != 0)
		data |= DANCY_KEYMOD_NUMLOCK;
	if ((state->leds[1] & (1 << 2)) != 0)
		data |= DANCY_KEYMOD_CAPSLOCK;
	if ((state->leds[1] & (1 << 0)) != 0)
		data |= DANCY_KEYMOD_SCROLLLOCK;

	for (;;) {
		size_t size = sizeof(int);
		void *buffer = &data;

		int r = dancy_kbd_write(&size, buffer);

		if (size == sizeof(int) || r != DE_RETRY)
			break;

		task_yield();
	}
}

static void process_repeat(int keycode, int release, struct state *state)
{
	uint64_t t = timer_read();
	int i;

	if (keycode <= 0) {
		state->idle[1] = 500;

		for (i = 0; i < 6; i++) {
			if (state->repeat[i].keycode == 0)
				continue;

			state->idle[1] = 20;

			if (state->repeat[i].stop != 0)
				continue;
			if ((t - state->repeat[i].timer) < 500)
				continue;

			process_keycode(state->repeat[i].keycode, 0, state);
			break;
		}
		return;
	}

	if (release) {
		for (i = 0; i < 6; i++) {
			if (state->repeat[i].keycode != keycode)
				continue;
			state->repeat[i].keycode = 0;
			state->repeat[i].stop = 0;
			state->repeat[i].timer = 0;
		}
		return;
	}

	for (i = 0; i < 6; i++) {
		if (state->repeat[i].keycode == keycode)
			return;
		state->repeat[i].stop = 1;
	}

	for (i = 0; i < 6; i++) {
		if (state->repeat[i].keycode != 0)
			continue;

		state->repeat[i].keycode = keycode;
		state->repeat[i].stop = 0;
		state->repeat[i].timer = t;

		state->idle[1] = 20;
		break;
	}
}

void usb_boot_keyboard_driver(struct vfs_node *node,
	struct dancy_usb_driver *driver)
{
	struct usb_endpoint_descriptor *in = get_in(driver);
	struct state *state;

	if (in == NULL || (state = malloc(sizeof(*state))) == NULL)
		return;

	if (set_idle(node, driver, 500))
		return;

	printk("[USB] Boot Interface Subclass, Keyboard Driver\n");

	memset(state, 0, sizeof(*state));

	state->in = in;

	state->leds[0] = 0x02;
	state->leds[1] = 0x02;

	state->idle[0] = 500;
	state->idle[1] = 500;

	while (read_report(node, state) == 0) {
		int a, b, i, j;

		a = (int)state->report_a[0];
		b = (int)state->report_b[0];

		if ((a & 0x01) != (b & 0x01)) {
			int release = ((b & 0x01) != 0);
			process_keycode(DANCY_KEY_LCTRL, release, state);
		}

		if ((a & 0x02) != (b & 0x02)) {
			int release = ((b & 0x02) != 0);
			process_keycode(DANCY_KEY_LSHIFT, release, state);
		}

		if ((a & 0x04) != (b & 0x04)) {
			int release = ((b & 0x04) != 0);
			process_keycode(DANCY_KEY_LALT, release, state);
		}

		if ((a & 0x08) != (b & 0x08)) {
			int release = ((b & 0x08) != 0);
			process_keycode(DANCY_KEY_LGUI, release, state);
		}

		if ((a & 0x10) != (b & 0x10)) {
			int release = ((b & 0x10) != 0);
			process_keycode(DANCY_KEY_RCTRL, release, state);
		}

		if ((a & 0x20) != (b & 0x20)) {
			int release = ((b & 0x20) != 0);
			process_keycode(DANCY_KEY_RSHIFT, release, state);
		}

		if ((a & 0x40) != (b & 0x40)) {
			int release = ((b & 0x40) != 0);
			process_keycode(DANCY_KEY_RALT, release, state);
		}

		if ((a & 0x80) != (b & 0x80)) {
			int release = ((b & 0x80) != 0);
			process_keycode(DANCY_KEY_RGUI, release, state);
		}

		for (i = 2; i < 8; i++) {
			const int release = 1;
			int state_keycode = 2;

			b = (int)state->report_b[i];

			if (b < 3 || b > 127)
				state_keycode = 0;

			for (j = 2; j < 8 && state_keycode == 2; j++) {
				if (b == (int)state->report_a[j])
					state_keycode = 1;
			}

			if (state_keycode > 1) {
				process_keycode(b, release, state);
				process_repeat(b, release, state);
			}
		}

		for (i = 2; i < 8; i++) {
			const int release = 0;
			int state_keycode = 2;

			a = (int)state->report_a[i];

			if (a < 3 || a > 127)
				state_keycode = 0;

			for (j = 2; j < 8 && state_keycode == 2; j++) {
				if (a == (int)state->report_b[j])
					state_keycode = 1;
			}

			if (state_keycode > 1)
				process_keycode(a, release, state);

			if (state_keycode > 0)
				process_repeat(a, release, state);
		}

		if (state->idle[0] != state->idle[1]) {
			set_idle(node, driver, state->idle[1]);
			state->idle[0] = state->idle[1];
		}

		process_repeat(-1, 0, state);
		task_sleep(10);
	}

	printk("[USB] Boot Interface Subclass, Keyboard Driver, Exit\n");
	free(state);
}
