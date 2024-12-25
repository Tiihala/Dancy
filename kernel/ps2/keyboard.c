/*
 * Copyright (c) 2021, 2022, 2023, 2024 Antti Tiihala
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
static int kbd_state = 0;

static int response_reset = 0;
static int response_type[2] = { 0, 0 };

static const int data_none = -1;
static int data_led_state[2] = { 0x02, 0x02 };
static int data_scan_code = 0x02;
static int data_typematic = 0x00;

static struct vfs_node *kbd_pipe_nodes[2];
static struct vfs_node kbd_node;

static int keymod_lctrl = 0;
static int keymod_lshift = 0;
static int keymod_lalt = 0;
static int keymod_lgui = 0;
static int keymod_rctrl = 0;
static int keymod_rshift = 0;
static int keymod_ralt = 0;
static int keymod_rgui = 0;

static int send_command(int command, int data, int count, int *response)
{
	uint32_t ticks_timeout = (uint32_t)((command == 0xFF) ? 2000 : 100);
	uint32_t ticks;
	int resend = 0;
	int b, i;

	for (i = 0; i < count; i++)
		response[i] = 0;

	ps2_send_port1((uint8_t)command);

	ticks = timer_ticks;

	while ((b = ps2_receive_port1()) != 0xFA) {
		if (b == 0xFE && resend < 32) {
			ps2_send_port1((uint8_t)command);
			resend += 1;
			continue;
		}
		if ((timer_ticks - ticks) >= ticks_timeout)
			return 1;
	}

	if (data != data_none) {
		ps2_send_port1((uint8_t)data);
		ticks = timer_ticks;

		while ((b = ps2_receive_port1()) != 0xFA) {
			if ((timer_ticks - ticks) >= ticks_timeout)
				return 2;
		}
	}

	for (i = 0; i < count; i++) {
		ticks = timer_ticks;

		while ((b = ps2_receive_port1()) < 0) {
			if ((timer_ticks - ticks) >= ticks_timeout)
				return 3;
		}
		response[i] = b;
	}

	while (ps2_receive_port1() >= 0) { /* void */ }

	return 0;
}

static int n_read(struct vfs_node *node,
	uint64_t offset, size_t *size, void *buffer)
{
	struct vfs_node *pn = kbd_pipe_nodes[0];

	(void)node;
	(void)offset;

	if ((*size % sizeof(int)) != 0)
		return *size = 0, DE_ALIGNMENT;

	if (((size_t)((addr_t)buffer) % sizeof(int)) != 0)
		return *size = 0, DE_ALIGNMENT;

	return pn->n_read(pn, 0, size, buffer);
}

static int n_write(struct vfs_node *node,
	uint64_t offset, size_t *size, const void *buffer)
{
	(void)node;
	(void)offset;

	if ((*size % sizeof(int)) != 0)
		return *size = 0, DE_ALIGNMENT;

	if (((size_t)((addr_t)buffer) % sizeof(int)) != 0)
		return *size = 0, DE_ALIGNMENT;

	return *size = 0, DE_FULL;
}

static int n_poll(struct vfs_node *node, int events, int *revents)
{
	struct vfs_node *pn = kbd_pipe_nodes[0];
	int r;

	(void)node;
	*revents = 0;

	if ((r = pn->n_poll(pn, events, revents)) == 0) {
		if ((events & POLLOUT) != 0)
			*revents |= POLLOUT;
		if ((events & POLLWRNORM) != 0)
			*revents |= POLLWRNORM;
	}

	return r;
}

