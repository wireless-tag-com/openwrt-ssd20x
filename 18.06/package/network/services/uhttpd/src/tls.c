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

#include <dlfcn.h>
#include "uhttpd.h"
#include "tls.h"

#ifdef __APPLE__
#define LIB_EXT "dylib"
#else
#define LIB_EXT "so"
#endif

static struct ustream_ssl_ops *ops;
static void *dlh;
static void *ctx;

int uh_tls_init(const char *key, const char *crt)
{
	static bool _init = false;

	if (_init)
		return 0;

	_init = true;
	dlh = dlopen("libustream-ssl." LIB_EXT, RTLD_LAZY | RTLD_LOCAL);
	if (!dlh) {
		fprintf(stderr, "Failed to load ustream-ssl library: %s\n", dlerror());
		return -ENOENT;
	}

	ops = dlsym(dlh, "ustream_ssl_ops");
	if (!ops) {
		fprintf(stderr, "Could not find required symbol 'ustream_ssl_ops' in ustream-ssl library\n");
		return -ENOENT;
	}

	ctx = ops->context_new(true);
	if (!ctx) {
		fprintf(stderr, "Failed to initialize ustream-ssl\n");
		return -EINVAL;
	}

	if (ops->context_set_crt_file(ctx, crt) ||
	    ops->context_set_key_file(ctx, key)) {
		fprintf(stderr, "Failed to load certificate/key files\n");
		return -EINVAL;
	}

	return 0;
}

static void tls_ustream_read_cb(struct ustream *s, int bytes)
{
	struct client *cl = container_of(s, struct client, ssl.stream);

	uh_client_read_cb(cl);
}

static void tls_ustream_write_cb(struct ustream *s, int bytes)
{
	struct client *cl = container_of(s, struct client, ssl.stream);

	if (cl->dispatch.write_cb)
		cl->dispatch.write_cb(cl);
}

static void tls_notify_state(struct ustream *s)
{
	struct client *cl = container_of(s, struct client, ssl.stream);

	uh_client_notify_state(cl);
}

void uh_tls_client_attach(struct client *cl)
{
	cl->us = &cl->ssl.stream;
	ops->init(&cl->ssl, &cl->sfd.stream, ctx, true);
	cl->us->notify_read = tls_ustream_read_cb;
	cl->us->notify_write = tls_ustream_write_cb;
	cl->us->notify_state = tls_notify_state;
}

void uh_tls_client_detach(struct client *cl)
{
	ustream_free(&cl->ssl.stream);
}
