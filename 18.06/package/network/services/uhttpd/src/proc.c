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
#include "uhttpd.h"

#define __headers \
	__header(accept, accept) \
	__header(accept_charset, accept-charset) \
	__header(accept_encoding, accept-encoding) \
	__header(accept_language, accept-language) \
	__header(authorization, authorization) \
	__header(connection, connection) \
	__header(cookie, cookie) \
	__header(host, host) \
	__header(origin, origin) \
	__header(referer, referer) \
	__header(user_agent, user-agent) \
	__header(content_type, content-type) \
	__header(content_length, content-length) \
	__header(x_http_method_override, x-http-method-override) \
	__header(http_auth_user, http-auth-user) \
	__header(http_auth_pass, http-auth-pass)

#undef __header
#define __header __enum_header
enum client_hdr {
	__headers
	__HDR_MAX,
};

#undef __header
#define __header __blobmsg_header
static const struct blobmsg_policy hdr_policy[__HDR_MAX] = {
	__headers
};

static const struct {
	const char *name;
	int idx;
} proc_header_env[] = {
	{ "HTTP_ACCEPT", HDR_accept },
	{ "HTTP_ACCEPT_CHARSET", HDR_accept_charset },
	{ "HTTP_ACCEPT_ENCODING", HDR_accept_encoding },
	{ "HTTP_ACCEPT_LANGUAGE", HDR_accept_language },
	{ "HTTP_AUTHORIZATION", HDR_authorization },
	{ "HTTP_CONNECTION", HDR_connection },
	{ "HTTP_COOKIE", HDR_cookie },
	{ "HTTP_HOST", HDR_host },
	{ "HTTP_ORIGIN", HDR_origin },
	{ "HTTP_REFERER", HDR_referer },
	{ "HTTP_USER_AGENT", HDR_user_agent },
	{ "HTTP_X_HTTP_METHOD_OVERRIDE", HDR_x_http_method_override },
	{ "HTTP_AUTH_USER", HDR_http_auth_user },
	{ "HTTP_AUTH_PASS", HDR_http_auth_pass },
	{ "CONTENT_TYPE", HDR_content_type },
	{ "CONTENT_LENGTH", HDR_content_length },
};

enum extra_vars {
	/* no update needed */
	_VAR_GW,
	_VAR_SOFTWARE,

	/* updated by uh_get_process_vars */
	VAR_SCRIPT_NAME,
	VAR_SCRIPT_FILE,
	VAR_DOCROOT,
	VAR_QUERY,
	VAR_REQUEST,
	VAR_PROTO,
	VAR_METHOD,
	VAR_PATH_INFO,
	VAR_USER,
	VAR_HTTPS,
	VAR_REDIRECT,
	VAR_SERVER_NAME,
	VAR_SERVER_ADDR,
	VAR_SERVER_PORT,
	VAR_REMOTE_NAME,
	VAR_REMOTE_ADDR,
	VAR_REMOTE_PORT,

	__VAR_MAX,
};

static char local_addr[INET6_ADDRSTRLEN], remote_addr[INET6_ADDRSTRLEN];
static char local_port[6], remote_port[6];
static char redirect_status[4];

static struct env_var extra_vars[] = {
	[_VAR_GW] = { "GATEWAY_INTERFACE", "CGI/1.1" },
	[_VAR_SOFTWARE] = { "SERVER_SOFTWARE", "uhttpd" },
	[VAR_SCRIPT_NAME] = { "SCRIPT_NAME" },
	[VAR_SCRIPT_FILE] = { "SCRIPT_FILENAME" },
	[VAR_DOCROOT] = { "DOCUMENT_ROOT" },
	[VAR_QUERY] = { "QUERY_STRING" },
	[VAR_REQUEST] = { "REQUEST_URI" },
	[VAR_PROTO] = { "SERVER_PROTOCOL" },
	[VAR_METHOD] = { "REQUEST_METHOD" },
	[VAR_PATH_INFO] = { "PATH_INFO" },
	[VAR_USER] = { "REMOTE_USER" },
	[VAR_HTTPS] = { "HTTPS" },
	[VAR_REDIRECT] = { "REDIRECT_STATUS", redirect_status },
	[VAR_SERVER_NAME] = { "SERVER_NAME", local_addr },
	[VAR_SERVER_ADDR] = { "SERVER_ADDR", local_addr },
	[VAR_SERVER_PORT] = { "SERVER_PORT", local_port },
	[VAR_REMOTE_NAME] = { "REMOTE_HOST", remote_addr },
	[VAR_REMOTE_ADDR] = { "REMOTE_ADDR", remote_addr },
	[VAR_REMOTE_PORT] = { "REMOTE_PORT", remote_port },
};

