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

#include "uhttpd.h"

struct uhttpd_ops {
	void (*dispatch_add)(struct dispatch_handler *d);
	bool (*path_match)(const char *prefix, const char *url);

	bool (*create_process)(struct client *cl, struct path_info *pi, char *url,
			       void (*cb)(struct client *cl, struct path_info *pi, char *url));
	struct env_var *(*get_process_vars)(struct client *cl, struct path_info *pi);

	void (*http_header)(struct client *cl, int code, const char *summary);
	void (*client_error)(struct client *cl, int code, const char *summary, const char *fmt, ...);
	void (*request_done)(struct client *cl);
	void (*chunk_write)(struct client *cl, const void *data, int len);
	void (*chunk_printf)(struct client *cl, const char *format, ...);

	int (*urlencode)(char *buf, int blen, const char *src, int slen);
	int (*urldecode)(char *buf, int blen, const char *src, int slen);
};

struct uhttpd_plugin {
	struct list_head list;

	int (*init)(const struct uhttpd_ops *ops, struct config *conf);
	void (*post_init)(void);
};
