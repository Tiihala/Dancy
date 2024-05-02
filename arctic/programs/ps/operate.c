/*
 * Copyright (c) 2024 Antti Tiihala
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
 * ps/operate.c
 *      Write information about processes
 */

#include "main.h"

static pid_t *pid_array;
static size_t pid_array_count;

static pid_t ps_sess = -1;

static struct {
	pid_t ppid; pid_t pgid; pid_t sess; size_t mem;
} *pid_array_other;

static int get_width(long long num, int min)
{
	char b[32];
	int w = snprintf(&b[0], sizeof(b), "%lld", num);
	return (w < min) ? min : w;
}

static int write_information(struct options *opt)
{
	int cmdline = 0;
	pid_t max_pid[4] = { 0, 0, 0, 0 };
	size_t max_mem[1] = { 0 };
	int width[5];
	size_t i, j;

	for (i = 0; opt->operands[i] != NULL; i++) {
		if (!strcmp(opt->operands[i], "a")) {
			cmdline = 1;

		} else {
			opt->error = "unknown operands";
			return EXIT_FAILURE;
		}
	}

	max_pid[0] = pid_array[pid_array_count - 1];

	for (i = 0; i < pid_array_count; i++) {
		pid_t pid = pid_array[i];
		ssize_t size = 0;

		if (!(size < 0))
			size = __dancy_procinfo(pid,
				__DANCY_PROCINFO_OWNER_ID,
				&pid_array_other[i].ppid, sizeof(pid_t));

		if (!(size < 0))
			size = __dancy_procinfo(pid,
				__DANCY_PROCINFO_GROUP_ID,
				&pid_array_other[i].pgid, sizeof(pid_t));

		if (!(size < 0))
			size = __dancy_procinfo(pid,
				__DANCY_PROCINFO_SESSION_ID,
				&pid_array_other[i].sess, sizeof(pid_t));

		if (!(size < 0))
			size = __dancy_procinfo(pid,
				__DANCY_PROCINFO_MEMORY,
				&pid_array_other[i].mem, sizeof(size_t));

		if (size < 0) {
			if (errno == ESRCH)
				continue;

			perror(MAIN_CMDNAME ":__dancy_procinfo");
			return EXIT_FAILURE;
		}

		if (ps_sess > 0 && ps_sess != pid_array_other[i].sess)
			continue;

		if (max_pid[1] < pid_array_other[i].ppid)
			max_pid[1] = pid_array_other[i].ppid;

		if (max_pid[2] < pid_array_other[i].pgid)
			max_pid[2] = pid_array_other[i].pgid;

		if (max_pid[3] < pid_array_other[i].sess)
			max_pid[3] = pid_array_other[i].sess;

		if (max_mem[0] < pid_array_other[i].mem)
			max_mem[0] = pid_array_other[i].mem;
	}

	width[0] = get_width((long long)max_pid[0], 4);
	width[1] = get_width((long long)max_pid[1], 4);
	width[2] = get_width((long long)max_pid[2], 4);
	width[3] = get_width((long long)max_pid[3], 4);
	width[4] = get_width((long long)max_mem[0], 6);

	printf("%*s  %*s  %*s  %*s  %*s  %s\n",
		width[0],  "PID", width[1], "PPID",
		width[2], "PGID", width[3], "SESS",
		width[4],  "MEMORY", "COMMAND");

	for (i = 0; i < pid_array_count; i++) {
		pid_t pid = pid_array[i];
		const int request = __DANCY_PROCINFO_CMDLINE;

		uint8_t cmd[48];
		ssize_t size;

		if (ps_sess > 0 && ps_sess != pid_array_other[i].sess)
			continue;

		size = __dancy_procinfo(pid, request, &cmd[0], sizeof(cmd));

		if (size < 0) {
			if (errno == ESRCH)
				continue;

			perror(MAIN_CMDNAME ":__dancy_procinfo");
			return EXIT_FAILURE;
		}

		printf("%*lld  %*lld  %*lld  %*lld  %*lld  ",
			width[0], (long long)pid_array[i],
			width[1], (long long)pid_array_other[i].ppid,
			width[2], (long long)pid_array_other[i].pgid,
			width[3], (long long)pid_array_other[i].sess,
			width[4], (long long)pid_array_other[i].mem);

		for (j = 0; j < (size_t)size; j++) {
			int c = (int)cmd[j];

			if (c == 0) {
				if (!cmdline)
					break;
				printf(" ");
				continue;
			}

			if (c <= 0x20 || c >= 0x7F) {
				printf("\\x%02X", c);
				continue;
			}

			printf("%c", c);
		}

		printf("\n");
	}

	return 0;
}

int operate(struct options *opt)
{
	size_t array_size = 64 * sizeof(pid_t);
	int r;

	if (!opt->all) {
		if ((ps_sess = getsid(0)) < 0) {
			perror(MAIN_CMDNAME ": getsid");
			return EXIT_FAILURE;
		}
	}

	for (;;) {
		ssize_t size;

		if ((pid_array = malloc(array_size)) == NULL) {
			fputs(MAIN_CMDNAME ": out of memory\n", stderr);
			return EXIT_FAILURE;
		}

		if ((size = __dancy_proclist(pid_array, array_size)) >= 0) {
			pid_array_count = (size_t)size / sizeof(pid_t);
			break;
		}

		free(pid_array);

		if (errno != ENOMEM) {
			perror(MAIN_CMDNAME ": __dancy_proclist");
			return EXIT_FAILURE;
		}

		if ((array_size <<= 1) > 0x10000000) {
			fputs(MAIN_CMDNAME ": unexpected failure\n", stderr);
			return EXIT_FAILURE;
		}
	}

	if (pid_array_count == 0) {
		fputs(MAIN_CMDNAME ": no processes\n", stderr);
		return free(pid_array), EXIT_FAILURE;
	}

	pid_array_other = calloc(pid_array_count, sizeof(*pid_array_other));

	if (pid_array_other == NULL) {
		fputs(MAIN_CMDNAME ": out of memory\n", stderr);
		return free(pid_array), EXIT_FAILURE;
	}

	r = write_information(opt);

	free(pid_array_other);
	free(pid_array);

	return r;
}
