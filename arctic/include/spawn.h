/*
 * Copyright (c) 2022 Antti Tiihala
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
 * spawn.h
 *      Spawn a new process
 */

#ifndef __DANCY_SPAWN_H
#define __DANCY_SPAWN_H

#include <__dancy/core.h>
#include <__dancy/sched.h>
#include <__dancy/spawn.h>

__Dancy_Header_Begin

#ifndef __DANCY_TYPEDEF_MODE_T
#define __DANCY_TYPEDEF_MODE_T
typedef __dancy_mode_t mode_t;
#endif

#ifndef __DANCY_TYPEDEF_PID_T
#define __DANCY_TYPEDEF_PID_T
typedef __dancy_pid_t pid_t;
#endif

#ifndef __DANCY_TYPEDEF_SIGSET_T
#define __DANCY_TYPEDEF_SIGSET_T
typedef __dancy_sigset_t sigset_t;
#endif

#define POSIX_SPAWN_RESETIDS        (0x0001)
#define POSIX_SPAWN_SETPGROUP       (0x0002)
#define POSIX_SPAWN_SETSIGDEF       (0x0004)
#define POSIX_SPAWN_SETSIGMASK      (0x0008)
#define POSIX_SPAWN_SETSCHEDPARAM   (0x0010)
#define POSIX_SPAWN_SETSCHEDULER    (0x0020)

typedef struct __dancy_spawn_file_actions
	posix_spawn_file_actions_t;

typedef struct __dancy_spawn_attributes
	posix_spawnattr_t;

int posix_spawn(pid_t *pid, const char *path,
	const posix_spawn_file_actions_t *actions,
	const posix_spawnattr_t *attrp,
	char *const argv[], char *const envp[]);

int posix_spawnp(pid_t *pid, const char *file,
	const posix_spawn_file_actions_t *actions,
	const posix_spawnattr_t *attrp,
	char *const argv[], char *const envp[]);

int posix_spawn_file_actions_init(posix_spawn_file_actions_t *actions);
int posix_spawn_file_actions_destroy(posix_spawn_file_actions_t *actions);

int posix_spawn_file_actions_addclose(
	posix_spawn_file_actions_t *actions, int fd);

int posix_spawn_file_actions_adddup2(
	posix_spawn_file_actions_t *actions, int fd, int new_fd);

int posix_spawn_file_actions_addopen(
	posix_spawn_file_actions_t *actions, int fd,
	const char *path, int flags, mode_t mode);

int posix_spawnattr_init(posix_spawnattr_t *attrp);
int posix_spawnattr_destroy(posix_spawnattr_t *attrp);

int posix_spawnattr_getflags(const posix_spawnattr_t *attrp, short *flags);
int posix_spawnattr_setflags(posix_spawnattr_t *attrp, short flags);

int posix_spawnattr_getpgroup(const posix_spawnattr_t *attrp, pid_t *pgroup);
int posix_spawnattr_setpgroup(posix_spawnattr_t *attrp, pid_t pgroup);

int posix_spawnattr_getschedparam(
	const posix_spawnattr_t *attrp, struct sched_param *param);

int posix_spawnattr_setschedparam(
	posix_spawnattr_t *attrp, const struct sched_param *param);

int posix_spawnattr_getschedpolicy(
	const posix_spawnattr_t *attrp, int *policy);

int posix_spawnattr_setschedpolicy(
	posix_spawnattr_t *attrp, int policy);

int posix_spawnattr_getsigdefault(
	const posix_spawnattr_t *attrp, sigset_t *sigdefault);

int posix_spawnattr_setsigdefault(
	posix_spawnattr_t *attrp, const sigset_t *sigdefault);

int posix_spawnattr_getsigmask(
	const posix_spawnattr_t *attrp, sigset_t *sigmask);

int posix_spawnattr_setsigmask(
	posix_spawnattr_t *attrp, const sigset_t *sigmask);

__Dancy_Header_End

#endif
