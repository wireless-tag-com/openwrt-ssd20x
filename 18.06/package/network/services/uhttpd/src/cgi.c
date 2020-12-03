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
#include <libubox/blobmsg.h>
#include "uhttpd.h"

static LIST_HEAD(interpreters);

void uh_interpreter_add(const char *ext, const char *path)
{
	struct interpreter *in;
	char *new_ext, *new_path;

	in = calloc_a(sizeof(*in),
		&new_ext, strlen(ext) + 1,
		&new_path, strlen(path) + 1);

	in->ext = strcpy(new_ext, ext);
	in->path = strcpy(new_path, path);
	list_add_tail(&in->list, &interpreters);
}

static void cgi_main(struct client *cl, struct path_info *pi, char *url)
{
	const struct interpreter *ip = pi->ip;
	struct env_var *var;

	clearenv();
	setenv("PATH", conf.cgi_path, 1);

	for (var = uh_get_process_vars(cl, pi); var->name; var++) {
		if (!var->value)
			continue;

		setenv(var->name, var->value, 1);
	}

	if (!chdir(pi->root)) {
		if (ip)
			execl(ip->path, ip->path, pi->phys, NULL);
		else
			execl(pi->phys, pi->phys, NULL);
	}

	printf("Status: 500 Internal Server Error\r\n\r\n"
	       "Unable to launch the requested CGI program:\n"
	       "  %s: %s\n", ip ? ip->path : pi->phys, strerror(errno));
}

static void cgi_handle_request(struct client *cl, char *url, struct path_info *pi)
{
	unsigned int mode = S_IFREG | S_IXOTH;
	char *escaped_url;

	if (!pi->ip && !((pi->stat.st_mode & mode) == mode)) {
		escaped_url = uh_htmlescape(url);

		uh_client_error(cl, 403, "Forbidden",
				"You don't have permission to access %s on this server.",
				escaped_url ? escaped_url : "the url");

		if (escaped_url)
			free(escaped_url);

		return;
	}

	if (!uh_create_process(cl, pi, url, cgi_main)) {
		uh_client_error(cl, 500, "Internal Server Error",
				"Failed to create CGI process: %s", strerror(errno));
		return;
	}

	return;
}

static bool check_cgi_path(struct path_info *pi, const char *url)
{
	struct interpreter *ip;
	const char *path = pi->phys;
	int path_len = strlen(path);

	list_for_each_entry(ip, &interpreters, list) {
		int len = strlen(ip->ext);

		if (len >= path_len)
			continue;

		if (strcmp(path + path_len - len, ip->ext) != 0)
			continue;

		pi->ip = ip;
		return true;
	}

	pi->ip = NULL;

	if (conf.cgi_docroot_path)
		return uh_path_match(conf.cgi_docroot_path, pi->phys);

	return false;
}

struct dispatch_handler cgi_dispatch = {
	.script = true,
	.check_path = check_cgi_path,
	.handle_request = cgi_handle_request,
};
