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

#include <libubox/blobmsg.h>
#include <ctype.h>

#include "uhttpd.h"
#include "tls.h"

static LIST_HEAD(clients);
static bool client_done = false;

int n_clients = 0;
struct config conf = {};

const char * const http_versions[] = {
	[UH_HTTP_VER_0_9] = "HTTP/0.9",
	[UH_HTTP_VER_1_0] = "HTTP/1.0",
	[UH_HTTP_VER_1_1] = "HTTP/1.1",
};

const char * const http_methods[] = {
	[UH_HTTP_MSG_GET] = "GET",
	[UH_HTTP_MSG_POST] = "POST",
	[UH_HTTP_MSG_HEAD] = "HEAD",
	[UH_HTTP_MSG_OPTIONS] = "OPTIONS",
	[UH_HTTP_MSG_PUT] = "PUT",
	[UH_HTTP_MSG_PATCH] = "PATCH",
	[UH_HTTP_MSG_DELETE] = "DELETE",
};

void uh_http_header(struct client *cl, int code, const char *summary)
{
	struct http_request *r = &cl->request;
	struct blob_attr *cur;
	const char *enc = "Transfer-Encoding: chunked\r\n";
	const char *conn;
	int rem;

	cl->http_code = code;

	if (!uh_use_chunked(cl))
		enc = "";

	if (r->connection_close)
		conn = "Connection: close";
	else
		conn = "Connection: Keep-Alive";

	ustream_printf(cl->us, "%s %03i %s\r\n%s\r\n%s",
		http_versions[cl->request.version],
		code, summary, conn, enc);

	if (!r->connection_close)
		ustream_printf(cl->us, "Keep-Alive: timeout=%d\r\n", conf.http_keepalive);

	blobmsg_for_each_attr(cur, cl->hdr_response.head, rem)
		ustream_printf(cl->us, "%s: %s\r\n", blobmsg_name(cur),
			       blobmsg_get_string(cur));
}

static void uh_connection_close(struct client *cl)
{
	cl->state = CLIENT_STATE_CLOSE;
	cl->us->eof = true;
	ustream_state_change(cl->us);
}

static void uh_dispatch_done(struct client *cl)
{
	if (cl->dispatch.free)
		cl->dispatch.free(cl);
	if (cl->dispatch.req_free)
		cl->dispatch.req_free(cl);
}

static void client_timeout(struct uloop_timeout *timeout)
{
	struct client *cl = container_of(timeout, struct client, timeout);

	cl->state = CLIENT_STATE_CLOSE;
	uh_connection_close(cl);
}

static void uh_set_client_timeout(struct client *cl, int timeout)
{
	cl->timeout.cb = client_timeout;
	uloop_timeout_set(&cl->timeout, timeout * 1000);
}

static void uh_keepalive_poll_cb(struct uloop_timeout *timeout)
{
	struct client *cl = container_of(timeout, struct client, timeout);
	int sec = cl->requests > 0 ? conf.http_keepalive : conf.network_timeout;

	uh_set_client_timeout(cl, sec);
	cl->us->notify_read(cl->us, 0);
}

static void uh_poll_connection(struct client *cl)
{
	cl->timeout.cb = uh_keepalive_poll_cb;
	uloop_timeout_set(&cl->timeout, 1);
}

void uh_request_done(struct client *cl)
{
	uh_chunk_eof(cl);
	uh_dispatch_done(cl);
	blob_buf_init(&cl->hdr_response, 0);
	memset(&cl->dispatch, 0, sizeof(cl->dispatch));

	if (!conf.http_keepalive || cl->request.connection_close)
		return uh_connection_close(cl);

	cl->state = CLIENT_STATE_INIT;
	cl->requests++;
	uh_poll_connection(cl);
}

void __printf(4, 5)
uh_client_error(struct client *cl, int code, const char *summary, const char *fmt, ...)
{
	va_list arg;

	uh_http_header(cl, code, summary);
	ustream_printf(cl->us, "Content-Type: text/html\r\n\r\n");

	uh_chunk_printf(cl, "<h1>%s</h1>", summary);

	if (fmt) {
		va_start(arg, fmt);
		uh_chunk_vprintf(cl, fmt, arg);
		va_end(arg);
	}

	uh_request_done(cl);
}

static void uh_header_error(struct client *cl, int code, const char *summary)
{
	uh_client_error(cl, code, summary, NULL);
	uh_connection_close(cl);
}

static int find_idx(const char * const *list, int max, const char *str)
{
	int i;

	for (i = 0; i < max; i++)
		if (!strcmp(list[i], str))
			return i;

	return -1;
}

