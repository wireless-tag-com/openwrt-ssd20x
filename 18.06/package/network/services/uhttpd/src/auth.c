/*
 * uhttpd - Tiny single-threaded httpd
 *
 *   Copyright (C) 2010-2013 Jo-Philipp Wich <xm@subsignal.org>
 *   Copyright (C) 2013 Felix Fietkau <nbd@openwrt.org>
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
 */

#define _GNU_SOURCE
#define _XOPEN_SOURCE	700
#include <strings.h>
#ifdef HAVE_SHADOW
#include <shadow.h>
#endif
#include "uhttpd.h"

static LIST_HEAD(auth_realms);

void uh_auth_add(const char *path, const char *user, const char *pass)
{
	struct auth_realm *new = NULL;
	struct passwd *pwd;
	const char *new_pass = NULL;
	char *dest_path, *dest_user, *dest_pass;

#ifdef HAVE_SHADOW
	struct spwd *spwd;
#endif

	/* given password refers to a passwd entry */
	if ((strlen(pass) > 3) && !strncmp(pass, "$p$", 3)) {
#ifdef HAVE_SHADOW
		/* try to resolve shadow entry */
		spwd = getspnam(&pass[3]);
		if (spwd)
			new_pass = spwd->sp_pwdp;
#endif
		if (!new_pass) {
			pwd = getpwnam(&pass[3]);
			if (pwd && pwd->pw_passwd && pwd->pw_passwd[0] &&
			    pwd->pw_passwd[0] != '!')
				new_pass = pwd->pw_passwd;
		}
	} else {
		new_pass = pass;
	}

	if (!new_pass || !new_pass[0])
		return;

	new = calloc_a(sizeof(*new),
		&dest_path, strlen(path) + 1,
		&dest_user, strlen(user) + 1,
		&dest_pass, strlen(new_pass) + 1);

	if (!new)
		return;

	new->path = strcpy(dest_path, path);
	new->user = strcpy(dest_user, user);
	new->pass = strcpy(dest_pass, new_pass);
	list_add(&new->list, &auth_realms);
}

bool uh_auth_check(struct client *cl, const char *path, const char *auth,
                   char **uptr, char **pptr)
{
	struct http_request *req = &cl->request;
	struct auth_realm *realm;
	bool user_match = false;
	char *user = NULL;
	char *pass = NULL;
	int plen;

	if (uptr)
		*uptr = NULL;

	if (pptr)
		*pptr = NULL;

	if (auth && !strncasecmp(auth, "Basic ", 6)) {
		auth += 6;

		uh_b64decode(uh_buf, sizeof(uh_buf), auth, strlen(auth));
		pass = strchr(uh_buf, ':');
		if (pass) {
			user = uh_buf;
			*pass++ = 0;
		}
	}

	req->realm = NULL;
	plen = strlen(path);
	list_for_each_entry(realm, &auth_realms, list) {
		int rlen = strlen(realm->path);

		if (plen < rlen)
			continue;

		if (strncasecmp(path, realm->path, rlen) != 0)
			continue;

		req->realm = realm;
		if (!user)
			break;

		if (strcmp(user, realm->user) != 0)
			continue;

		user_match = true;
		break;
	}

	if (!req->realm)
		return true;

	if (user_match &&
	    (!strcmp(pass, realm->pass) ||
	     !strcmp(crypt(pass, realm->pass), realm->pass))) {
		if (uptr)
			*uptr = user;

		if (pptr)
			*pptr = pass;

		return true;
	}

	uh_http_header(cl, 401, "Authorization Required");
	ustream_printf(cl->us,
				  "WWW-Authenticate: Basic realm=\"%s\"\r\n"
				  "Content-Type: text/plain\r\n\r\n",
				  conf.realm);
	uh_chunk_printf(cl, "Authorization Required\n");
	uh_request_done(cl);

	return false;
}