int ps2_kbd_init(void)
{
	static int create_device_node_once;

	/*
	 * Reset the keyboard device.
	 */
	(void)send_command(0xFF, data_none, 1, &response_reset);

	/*
	 * Disable keyboard scanning.
	 */
	if (send_command(0xF5, data_none, 0, NULL))
		return 1;

	/*
	 * Read the keyboard type.
	 */
	if (send_command(0xF2, data_none, 2, &response_type[0]))
		return 1;

	/*
	 * Set scan code set 2 (will be translated to code set 1).
	 */
	if (send_command(0xF0, data_scan_code, 0, NULL))
		return 1;

	/*
	 * Set keyboard LEDs.
	 */
	if (send_command(0xED, data_led_state[0], 0, NULL))
		return 1;

	/*
	 * Set keyboard typematic rate and delay.
	 */
	if (send_command(0xF3, data_typematic, 0, NULL))
		return 1;

	/*
	 * Enable keyboard scanning.
	 */
	(void)send_command(0xF4, data_none, 0, NULL);

	/*
	 * Create the device node.
	 */
	if (spin_trylock(&create_device_node_once)) {
		const char *name = "/dev/dancy-keyboard";
		struct vfs_node *node;

		if (vfs_pipe(kbd_pipe_nodes) != 0)
			return 1;

		if (vfs_open(name, &node, 0, vfs_mode_create) != 0)
			return 1;

		node->n_release(&node);

		vfs_init_node(&kbd_node, 0);
		kbd_node.type = vfs_type_character;
		kbd_node.internal_event = kbd_pipe_nodes[0]->internal_event;
		kbd_node.n_read = n_read;
		kbd_node.n_write = n_write;
		kbd_node.n_poll = n_poll;

		if (vfs_mount(name, &kbd_node) != 0)
			return 1;
	}

	kbd_ready = 1;

	return 0;
}

static int kbd_table[128] = {
	DANCY_KEY_NULL,                   /* 00 */
	DANCY_KEY_ESCAPE,                 /* 01 */
	DANCY_KEY_1,                      /* 02 */
	DANCY_KEY_2,                      /* 03 */
	DANCY_KEY_3,                      /* 04 */
	DANCY_KEY_4,                      /* 05 */
	DANCY_KEY_5,                      /* 06 */
	DANCY_KEY_6,                      /* 07 */
	DANCY_KEY_7,                      /* 08 */
	DANCY_KEY_8,                      /* 09 */
	DANCY_KEY_9,                      /* 0A */
	DANCY_KEY_0,                      /* 0B */
	DANCY_KEY_UNDERSCORE,             /* 0C */
	DANCY_KEY_EQUALS,                 /* 0D */
	DANCY_KEY_BACKSPACE,              /* 0E */
	DANCY_KEY_TAB,                    /* 0F */
	DANCY_KEY_Q,                      /* 10 */
	DANCY_KEY_W,                      /* 11 */
	DANCY_KEY_E,                      /* 12 */
	DANCY_KEY_R,                      /* 13 */
	DANCY_KEY_T,                      /* 14 */
	DANCY_KEY_Y,                      /* 15 */
	DANCY_KEY_U,                      /* 16 */
	DANCY_KEY_I,                      /* 17 */
	DANCY_KEY_O,                      /* 18 */
	DANCY_KEY_P,                      /* 19 */
	DANCY_KEY_LEFTBRACKET,            /* 1A */
	DANCY_KEY_RIGHTBRACKET,           /* 1B */
	DANCY_KEY_ENTER,                  /* 1C */
	DANCY_KEY_LCTRL,                  /* 1D */
	DANCY_KEY_A,                      /* 1E */
	DANCY_KEY_S,                      /* 1F */
	DANCY_KEY_D,                      /* 20 */
	DANCY_KEY_F,                      /* 21 */
	DANCY_KEY_G,                      /* 22 */
	DANCY_KEY_H,                      /* 23 */
	DANCY_KEY_J,                      /* 24 */
	DANCY_KEY_K,                      /* 25 */
	DANCY_KEY_L,                      /* 26 */
	DANCY_KEY_SEMICOLON,              /* 27 */
	DANCY_KEY_APOSTROPHE,             /* 28 */
	DANCY_KEY_GRAVE,                  /* 29 */
	DANCY_KEY_LSHIFT,                 /* 2A */
	DANCY_KEY_BACKSLASH,              /* 2B */
	DANCY_KEY_Z,                      /* 2C */
	DANCY_KEY_X,                      /* 2D */
	DANCY_KEY_C,                      /* 2E */
	DANCY_KEY_V,                      /* 2F */
	DANCY_KEY_B,                      /* 30 */
	DANCY_KEY_N,                      /* 31 */
	DANCY_KEY_M,                      /* 32 */
	DANCY_KEY_COMMA,                  /* 33 */
	DANCY_KEY_PERIOD,                 /* 34 */
	DANCY_KEY_SLASH,                  /* 35 */
	DANCY_KEY_RSHIFT,                 /* 36 */
	DANCY_KEY_PADASTERISK,            /* 37 */
	DANCY_KEY_LALT,                   /* 38 */
	DANCY_KEY_SPACE,                  /* 39 */
	DANCY_KEY_CAPSLOCK,               /* 3A */
	DANCY_KEY_F1,                     /* 3B */
	DANCY_KEY_F2,                     /* 3C */
	DANCY_KEY_F3,                     /* 3D */
	DANCY_KEY_F4,                     /* 3E */
	DANCY_KEY_F5,                     /* 3F */
	DANCY_KEY_F6,                     /* 40 */
	DANCY_KEY_F7,                     /* 41 */
	DANCY_KEY_F8,                     /* 42 */
	DANCY_KEY_F9,                     /* 43 */
	DANCY_KEY_F10,                    /* 44 */
	DANCY_KEY_NUMLOCK,                /* 45 */
	DANCY_KEY_SCROLLLOCK,             /* 46 */
	DANCY_KEY_PAD7,                   /* 47 */
	DANCY_KEY_PAD8,                   /* 48 */
	DANCY_KEY_PAD9,                   /* 49 */
	DANCY_KEY_PADMINUS,               /* 4A */
	DANCY_KEY_PAD4,                   /* 4B */
	DANCY_KEY_PAD5,                   /* 4C */
	DANCY_KEY_PAD6,                   /* 4D */
	DANCY_KEY_PADPLUS,                /* 4E */
	DANCY_KEY_PAD1,                   /* 4F */
	DANCY_KEY_PAD2,                   /* 50 */
	DANCY_KEY_PAD3,                   /* 51 */
	DANCY_KEY_PAD0,                   /* 52 */
	DANCY_KEY_PADDELETE,              /* 53 */
	DANCY_KEY_PRINTSCREEN,            /* 54 */
	0,                                /* 55 */
	DANCY_KEY_NONUS,                  /* 56 */
	DANCY_KEY_F11,                    /* 57 */
	DANCY_KEY_F12,                    /* 58 */
	0
};

