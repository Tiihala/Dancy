/*
 * Copyright (c) 2023 Antti Tiihala
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
 * terminal/operate.c
 *      The terminal program
 */

#include "main.h"

static int fd_amaster;
static int fd_aslave;
static int fd_keyboard;

static const struct __dancy_keymap *keymap;

static void handle_key(int key)
{
	const char *command = NULL;
	static int state;

	if ((key & __DANCY_KEYTYP_RELEASE) != 0)
		return;

	switch (key & 0xFF) {
		case __DANCY_KEY_ENTER:
			command = "\r";
			break;
		case __DANCY_KEY_ESCAPE:
			command = "\033";
			break;
		case __DANCY_KEY_F1:
			command = "\033[[A";
			break;
		case __DANCY_KEY_F2:
			command = "\033[[B";
			break;
		case __DANCY_KEY_F3:
			command = "\033[[C";
			break;
		case __DANCY_KEY_F4:
			command = "\033[[D";
			break;
		case __DANCY_KEY_F5:
			command = "\033[[E";
			break;
		case __DANCY_KEY_F6:
			command = "\033[17~";
			break;
		case __DANCY_KEY_F7:
			command = "\033[18~";
			break;
		case __DANCY_KEY_F8:
			command = "\033[19~";
			break;
		case __DANCY_KEY_F9:
			command = "\033[20~";
			break;
		case __DANCY_KEY_F10:
			command = "\033[21~";
			break;
		case __DANCY_KEY_F11:
			command = "\033[23~";
			break;
		case __DANCY_KEY_F12:
			command = "\033[24~";
			break;
		case __DANCY_KEY_UPARROW:
			command = "\033[A";
			break;
		case __DANCY_KEY_DOWNARROW:
			command = "\033[B";
			break;
		case __DANCY_KEY_RIGHTARROW:
			command = "\033[C";
			break;
		case __DANCY_KEY_LEFTARROW:
			command = "\033[D";
			break;
		case __DANCY_KEY_HOME:
			command = "\033[H";
			break;
		case __DANCY_KEY_INSERT:
			command = "\033[2~";
			break;
		case __DANCY_KEY_DELETE:
			command = "\033[3~";
			break;
		case __DANCY_KEY_END:
			command = "\033[F";
			break;
		case __DANCY_KEY_PAGEUP:
			command = "\033[5~";
			break;
		case __DANCY_KEY_PAGEDOWN:
			command = "\033[6~";
			break;
	}

	if (command == NULL && (key & __DANCY_KEYMOD_NUMLOCK) == 0) {
		switch (key & 0xFF) {
			case __DANCY_KEY_PAD1:
				command = "\033[F";
				break;
			case __DANCY_KEY_PAD2:
				command = "\033[B";
				break;
			case __DANCY_KEY_PAD3:
				command = "\033[6~";
				break;
			case __DANCY_KEY_PAD4:
				command = "\033[D";
				break;
			case __DANCY_KEY_PAD6:
				command = "\033[C";
				break;
			case __DANCY_KEY_PAD7:
				command = "\033[H";
				break;
			case __DANCY_KEY_PAD8:
				command = "\033[A";
				break;
			case __DANCY_KEY_PAD9:
				command = "\033[5~";
				break;
			case __DANCY_KEY_PAD0:
				command = "\033[2~";
				break;
			case __DANCY_KEY_PADDELETE:
				command = "\033[3~";
				break;
		}
	}

	if (command != NULL) {
		write(fd_amaster, command, strlen(command));
		return;
	}

	__dancy_keymap_write(fd_amaster, key, keymap, &state);
}

int operate(struct options *opt)
{
	const char *exe_path = opt->operands[0];
	struct termios termios;
	char pty_name[128];
	int r = 0;
	pid_t pid;

	if (isatty(0) || isatty(1) || isatty(2)) {
		fputs("Error: only single terminal allowed\n", stderr);
		return EXIT_FAILURE;
	}

	fd_keyboard = open("/dev/dancy-keyboard-1", O_RDONLY | O_NONBLOCK);

	if (fd_keyboard < 0) {
		perror("/dev/dancy-keyboard-1");
		return EXIT_FAILURE;
	}

	keymap = select_keymap(fd_keyboard);

	if (openpty(&fd_amaster, &fd_aslave, &pty_name[0], NULL, NULL)) {
		perror("openpty");
		close(fd_keyboard);
		return EXIT_FAILURE;
	}

	if (!tcgetattr(fd_amaster, &termios))
		tcsetattr(fd_amaster, TCSANOW, &termios);

	if (exe_path) {
		const char *p = &pty_name[0];
		posix_spawn_file_actions_t actions;
		posix_spawnattr_t attr;

		posix_spawn_file_actions_init(&actions);
		posix_spawnattr_init(&attr);

		posix_spawn_file_actions_addclose(&actions, fd_keyboard);
		posix_spawn_file_actions_addclose(&actions, fd_amaster);
		posix_spawn_file_actions_addclose(&actions, fd_aslave);

		posix_spawn_file_actions_addopen(&actions, 0, p, O_RDWR, 0);
		posix_spawn_file_actions_addopen(&actions, 1, p, O_RDWR, 0);
		posix_spawn_file_actions_addopen(&actions, 2, p, O_RDWR, 0);
		posix_spawn_file_actions_addtcsetpgrp_np(&actions, 0);

		posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSID);

		if (r == 0) {
			r = posix_spawn(&pid, exe_path,
				&actions, &attr, opt->operands, environ);
		}

		if (r != 0) {
			errno = r;
			perror("posix_spawn");
			exe_path = NULL;
		}

		posix_spawn_file_actions_destroy(&actions);
		posix_spawnattr_destroy(&attr);
	}

	while (exe_path) {
		struct pollfd fds[2];

		fds[0].fd = fd_keyboard;
		fds[0].events = POLLIN;
		fds[0].revents = 0;

		fds[1].fd = fd_amaster;
		fds[1].events = POLLIN;
		fds[1].revents = 0;

		if (pid >= 0 && waitpid(pid, NULL, WNOHANG) == pid)
			pid = -1;

		if (pid >= 0)
			r = poll(&fds[0], 2, 100);
		else
			r = poll(&fds[1], 1, 100);

		if (r < 0) {
			perror("poll");
			r = EXIT_FAILURE;
			break;
		}

		if (r == 0) {
			if (pid < 0)
				break;
			continue;
		}

		if ((fds[0].revents & POLLIN) != 0) {
			int buffer[128];
			ssize_t size = (ssize_t)sizeof(buffer);

			size = read(fd_keyboard, &buffer[0], (size_t)size);

			if (size > 0) {
				int i, count = (int)size / (int)sizeof(int);

				for (i = 0; i < count; i++)
					handle_key(buffer[i]);
			}
		}

		if ((fds[1].revents & POLLIN) != 0) {
			unsigned char buffer[2048];
			ssize_t size = (ssize_t)(sizeof(buffer));

			size = read(fd_amaster, &buffer[0], (size_t)size);

			if (size > 0)
				write(1, &buffer[0], (size_t)size);
		}
	}

	close(fd_keyboard);
	close(fd_amaster);
	close(fd_aslave);

	return r;
}
