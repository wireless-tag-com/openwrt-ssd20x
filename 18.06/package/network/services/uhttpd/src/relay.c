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

#include <signal.h>
#include "uhttpd.h"

void uh_relay_free(struct relay *r)
{
	if (!r->cl)
		return;

	if (r->proc.pending)
		kill(r->proc.pid, SIGKILL);

	uloop_timeout_cancel(&r->timeout);
	uloop_process_delete(&r->proc);
	ustream_free(&r->sfd.stream);
	close(r->sfd.fd.fd);

	r->cl = NULL;
}

void uh_relay_close(struct relay *r, int ret)
{
	struct ustream *us = &r->sfd.stream;

	if (!us->notify_read)
		return;

	us->notify_read = NULL;
	us->notify_write = NULL;
	us->notify_state = NULL;

	if (r->close)
		r->close(r, ret);
}

static void relay_error(struct relay *r)
{
	struct ustream *s = &r->sfd.stream;
	int len;

	r->error = true;
	s->eof = true;
	ustream_get_read_buf(s, &len);
	if (len)
		ustream_consume(s, len);
	ustream_state_change(s);
}

static void relay_process_headers(struct relay *r)
{
	struct ustream *s = &r->sfd.stream;
	char *buf, *newline;
	int len;

	if (!r->header_cb)
		return;

	while (r->header_cb) {
		int line_len;
		char *val;

		buf = ustream_get_read_buf(s, &len);
		if (!buf || !len)
			break;

		newline = strchr(buf, '\n');
		if (!newline)
			break;

		line_len = newline + 1 - buf;
		if (newline > buf && newline[-1] == '\r')
			newline--;

		*newline = 0;
		if (newline == buf) {
			r->header_cb = NULL;
			if (r->header_end)
				r->header_end(r);
			ustream_consume(s, line_len);
			break;
		}

		val = uh_split_header(buf);
		if (!val) {
			relay_error(r);
			return;
		}

		r->header_cb(r, buf, val);
		ustream_consume(s, line_len);
	}
}

static void relay_read_cb(struct ustream *s, int bytes)
{
	struct relay *r = container_of(s, struct relay, sfd.stream);
	struct client *cl = r->cl;
	struct ustream *us = cl->us;
	char *buf;
	int len;

	if (r->process_done)
		uloop_timeout_set(&r->timeout, 1);

	if (!r->error)
		relay_process_headers(r);

	if (r->header_cb) {
		/*
		 * if eof, ensure that remaining data is discarded, so the
		 * state change cb will tear down the stream
		 */
		if (s->eof)
			relay_error(r);
		return;
	}

	if (!s->eof && ustream_pending_data(us, true)) {
		ustream_set_read_blocked(s, true);
		return;
	}

	buf = ustream_get_read_buf(s, &len);
	if (!buf || !len)
		return;

	if (!r->skip_data)
		uh_chunk_write(cl, buf, len);

	ustream_consume(s, len);
}

static void relay_close_if_done(struct uloop_timeout *timeout)
{
	struct relay *r = container_of(timeout, struct relay, timeout);
	struct ustream *s = &r->sfd.stream;

	while (ustream_poll(&r->sfd.stream));

	if (!(r->process_done || s->eof) || ustream_pending_data(s, false))
		return;

	uh_relay_close(r, r->ret);
}

static void relay_state_cb(struct ustream *s)
{
	struct relay *r = container_of(s, struct relay, sfd.stream);

	if (r->process_done)
		uloop_timeout_set(&r->timeout, 1);
}

static void relay_proc_cb(struct uloop_process *proc, int ret)
{
	struct relay *r = container_of(proc, struct relay, proc);

	r->process_done = true;
	r->ret = ret;
	uloop_timeout_set(&r->timeout, 1);
}

void uh_relay_kill(struct client *cl, struct relay *r)
{
	struct ustream *us = &r->sfd.stream;

	kill(r->proc.pid, SIGKILL);
	us->eof = true;
	ustream_state_change(us);
}

void uh_relay_open(struct client *cl, struct relay *r, int fd, int pid)
{
	struct ustream *us = &r->sfd.stream;

	r->cl = cl;
	us->notify_read = relay_read_cb;
	us->notify_state = relay_state_cb;
	us->string_data = true;
	ustream_fd_init(&r->sfd, fd);

	r->proc.pid = pid;
	r->proc.cb = relay_proc_cb;
	uloop_process_add(&r->proc);

	r->timeout.cb = relay_close_if_done;
}
