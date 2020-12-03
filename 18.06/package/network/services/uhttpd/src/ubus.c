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

#include <arpa/inet.h>
#include <libubox/blobmsg.h>
#include <libubox/blobmsg_json.h>
#include <libubox/avl.h>
#include <libubox/avl-cmp.h>
#include <stdio.h>
#include <poll.h>

#include "uhttpd.h"
#include "plugin.h"
#include "process.h"

static const struct uhttpd_ops *ops;
static struct config *_conf;
#define conf (*_conf)

static struct ubus_context *ctx;

#define UH_UBUS_MAX_POST_SIZE	8912
#define UH_UBUS_DEFAULT_SID	"00000000000000000000000000000000"

extern void uh_ubus_send_header(struct client *cl);
extern void uh_ubus_send_response(struct client *cl, char* buf);

#define JSON_PARAM_ERROR "{\"errcode\", \"-1000\"}"

void uh_ubus_send_header(struct client *cl)
{
	ops->http_header(cl, 200, "OK");

	ustream_printf(cl->us, "Content-Type: application/json\r\n");

	if (cl->request.method == UH_HTTP_MSG_OPTIONS)
		ustream_printf(cl->us, "Content-Length: 0\r\n");

	ustream_printf(cl->us, "\r\n");
}

void uh_ubus_send_response(struct client *cl, char* buf)
{
	const char *sep = "";

	ops->chunk_printf(cl, "%s%s", sep, buf);

	return ops->request_done(cl);
}

static void uh_ubus_json_error(struct client *cl)
{
	uh_ubus_send_header(cl);
	uh_ubus_send_response(cl, JSON_PARAM_ERROR);
}

static void uh_ubus_close_fds(struct client *cl)
{
	if (ctx->sock.fd < 0)
		return;

	close(ctx->sock.fd);
	ctx->sock.fd = -1;
}

static void uh_ubus_request_free(struct client *cl)
{
	struct dispatch_ubus *du = &cl->dispatch.ubus;

	uloop_timeout_cancel(&du->timeout);

	if (du->jsobj)
		json_object_put(du->jsobj);

	if (du->jstok)
		json_tokener_free(du->jstok);

	if (du->req_pending)
		ubus_abort_request(ctx, &du->req);
}

#ifdef UHTTPD_UBUS_DEBUG
#define UBUS_DEBUG_BUF_LEN      (1024)
static int ubus_console_printf(char *fmt, ...)
{
	char buf[UBUS_DEBUG_BUF_LEN] = {0};
	char cmd[UBUS_DEBUG_BUF_LEN] = {0};
	va_list valist;

	va_start(valist, fmt);
	vsnprintf(buf, UBUS_DEBUG_BUF_LEN, fmt, valist);
	va_end(valist);

	snprintf(cmd, UBUS_DEBUG_BUF_LEN, "echo \'%s\' > /dev/console", buf);

	return system(cmd);
}
#define UBUS_PRINTF(format, arg...) ubus_console_printf("%s:%d " format ,__FUNCTION__ ,__LINE__, ## arg);

#else

static inline void no_debug(int level, const char *fmt, ...)
{
}

#define UBUS_PRINTF(format, ...) no_debug(0, format, ## __VA_ARGS__)
#endif

static void uh_ubus_data_done(struct client *cl)
{
	struct dispatch_ubus *du = &cl->dispatch.ubus;
	struct json_object *obj = du->jsobj;
	char u_remote_addr[INET6_ADDRSTRLEN];
	struct json_object *jvalue;
	const char *json_str;
	int ret;

	uh_client_ref(cl);

	if (!obj) {
		goto error;
	}

	if (json_object_get_type(obj) != json_type_object) {
		goto error;
	}

	inet_ntop(cl->peer_addr.family, &cl->peer_addr.in, u_remote_addr, sizeof(u_remote_addr));
	jvalue = json_object_new_string(u_remote_addr);
	if (!jvalue) {
		goto error;
	}
	json_object_object_add(obj, "remote_addr", jvalue);

	json_str = json_object_to_json_string(obj);

	if (json_str) {
		ret = uapi_process_data(json_str, cl);
		if (ret != 0) {
			goto error;
		}
	}
	else
		goto error;

	return;

error:
	uh_ubus_json_error(cl);
	uh_client_unref(cl);
}

static int uh_ubus_data_send(struct client *cl, const char *data, int len)
{
	struct dispatch_ubus *du = &cl->dispatch.ubus;

	if (du->jsobj || !du->jstok)
		goto error;

	du->post_len += len;
	if (du->post_len > UH_UBUS_MAX_POST_SIZE)
		goto error;

	du->jsobj = json_tokener_parse_ex(du->jstok, data, len);
	return len;

error:
	uh_ubus_json_error(cl);
	return 0;
}

static void uh_ubus_handle_request(struct client *cl, char *url, struct path_info *pi)
{
	struct dispatch *d = &cl->dispatch;

	switch (cl->request.method)
	{
	case UH_HTTP_MSG_POST:
		d->data_send = uh_ubus_data_send;
		d->data_done = uh_ubus_data_done;
		d->close_fds = uh_ubus_close_fds;
		d->free = uh_ubus_request_free;
		d->ubus.jstok = json_tokener_new();
		break;

	default:
		ops->client_error(cl, 400, "Bad Request", "Invalid Request");
	}
}

static bool
uh_ubus_check_url(const char *url)
{
	return ops->path_match(conf.ubus_prefix, url);
}

static int
uh_ubus_init(void)
{
	static struct dispatch_handler ubus_dispatch = {
		.check_url = uh_ubus_check_url,
		.handle_request = uh_ubus_handle_request,
	};

	ctx = ubus_connect(conf.ubus_socket);
	if (!ctx) {
		fprintf(stderr, "Unable to connect to ubus socket\n");
		exit(1);
	}

	ops->dispatch_add(&ubus_dispatch);

	uloop_done();
	return 0;
}


static int uh_ubus_plugin_init(const struct uhttpd_ops *o, struct config *c)
{
	ops = o;
	_conf = c;
	return uh_ubus_init();
}

static void uh_ubus_post_init(void)
{
	ubus_add_uloop(ctx);
}

struct uhttpd_plugin uhttpd_plugin = {
	.init = uh_ubus_plugin_init,
	.post_init = uh_ubus_post_init,
};
