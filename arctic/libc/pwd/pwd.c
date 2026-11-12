/*
 * Copyright (c) 2026 Antti Tiihala
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
 * libc/pwd/pwd.c
 *      Password structure functions
 */

#include <errno.h>
#include <pwd.h>
#include <string.h>

static struct passwd s_passwd;
static char s_buffer[64];

static int dancy_database_state = 0;

static struct passwd *dancy_get_passwd(struct passwd *pwd,
	char *buffer, size_t buffer_size)
{
	char pw_name[]  = "root";
	char pw_dir[]   = "/root";
	char pw_shell[] = "/bin/dsh";

	size_t s = 0;

	memset(pwd, 0, sizeof(*pwd));

	s += sizeof(pw_name);
	s += sizeof(pw_dir);
	s += sizeof(pw_shell);

	if (s > buffer_size)
		return NULL;

	pwd->pw_name = strcpy(buffer, &pw_name[0]);
	buffer += sizeof(pw_name);

	pwd->pw_dir = strcpy(buffer, &pw_dir[0]);
	buffer += sizeof(pw_dir);

	pwd->pw_shell = strcpy(buffer, &pw_shell[0]);
	buffer += sizeof(pw_shell);

	return pwd;
}

void endpwent(void)
{
	dancy_database_state = -1;
}

void setpwent(void)
{
	dancy_database_state = 0;
}

struct passwd *getpwent(void)
{
	if (dancy_database_state != 0)
		return NULL;

	dancy_database_state = 1;

	return dancy_get_passwd(&s_passwd, &s_buffer[0], sizeof(s_buffer));
}

struct passwd *getpwnam(const char *name)
{
	if (!name || strcmp(name, "root"))
		return NULL;

	return dancy_get_passwd(&s_passwd, &s_buffer[0], sizeof(s_buffer));
}

struct passwd *getpwuid(uid_t uid)
{
	if (uid != 0)
		return NULL;

	return dancy_get_passwd(&s_passwd, &s_buffer[0], sizeof(s_buffer));
}

int getpwnam_r(const char *name, struct passwd *pwd, char *buffer,
	size_t buffer_size, struct passwd **result)
{
	if (!name || strcmp(name, "root"))
		return (*result = NULL), 0;

	if ((*result = dancy_get_passwd(pwd, buffer, buffer_size)) == NULL)
		return ERANGE;

	return 0;
}

int getpwuid_r(uid_t uid, struct passwd *pwd, char *buffer,
	size_t buffer_size, struct passwd **result)
{
	if (uid != 0)
		return (*result = NULL), 0;

	if ((*result = dancy_get_passwd(pwd, buffer, buffer_size)) == NULL)
		return ERANGE;

	return 0;
}