static int client_parse_request(struct client *cl, char *data)
{
	struct http_request *req = &cl->request;
	char *type, *path, *version;
	int h_method, h_version;

	type = strtok(data, " ");
	path = strtok(NULL, " ");
	version = strtok(NULL, " ");
	if (!type || !path || !version)
		return CLIENT_STATE_DONE;

	blobmsg_add_string(&cl->hdr, "URL", path);

	memset(&cl->request, 0, sizeof(cl->request));
	h_method = find_idx(http_methods, ARRAY_SIZE(http_methods), type);
	h_version = find_idx(http_versions, ARRAY_SIZE(http_versions), version);
	if (h_method < 0 || h_version < 0) {
		req->version = UH_HTTP_VER_1_0;
		return CLIENT_STATE_DONE;
	}

	req->method = h_method;
	req->version = h_version;
	if (req->version < UH_HTTP_VER_1_1 || req->method == UH_HTTP_MSG_POST ||
	    !conf.http_keepalive)
		req->connection_close = true;

	return CLIENT_STATE_HEADER;
}

static bool client_init_cb(struct client *cl, char *buf, int len)
{
	char *newline;

	newline = strstr(buf, "\r\n");
	if (!newline)
		return false;

	if (newline == buf) {
		ustream_consume(cl->us, 2);
		return true;
	}

	*newline = 0;
	blob_buf_init(&cl->hdr, 0);
	cl->state = client_parse_request(cl, buf);
	ustream_consume(cl->us, newline + 2 - buf);
	if (cl->state == CLIENT_STATE_DONE)
		uh_header_error(cl, 400, "Bad Request");

	return true;
}

static bool rfc1918_filter_check(struct client *cl)
{
	if (!conf.rfc1918_filter)
		return true;

	if (!uh_addr_rfc1918(&cl->peer_addr) || uh_addr_rfc1918(&cl->srv_addr))
		return true;

	uh_client_error(cl, 403, "Forbidden",
			"Rejected request from RFC1918 IP "
			"to public server address");
	return false;
}

static bool tls_redirect_check(struct client *cl)
{
	int rem, port;
	struct blob_attr *cur;
	char *ptr, *url = NULL, *host = NULL;

	if (cl->tls || !conf.tls_redirect)
		return true;

	if ((port = uh_first_tls_port(cl->srv_addr.family)) == -1)
		return true;

	blob_for_each_attr(cur, cl->hdr.head, rem) {
		if (!strcmp(blobmsg_name(cur), "host"))
			host = blobmsg_get_string(cur);

		if (!strcmp(blobmsg_name(cur), "URL"))
			url = blobmsg_get_string(cur);

		if (url && host)
			break;
	}

	if (!url || !host)
		return true;

	if ((ptr = strchr(host, ']')) != NULL)
		*(ptr+1) = 0;
	else if ((ptr = strchr(host, ':')) != NULL)
		*ptr = 0;

	cl->request.disable_chunked = true;
	cl->request.connection_close = true;

	uh_http_header(cl, 307, "Temporary Redirect");

	if (port != 443)
		ustream_printf(cl->us, "Location: https://%s:%d%s\r\n\r\n", host, port, url);
	else
		ustream_printf(cl->us, "Location: https://%s%s\r\n\r\n", host, url);

	uh_request_done(cl);

	return false;
}

static void client_header_complete(struct client *cl)
{
	struct http_request *r = &cl->request;

	if (!rfc1918_filter_check(cl))
		return;

	if (!tls_redirect_check(cl))
		return;

	if (r->expect_cont)
		ustream_printf(cl->us, "HTTP/1.1 100 Continue\r\n\r\n");

	switch(r->ua) {
	case UH_UA_MSIE_OLD:
		if (r->method != UH_HTTP_MSG_POST)
			break;

		/* fall through */
	case UH_UA_SAFARI:
		r->connection_close = true;
		break;
	default:
		break;
	}

	uh_handle_request(cl);
}

