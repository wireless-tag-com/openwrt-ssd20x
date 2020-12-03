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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netdb.h>

#include "uhttpd.h"

struct listener {
	struct list_head list;
	struct uloop_fd fd;
	int socket;
	int n_clients;
	struct sockaddr_in6 addr;
	bool tls;
	bool blocked;
};

static LIST_HEAD(listeners);
static int n_blocked;

void uh_close_listen_fds(void)
{
	struct listener *l;

	list_for_each_entry(l, &listeners, list)
		close(l->fd.fd);
}

static void uh_block_listener(struct listener *l)
{
	uloop_fd_delete(&l->fd);
	n_blocked++;
	l->blocked = true;
}

static void uh_poll_listeners(struct uloop_timeout *timeout)
{
	struct listener *l;

	if ((!n_blocked && conf.max_connections) ||
	    n_clients >= conf.max_connections)
		return;

	list_for_each_entry(l, &listeners, list) {
		if (!l->blocked)
			continue;

		l->fd.cb(&l->fd, ULOOP_READ);
	    if (n_clients >= conf.max_connections)
			break;

		n_blocked--;
		l->blocked = false;
		uloop_fd_add(&l->fd, ULOOP_READ);
	}
}

void uh_unblock_listeners(void)
{
	static struct uloop_timeout poll_timer = {
		.cb = uh_poll_listeners
	};

	uloop_timeout_set(&poll_timer, 1);
}

static void listener_cb(struct uloop_fd *fd, unsigned int events)
{
	struct listener *l = container_of(fd, struct listener, fd);

	while (1) {
		if (!uh_accept_client(fd->fd, l->tls))
			break;
	}

	if (conf.max_connections && n_clients >= conf.max_connections)
		uh_block_listener(l);
}

void uh_setup_listeners(void)
{
	struct listener *l;
	int yes = 1;

	list_for_each_entry(l, &listeners, list) {
		int sock = l->fd.fd;

		/* TCP keep-alive */
		if (conf.tcp_keepalive > 0) {
#ifdef linux
			int tcp_ka_idl, tcp_ka_int, tcp_ka_cnt, tcp_fstopn;

			tcp_ka_idl = 1;
			tcp_ka_cnt = 3;
			tcp_ka_int = conf.tcp_keepalive;
			tcp_fstopn = 5;

			setsockopt(sock, SOL_TCP, TCP_KEEPIDLE,  &tcp_ka_idl, sizeof(tcp_ka_idl));
			setsockopt(sock, SOL_TCP, TCP_KEEPINTVL, &tcp_ka_int, sizeof(tcp_ka_int));
			setsockopt(sock, SOL_TCP, TCP_KEEPCNT,   &tcp_ka_cnt, sizeof(tcp_ka_cnt));
			setsockopt(sock, SOL_TCP, TCP_FASTOPEN,  &tcp_fstopn, sizeof(tcp_fstopn));
#endif

			setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes));
		}

		l->fd.cb = listener_cb;
		uloop_fd_add(&l->fd, ULOOP_READ);
	}
}

int uh_socket_bind(const char *host, const char *port, bool tls)
{
	int sock = -1;
	int yes = 1;
	int status;
	int bound = 0;
	struct listener *l = NULL;
	struct addrinfo *addrs = NULL, *p = NULL;
	static struct addrinfo hints = {
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_STREAM,
		.ai_flags = AI_PASSIVE,
	};

	if ((status = getaddrinfo(host, port, &hints, &addrs)) != 0) {
		fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(status));
		return 0;
	}

	/* try to bind a new socket to each found address */
	for (p = addrs; p; p = p->ai_next) {
		/* get the socket */
		sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (sock < 0) {
			perror("socket()");
			goto error;
		}

		/* "address already in use" */
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) {
			perror("setsockopt()");
			goto error;
		}

		/* required to get parallel v4 + v6 working */
		if (p->ai_family == AF_INET6 &&
		    setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(yes)) < 0) {
			perror("setsockopt()");
			goto error;
		}

		/* bind */
		if (bind(sock, p->ai_addr, p->ai_addrlen) < 0) {
			perror("bind()");
			goto error;
		}

		/* listen */
		if (listen(sock, UH_LIMIT_CLIENTS) < 0) {
			perror("listen()");
			goto error;
		}

		fd_cloexec(sock);

		l = calloc(1, sizeof(*l));
		if (!l)
			goto error;

		l->fd.fd = sock;
		l->tls = tls;
		l->addr = *(struct sockaddr_in6 *)p->ai_addr;
		list_add_tail(&l->list, &listeners);
		bound++;

		continue;

error:
		if (sock > -1)
			close(sock);
	}

	freeaddrinfo(addrs);

	return bound;
}

int uh_first_tls_port(int family)
{
	struct listener *l;
	int tls_port = -1;

	list_for_each_entry(l, &listeners, list) {
		if (!l->tls || l->addr.sin6_family != family)
			continue;

		if (tls_port != -1 && ntohs(l->addr.sin6_port) != 443)
			continue;

		tls_port = ntohs(l->addr.sin6_port);
	}

	return tls_port;
}