struct env_var *uh_get_process_vars(struct client *cl, struct path_info *pi)
{
	struct http_request *req = &cl->request;
	struct blob_attr *data = cl->hdr.head;
	struct env_var *vars = (void *) uh_buf;
	struct blob_attr *tb[__HDR_MAX];
	const char *url;
	int len;
	int i;

	url = blobmsg_data(blob_data(cl->hdr.head));
	len = ARRAY_SIZE(proc_header_env);
	len += ARRAY_SIZE(extra_vars);
	len *= sizeof(struct env_var);

	BUILD_BUG_ON(sizeof(uh_buf) < len);

	extra_vars[VAR_SCRIPT_NAME].value = pi->name;
	extra_vars[VAR_SCRIPT_FILE].value = pi->phys;
	extra_vars[VAR_DOCROOT].value = pi->root;
	extra_vars[VAR_QUERY].value = pi->query ? pi->query : "";
	extra_vars[VAR_REQUEST].value = url;
	extra_vars[VAR_PROTO].value = http_versions[req->version];
	extra_vars[VAR_METHOD].value = http_methods[req->method];
	extra_vars[VAR_PATH_INFO].value = pi->info;
	extra_vars[VAR_USER].value = req->realm ? req->realm->user : NULL;
	extra_vars[VAR_HTTPS].value = cl->tls ? "on" : NULL;

	snprintf(redirect_status, sizeof(redirect_status),
		 "%d", req->redirect_status);
	inet_ntop(cl->srv_addr.family, &cl->srv_addr.in, local_addr, sizeof(local_addr));
	snprintf(local_port, sizeof(local_port), "%d", cl->srv_addr.port);
	inet_ntop(cl->peer_addr.family, &cl->peer_addr.in, remote_addr, sizeof(remote_addr));
	snprintf(remote_port, sizeof(remote_port), "%d", cl->peer_addr.port);

	blobmsg_parse(hdr_policy, __HDR_MAX, tb, blob_data(data), blob_len(data));
	for (i = 0; i < ARRAY_SIZE(proc_header_env); i++) {
		struct blob_attr *cur;

		cur = tb[proc_header_env[i].idx];
		vars[i].name = proc_header_env[i].name;
		vars[i].value = cur ? blobmsg_data(cur) : NULL;
	}

	memcpy(&vars[i], extra_vars, sizeof(extra_vars));
	i += ARRAY_SIZE(extra_vars);
	vars[i].name = NULL;
	vars[i].value = NULL;

	return vars;
}

static void proc_close_fds(struct client *cl)
{
	struct dispatch_proc *p = &cl->dispatch.proc;

	close(p->r.sfd.fd.fd);
	if (p->wrfd.fd >= 0)
		close(p->wrfd.fd);
}

static void proc_handle_close(struct relay *r, int ret)
{
	if (r->header_cb) {
		uh_client_error(r->cl, 502, "Bad Gateway",
				"The process did not produce any response");
		return;
	}

	uh_request_done(r->cl);
}

static void proc_handle_header(struct relay *r, const char *name, const char *val)
{
	static char status_buf[64];
	struct client *cl = r->cl;
	char *sep;
	char buf[4];

	if (!strcmp(name, "Status")) {
		sep = strchr(val, ' ');
		if (sep != val + 3)
			return;

		memcpy(buf, val, 3);
		buf[3] = 0;
		snprintf(status_buf, sizeof(status_buf), "%s", sep + 1);
		cl->dispatch.proc.status_msg = status_buf;
		cl->dispatch.proc.status_code = atoi(buf);
		return;
	}

	blobmsg_add_string(&cl->dispatch.proc.hdr, name, val);
}