static void client_parse_header(struct client *cl, char *data)
{
	struct http_request *r = &cl->request;
	char *err;
	char *name;
	char *val;

	if (!*data) {
		uloop_timeout_cancel(&cl->timeout);
		cl->state = CLIENT_STATE_DATA;
		client_header_complete(cl);
		return;
	}

	val = uh_split_header(data);
	if (!val) {
		cl->state = CLIENT_STATE_DONE;
		return;
	}

	for (name = data; *name; name++)
		if (isupper(*name))
			*name = tolower(*name);

	if (!strcmp(data, "expect")) {
		if (!strcasecmp(val, "100-continue"))
			r->expect_cont = true;
		else {
			uh_header_error(cl, 412, "Precondition Failed");
			return;
		}
	} else if (!strcmp(data, "content-length")) {
		r->content_length = strtoul(val, &err, 0);
		if ((err && *err) || r->content_length < 0) {
			uh_header_error(cl, 400, "Bad Request");
			return;
		}
	} else if (!strcmp(data, "transfer-encoding")) {
		if (!strcmp(val, "chunked"))
			r->transfer_chunked = true;
	} else if (!strcmp(data, "connection")) {
		if (!strcasecmp(val, "close"))
			r->connection_close = true;
	} else if (!strcmp(data, "user-agent")) {
		char *str;

		if (strstr(val, "Opera"))
			r->ua = UH_UA_OPERA;
		else if ((str = strstr(val, "MSIE ")) != NULL) {
			r->ua = UH_UA_MSIE_NEW;
			if (str[5] && str[6] == '.') {
				switch (str[5]) {
				case '6':
					if (strstr(str, "SV1"))
						break;
					/* fall through */
				case '5':
				case '4':
					r->ua = UH_UA_MSIE_OLD;
					break;
				}
			}
		}
		else if (strstr(val, "Chrome/"))
			r->ua = UH_UA_CHROME;
		else if (strstr(val, "Safari/") && strstr(val, "Mac OS X"))
			r->ua = UH_UA_SAFARI;
		else if (strstr(val, "Gecko/"))
			r->ua = UH_UA_GECKO;
		else if (strstr(val, "Konqueror"))
			r->ua = UH_UA_KONQUEROR;
	}


	blobmsg_add_string(&cl->hdr, data, val);

	cl->state = CLIENT_STATE_HEADER;
}

void client_poll_post_data(struct client *cl)
{
	struct dispatch *d = &cl->dispatch;
	struct http_request *r = &cl->request;
	char *buf;
	int len;

	if (cl->state == CLIENT_STATE_DONE)
		return;

	while (1) {
		char *sep;
		int offset = 0;
		int cur_len;

		buf = ustream_get_read_buf(cl->us, &len);
		if (!buf || !len)
			break;

		if (!d->data_send)
			return;

		cur_len = min(r->content_length, len);
		if (cur_len) {
			if (d->data_blocked)
				break;

			if (d->data_send)
				cur_len = d->data_send(cl, buf, cur_len);

			r->content_length -= cur_len;
			ustream_consume(cl->us, cur_len);
			continue;
		}

		if (!r->transfer_chunked)
			break;

		if (r->transfer_chunked > 1)
			offset = 2;

		sep = strstr(buf + offset, "\r\n");
		if (!sep)
			break;

		*sep = 0;

		r->content_length = strtoul(buf + offset, &sep, 16);
		r->transfer_chunked++;
		ustream_consume(cl->us, sep + 2 - buf);

		/* invalid chunk length */
		if ((sep && *sep) || r->content_length < 0) {
			r->content_length = 0;
			r->transfer_chunked = 0;
			break;
		}

		/* empty chunk == eof */
		if (!r->content_length) {
			r->transfer_chunked = false;
			break;
		}
	}

	buf = ustream_get_read_buf(cl->us, &len);
	if (!r->content_length && !r->transfer_chunked &&
		cl->state != CLIENT_STATE_DONE) {
		if (cl->dispatch.data_done)
			cl->dispatch.data_done(cl);

		cl->state = CLIENT_STATE_DONE;
	}
}

static bool client_data_cb(struct client *cl, char *buf, int len)
{
	client_poll_post_data(cl);
	return false;
}

static bool client_header_cb(struct client *cl, char *buf, int len)
{
	char *newline;
	int line_len;

	newline = strstr(buf, "\r\n");
	if (!newline)
		return false;

	*newline = 0;
	client_parse_header(cl, buf);
	line_len = newline + 2 - buf;
	ustream_consume(cl->us, line_len);
	if (cl->state == CLIENT_STATE_DATA)
		return client_data_cb(cl, newline + 2, len - line_len);

	return true;
}

typedef bool (*read_cb_t)(struct client *cl, char *buf, int len);
static read_cb_t read_cbs[] = {
	[CLIENT_STATE_INIT] = client_init_cb,
	[CLIENT_STATE_HEADER] = client_header_cb,
	[CLIENT_STATE_DATA] = client_data_cb,
};

