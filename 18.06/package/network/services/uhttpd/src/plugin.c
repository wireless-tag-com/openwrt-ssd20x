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
#include "plugin.h"

static LIST_HEAD(plugins);

static const struct uhttpd_ops ops = {
	.dispatch_add = uh_dispatch_add,
	.path_match = uh_path_match,
	.create_process = uh_create_process,
	.get_process_vars = uh_get_process_vars,
	.http_header = uh_http_header,
	.client_error = uh_client_error,
	.request_done = uh_request_done,
	.chunk_write = uh_chunk_write,
	.chunk_printf = uh_chunk_printf,
	.urlencode = uh_urlencode,
	.urldecode = uh_urldecode,
};

int uh_plugin_init(const char *name)
{
	struct uhttpd_plugin *p;
	const char *sym;
	void *dlh;

	dlh = dlopen(name, RTLD_LAZY | RTLD_GLOBAL);
	if (!dlh) {
		fprintf(stderr, "Could not open plugin %s: %s\n", name, dlerror());
		return -ENOENT;
	}

	sym = "uhttpd_plugin";
	p = dlsym(dlh, sym);
	if (!p) {
		fprintf(stderr, "Could not find symbol '%s' in plugin '%s'\n", sym, name);
		return -ENOENT;
	}

	list_add(&p->list, &plugins);
	return p->init(&ops, &conf);
}

void uh_plugin_post_init(void)
{
	struct uhttpd_plugin *p;

	list_for_each_entry(p, &plugins, list)
		if (p->post_init)
			p->post_init();
}
