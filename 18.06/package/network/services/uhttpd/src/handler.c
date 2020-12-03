/*
 * uhttpd - Tiny single-threaded httpd
 *
 *   Copyright (C) 2015 Felix Fietkau <nbd@openwrt.org>
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

#include <libubox/blobmsg.h>
#include <libubox/blobmsg_json.h>
#include <libubox/json_script.h>

#include "uhttpd.h"

struct handler {
	struct list_head list;

	struct json_script_file *request;
	struct json_script_file *fallback;
};

static LIST_HEAD(handlers);
static struct json_script_ctx handler_ctx;
static struct env_var *cur_vars;
static struct blob_buf b;
static int handler_ret;
static struct client *cur_client;
static char **cur_url;

static void
handle_redirect(struct json_script_ctx *ctx, struct blob_attr *data)
{
	struct client *cl = cur_client;
	static struct blobmsg_policy policy[3] = {
		 { .type = BLOBMSG_TYPE_STRING },
		 { .type = BLOBMSG_TYPE_INT32 },
		 { .type = BLOBMSG_TYPE_STRING },
	};
	struct blob_attr *tb[3];
	const char *status = "Found";
	int code = 302;

	blobmsg_parse_array(policy, ARRAY_SIZE(policy), tb, blobmsg_data(data), blobmsg_data_len(data));
	if (!tb[0])
		return;

	if (tb[1]) {
		code = blobmsg_get_u32(tb[1]);
		if (tb[2])
			status = blobmsg_get_string(tb[2]);
	}

	uh_http_header(cl, code, status);
	if (!uh_use_chunked(cl))
		ustream_printf(cl->us, "Content-Length: 0\r\n");
	ustream_printf(cl->us, "Location: %s\r\n\r\n",
		       blobmsg_get_string(tb[0]));
	uh_request_done(cl);
	*cur_url = NULL;

	handler_ret = 1;
	json_script_abort(ctx);
}

static void
handle_set_uri(struct json_script_ctx *ctx, struct blob_attr *data)
{
	struct client *cl = cur_client;
	static struct blobmsg_policy policy = {
		 .type = BLOBMSG_TYPE_STRING,
	};
	struct blob_attr *tb;
	struct blob_attr *old_url = blob_data(cl->hdr.head);

	blobmsg_parse_array(&policy, 1, &tb, blobmsg_data(data), blobmsg_data_len(data));
	if (!tb)
		return;

	blob_buf_init(&b, 0);
	blob_put_raw(&b, blob_next(old_url), blob_len(cl->hdr.head) - blob_pad_len(old_url));

	/* replace URL in client header cache */
	blob_buf_init(&cl->hdr, 0);
	blobmsg_add_string(&cl->hdr, "URL", blobmsg_get_string(tb));
	blob_put_raw(&cl->hdr, blob_data(b.head), blob_len(b.head));
	*cur_url = blobmsg_data(blob_data(cl->hdr.head));
	cur_vars = NULL;

	blob_buf_init(&b, 0);

	handler_ret = 1;
	json_script_abort(ctx);
}

static void
handle_add_header(struct json_script_ctx *ctx, struct blob_attr *data)
{
	struct client *cl = cur_client;
	static struct blobmsg_policy policy[2] = {
		 { .type = BLOBMSG_TYPE_STRING },
		 { .type = BLOBMSG_TYPE_STRING },
	};
	struct blob_attr *tb[2];

	blobmsg_parse_array(policy, ARRAY_SIZE(tb), tb, blobmsg_data(data), blobmsg_data_len(data));
	if (!tb[0] || !tb[1])
		return;

	blobmsg_add_string(&cl->hdr_response, blobmsg_get_string(tb[0]),
			   blobmsg_get_string(tb[1]));
}

static void
handle_no_cache(struct json_script_ctx *ctx, struct blob_attr *data)
{
	struct client *cl = cur_client;

	cl->dispatch.no_cache = true;
}

static void
handle_command(struct json_script_ctx *ctx, const char *name,
	       struct blob_attr *data, struct blob_attr *vars)
{
	static const struct {
		const char *name;
		void (*func)(struct json_script_ctx *ctx, struct blob_attr *data);
	} cmds[] = {
		{ "redirect", handle_redirect },
		{ "rewrite", handle_set_uri },
		{ "add-header", handle_add_header },
		{ "no-cache", handle_no_cache },
	};
	int i;

	for (i = 0; i < ARRAY_SIZE(cmds); i++) {
		if (!strcmp(cmds[i].name, name)) {
			cmds[i].func(ctx, data);
			return;
		}
	}
}

static const char *
handle_var(struct json_script_ctx *ctx, const char *name,
	   struct blob_attr *vars)
{
	struct client *cl = cur_client;
	struct env_var *cur;
	static struct path_info empty_path;

	if (!cur_vars) {
		struct path_info *p = uh_path_lookup(cl, *cur_url);

		if (!p)
			p = &empty_path;

		cur_vars = uh_get_process_vars(cl, p);
	}

	for (cur = cur_vars; cur->name; cur++) {
		if (!strcmp(cur->name, name))
			return cur->value;
	}
	return NULL;
}

static void
handler_init(void)
{
	if (handler_ctx.handle_command)
		return;

	json_script_init(&handler_ctx);
	handler_ctx.handle_command = handle_command;
	handler_ctx.handle_var = handle_var;
}

static bool set_handler(struct json_script_file **dest, struct blob_attr *data)
{
	if (!data)
		return true;

	*dest = json_script_file_from_blobmsg(NULL, blobmsg_data(data), blobmsg_data_len(data));
	return *dest;
}

int uh_handler_add(const char *file)
{
	enum {
		H_REQUEST,
		H_FALLBACK,
		__H_MAX,
	};
	struct blobmsg_policy policy[__H_MAX] = {
		[H_REQUEST] = { "request", BLOBMSG_TYPE_ARRAY },
		[H_FALLBACK] = { "fallback", BLOBMSG_TYPE_ARRAY },
	};
	struct blob_attr *tb[__H_MAX];
	struct handler *h;

	handler_init();
	blob_buf_init(&b, 0);

	if (!blobmsg_add_json_from_file(&b, file))
		return -1;

	blobmsg_parse(policy, __H_MAX, tb, blob_data(b.head), blob_len(b.head));
	if (!tb[H_REQUEST] && !tb[H_FALLBACK])
		return -1;

	h = calloc(1, sizeof(*h));
	if (!set_handler(&h->request, tb[H_REQUEST]) ||
	    !set_handler(&h->fallback, tb[H_FALLBACK])) {
		free(h->request);
		free(h->fallback);
		free(h);
		return -1;
	}

	list_add_tail(&h->list, &handlers);
	return 0;
}

int uh_handler_run(struct client *cl, char **url, bool fallback)
{
	struct json_script_file *f;
	struct handler *h;

	cur_client = cl;
	cur_url = url;
	cur_vars = NULL;

	handler_ret = 0;

	list_for_each_entry(h, &handlers, list) {
		f = fallback ? h->fallback : h->request;
		if (!f)
			continue;

		blob_buf_init(&b, 0);
		json_script_run_file(&handler_ctx, f, b.head);
		if (handler_ctx.abort)
			break;
	}

	return handler_ret;
}