static void process_keycode(int keycode, int release)
{
	struct vfs_node *pn = kbd_pipe_nodes[1];
	int data = keycode;
	int r;

	if (release)
		data |= DANCY_KEYTYP_RELEASE;

	switch (keycode) {
		case DANCY_KEY_LCTRL:
			keymod_lctrl = !release;
			break;
		case DANCY_KEY_LSHIFT:
			keymod_lshift = !release;
			break;
		case DANCY_KEY_LALT:
			keymod_lalt = !release;
			break;
		case DANCY_KEY_LGUI:
			keymod_lgui = !release;
			break;
		case DANCY_KEY_RCTRL:
			keymod_rctrl = !release;
			break;
		case DANCY_KEY_RSHIFT:
			keymod_rshift = !release;
			break;
		case DANCY_KEY_RALT:
			keymod_ralt = !release;
			break;
		case DANCY_KEY_RGUI:
			keymod_rgui = !release;
			break;
		case DANCY_KEY_NUMLOCK:
			data_led_state[1] ^= ((!release) << 1);
			break;
		case DANCY_KEY_CAPSLOCK:
			data_led_state[1] ^= ((!release) << 2);
			break;
		case DANCY_KEY_SCROLLLOCK:
			data_led_state[1] ^= ((!release) << 0);
			break;
		default:
			break;
	}

	if ((keymod_lctrl || keymod_rctrl) && (keymod_lalt || keymod_ralt)) {
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
			if (release)
				return;
			cpu_write32(&kernel->keyboard.console_switch_data, f);
			event_signal(kernel->keyboard.console_switch_event);
			return;
		}
	}

	if (keymod_lctrl)
		data |= DANCY_KEYMOD_LCTRL;
	if (keymod_lshift)
		data |= DANCY_KEYMOD_LSHIFT;
	if (keymod_lalt)
		data |= DANCY_KEYMOD_LALT;
	if (keymod_lgui)
		data |= DANCY_KEYMOD_LGUI;

	if (keymod_rctrl)
		data |= DANCY_KEYMOD_RCTRL;
	if (keymod_rshift)
		data |= DANCY_KEYMOD_RSHIFT;
	if (keymod_ralt)
		data |= DANCY_KEYMOD_RALT;
	if (keymod_rgui)
		data |= DANCY_KEYMOD_RGUI;

	if ((data_led_state[1] & (1 << 1)) != 0)
		data |= DANCY_KEYMOD_NUMLOCK;
	if ((data_led_state[1] & (1 << 2)) != 0)
		data |= DANCY_KEYMOD_CAPSLOCK;
	if ((data_led_state[1] & (1 << 0)) != 0)
		data |= DANCY_KEYMOD_SCROLLLOCK;

	for (;;) {
		size_t size = sizeof(int);
		void *buffer = &data;

		r = pn->n_write(pn, 0, &size, buffer);

		if (size == sizeof(int) || r != DE_RETRY)
			break;

		task_yield();
	}
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
		int keycode = 0;
		int release = 0;

		if (kbd_state == 0) {
			if (b == 0xE0) {
				kbd_state = 0xE0;
				continue;
			}
			if (b == 0xE1) {
				kbd_state = 0xE1;
				continue;
			}
			if (b >= 0x80) {
				b = b & 0x7F;
				release = 1;
			}
			keycode = kbd_table[b];

		} else if ((kbd_state & 0xFF) == 0xE0) {
			if (b >= 0x80) {
				b = b & 0x7F;
				release = 1;
			}
			switch (b) {
			case 0x1C:
				keycode = DANCY_KEY_PADENTER;
				break;
			case 0x1D:
				keycode = DANCY_KEY_RCTRL;
				break;
			case 0x35:
				keycode = DANCY_KEY_PADSLASH;
				break;
			case 0x37:
				keycode = DANCY_KEY_PRINTSCREEN;
				break;
			case 0x38:
				keycode = DANCY_KEY_RALT;
				break;
			case 0x46:
				keycode = DANCY_KEY_PAUSE;
				break;
			case 0x47:
				keycode = DANCY_KEY_HOME;
				break;
			case 0x48:
				keycode = DANCY_KEY_UPARROW;
				break;
			case 0x49:
				keycode = DANCY_KEY_PAGEUP;
				break;
			case 0x4B:
				keycode = DANCY_KEY_LEFTARROW;
				break;
			case 0x4D:
				keycode = DANCY_KEY_RIGHTARROW;
				break;
			case 0x4F:
				keycode = DANCY_KEY_END;
				break;
			case 0x50:
				keycode = DANCY_KEY_DOWNARROW;
				break;
			case 0x51:
				keycode = DANCY_KEY_PAGEDOWN;
				break;
			case 0x52:
				keycode = DANCY_KEY_INSERT;
				break;
			case 0x53:
				keycode = DANCY_KEY_DELETE;
				break;
			case 0x5B:
				keycode = DANCY_KEY_LGUI;
				break;
			case 0x5C:
				keycode = DANCY_KEY_RGUI;
				break;
			default:
				break;
			}
			kbd_state = 0;

		} else if ((kbd_state & 0xFF) == 0xE1) {
			if (b >= 0x80) {
				b = b & 0x7F;
				release = 1;
			}
			if (kbd_state == 0x00E1 && (b == 0x1D || b == 0x45)) {
				kbd_state = 0x01E1;
				continue;
			}
			if (b == 0x1D || b == 0x45)
				keycode = DANCY_KEY_PAUSE;
			kbd_state = 0;
		}

		if (keycode != 0)
			process_keycode(keycode, release);
	}

	if (data_led_state[0] != data_led_state[1]) {
		data_led_state[0] = data_led_state[1];
		send_command(0xED, data_led_state[0], 0, NULL);
	}
}

void ps2_kbd_probe(void)
{
	/*
	 * Send an echo command to the keyboard.
	 */
	if (probe_state == 0 && kbd_ready != 0) {
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