void uh_client_read_cb(struct client *cl)
{
	struct ustream *us = cl->us;
	char *str;
	int len;

	client_done = false;
	do {
		str = ustream_get_read_buf(us, &len);
		if (!str || !len)
			break;

		if (cl->state >= array_size(read_cbs) || !read_cbs[cl->state])
			break;

		if (!read_cbs[cl->state](cl, str, len)) {
			if (len == us->r.buffer_len &&
			    cl->state != CLIENT_STATE_DATA)
				uh_header_error(cl, 413, "Request Entity Too Large");
			break;
		}
	} while (!client_done);
}

static void client_close(struct client *cl)
{
	if (cl->refcount) {
		cl->state = CLIENT_STATE_CLEANUP;
		return;
	}

	client_done = true;
	n_clients--;
	uh_dispatch_done(cl);
	uloop_timeout_cancel(&cl->timeout);
	if (cl->tls)
		uh_tls_client_detach(cl);
	ustream_free(&cl->sfd.stream);
	close(cl->sfd.fd.fd);
	list_del(&cl->list);
	blob_buf_free(&cl->hdr);
	blob_buf_free(&cl->hdr_response);
	free(cl);

	uh_unblock_listeners();
}

void uh_client_notify_state(struct client *cl)
{
	struct ustream *s = cl->us;

	if (!s->write_error && cl->state != CLIENT_STATE_CLEANUP) {
		if (cl->state == CLIENT_STATE_DATA)
			return;

		if (!s->eof || s->w.data_bytes)
			return;

#ifdef HAVE_TLS
		if (cl->tls && cl->ssl.conn && cl->ssl.conn->w.data_bytes) {
			cl->ssl.conn->eof = s->eof;
			if (!ustream_write_pending(cl->ssl.conn))
				return;
		}
#endif
	}

	return client_close(cl);
}

static void client_ustream_read_cb(struct ustream *s, int bytes)
{
	struct client *cl = container_of(s, struct client, sfd.stream);

	uh_client_read_cb(cl);
}

static void client_ustream_write_cb(struct ustream *s, int bytes)
{
	struct client *cl = container_of(s, struct client, sfd.stream);

	if (cl->dispatch.write_cb)
		cl->dispatch.write_cb(cl);
}

static void client_notify_state(struct ustream *s)
{
	struct client *cl = container_of(s, struct client, sfd.stream);

	uh_client_notify_state(cl);
}

static void set_addr(struct uh_addr *addr, void *src)
{
	struct sockaddr_in *sin = src;
	struct sockaddr_in6 *sin6 = src;

	addr->family = sin->sin_family;
	if (addr->family == AF_INET) {
		addr->port = ntohs(sin->sin_port);
		memcpy(&addr->in, &sin->sin_addr, sizeof(addr->in));
	} else {
		addr->port = ntohs(sin6->sin6_port);
		memcpy(&addr->in6, &sin6->sin6_addr, sizeof(addr->in6));
	}
}

bool uh_accept_client(int fd, bool tls)
{
	static struct client *next_client;
	struct client *cl;
	unsigned int sl;
	int sfd;
	static int client_id = 0;
	struct sockaddr_in6 addr;

	if (!next_client)
		next_client = calloc(1, sizeof(*next_client));

	cl = next_client;

	sl = sizeof(addr);
	sfd = accept(fd, (struct sockaddr *) &addr, &sl);
	if (sfd < 0)
		return false;

	set_addr(&cl->peer_addr, &addr);
	sl = sizeof(addr);
	getsockname(sfd, (struct sockaddr *) &addr, &sl);
	set_addr(&cl->srv_addr, &addr);

	cl->us = &cl->sfd.stream;
	if (tls) {
		uh_tls_client_attach(cl);
	} else {
		cl->us->notify_read = client_ustream_read_cb;
		cl->us->notify_write = client_ustream_write_cb;
		cl->us->notify_state = client_notify_state;
	}

	cl->us->string_data = true;
	ustream_fd_init(&cl->sfd, sfd);

	uh_poll_connection(cl);
	list_add_tail(&cl->list, &clients);

	next_client = NULL;
	n_clients++;
	cl->id = client_id++;
	cl->tls = tls;

	return true;
}

void uh_close_fds(void)
{
	struct client *cl;

	uloop_done();
	uh_close_listen_fds();
	list_for_each_entry(cl, &clients, list) {
		close(cl->sfd.fd.fd);
		if (cl->dispatch.close_fds)
			cl->dispatch.close_fds(cl);
	}
}