static void proc_handle_header_end(struct relay *r)
{
	struct client *cl = r->cl;
	struct dispatch_proc *p = &cl->dispatch.proc;
	struct blob_attr *cur;
	int rem;

	uloop_timeout_cancel(&p->timeout);
	uh_http_header(cl, cl->dispatch.proc.status_code, cl->dispatch.proc.status_msg);
	blob_for_each_attr(cur, cl->dispatch.proc.hdr.head, rem)
		ustream_printf(cl->us, "%s: %s\r\n", blobmsg_name(cur),
			       blobmsg_get_string(cur));

	ustream_printf(cl->us, "\r\n");

	if (cl->request.method == UH_HTTP_MSG_HEAD)
		r->skip_data = true;
}

static void proc_write_close(struct client *cl)
{
	struct dispatch_proc *p = &cl->dispatch.proc;

	if (p->wrfd.fd < 0)
		return;

	uloop_fd_delete(&p->wrfd);
	close(p->wrfd.fd);
	p->wrfd.fd = -1;
}

static void proc_free(struct client *cl)
{
	struct dispatch_proc *p = &cl->dispatch.proc;

	uloop_timeout_cancel(&p->timeout);
	blob_buf_free(&p->hdr);
	proc_write_close(cl);
	uh_relay_free(&p->r);
}

static void proc_write_cb(struct uloop_fd *fd, unsigned int events)
{
	struct client *cl = container_of(fd, struct client, dispatch.proc.wrfd);

	client_poll_post_data(cl);
}

static void proc_relay_write_cb(struct client *cl)
{
	struct dispatch_proc *p = &cl->dispatch.proc;

	if (ustream_pending_data(cl->us, true))
		return;

	ustream_set_read_blocked(&p->r.sfd.stream, false);
	p->r.sfd.stream.notify_read(&p->r.sfd.stream, 0);
}

static int proc_data_send(struct client *cl, const char *data, int len)
{
	struct dispatch_proc *p = &cl->dispatch.proc;
	int retlen = 0;
	int ret;

	while (len) {
		ret = write(p->wrfd.fd, data, len);

		if (ret < 0) {
			if (errno == EINTR)
				continue;

			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;

			/* consume all data */
			ret = len;
		}

		if (!ret)
			break;

		retlen += ret;
		len -= ret;
		data += ret;
	}

	if (len)
		uloop_fd_add(&p->wrfd, ULOOP_WRITE);
	else
		uloop_fd_delete(&p->wrfd);

	return retlen;
}

static void proc_timeout_cb(struct uloop_timeout *timeout)
{
	struct dispatch_proc *proc = container_of(timeout, struct dispatch_proc, timeout);
	struct client *cl = container_of(proc, struct client, dispatch.proc);

	uh_relay_kill(cl, &proc->r);
}

bool uh_create_process(struct client *cl, struct path_info *pi, char *url,
		       void (*cb)(struct client *cl, struct path_info *pi, char *url))
{
	struct dispatch *d = &cl->dispatch;
	struct dispatch_proc *proc = &d->proc;
	int rfd[2], wfd[2];
	int pid;

	blob_buf_init(&proc->hdr, 0);
	proc->status_code = 200;
	proc->status_msg = "OK";

	if (pipe(rfd))
		return false;

	if (pipe(wfd))
		goto close_rfd;

	pid = fork();
	if (pid < 0)
		goto close_wfd;

	if (!pid) {
		close(0);
		close(1);

		dup2(rfd[1], 1);
		dup2(wfd[0], 0);

		close(rfd[0]);
		close(rfd[1]);
		close(wfd[0]);
		close(wfd[1]);

		uh_close_fds();
		cb(cl, pi, url);
		exit(0);
	}

	close(rfd[1]);
	close(wfd[0]);

	proc->wrfd.fd = wfd[1];
	uh_relay_open(cl, &proc->r, rfd[0], pid);

	d->free = proc_free;
	d->close_fds = proc_close_fds;
	d->data_send = proc_data_send;
	d->data_done = proc_write_close;
	d->write_cb = proc_relay_write_cb;
	proc->r.header_cb = proc_handle_header;
	proc->r.header_end = proc_handle_header_end;
	proc->r.close = proc_handle_close;
	proc->wrfd.cb = proc_write_cb;
	proc->timeout.cb = proc_timeout_cb;
	if (conf.script_timeout > 0)
		uloop_timeout_set(&proc->timeout, conf.script_timeout * 1000);

	return true;

close_wfd:
	close(wfd[0]);
	close(wfd[1]);
close_rfd:
	close(rfd[0]);
	close(rfd[1]);

	return false;
}
