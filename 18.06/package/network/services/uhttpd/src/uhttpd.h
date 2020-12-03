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

#ifndef __UHTTPD_H
#define __UHTTPD_H

#include <netinet/in.h>
#include <limits.h>
#include <dirent.h>

#include <libubox/list.h>
#include <libubox/uloop.h>
#include <libubox/ustream.h>
#include <libubox/blob.h>
#include <libubox/utils.h>
#ifdef HAVE_UBUS
#include <libubus.h>
#include <json-c/json.h>
#endif
#ifdef HAVE_TLS
#include <libubox/ustream-ssl.h>
#endif

#include "utils.h"

#define UH_LIMIT_CLIENTS	64

#define __enum_header(_name, _val) HDR_##_name,
#define __blobmsg_header(_name, _val) [HDR_##_name] = { .name = #_val, .type = BLOBMSG_TYPE_STRING },

struct client;

struct alias {
	struct list_head list;
	char *alias;
	char *path;
};

struct lua_prefix {
	struct list_head list;
	const char *handler;
	const char *prefix;
	void *ctx;
};

struct config {
	const char *docroot;
	const char *realm;
	const char *file;
	const char *error_handler;
	const char *cgi_prefix;
	const char *cgi_docroot_path;
	const char *cgi_path;
	const char *ubus_prefix;
	const char *ubus_socket;
	int no_symlinks;
	int no_dirlists;
	int network_timeout;
	int rfc1918_filter;
	int tls_redirect;
	int tcp_keepalive;
	int max_script_requests;
	int max_connections;
	int http_keepalive;
	int script_timeout;
	int ubus_noauth;
	int ubus_cors;
	int cgi_prefix_len;
	struct list_head cgi_alias;
	struct list_head lua_prefix;
};

struct auth_realm {
	struct list_head list;
	const char *path;
	const char *user;
	const char *pass;
};

enum http_method {
	UH_HTTP_MSG_GET,
	UH_HTTP_MSG_POST,
	UH_HTTP_MSG_HEAD,
	UH_HTTP_MSG_OPTIONS,
	UH_HTTP_MSG_PUT,
	UH_HTTP_MSG_PATCH,
	UH_HTTP_MSG_DELETE,
};

enum http_version {
	UH_HTTP_VER_0_9,
	UH_HTTP_VER_1_0,
	UH_HTTP_VER_1_1,
};

enum http_user_agent {
	UH_UA_UNKNOWN,
	UH_UA_GECKO,
	UH_UA_CHROME,
	UH_UA_SAFARI,
	UH_UA_MSIE,
	UH_UA_KONQUEROR,
	UH_UA_OPERA,
	UH_UA_MSIE_OLD,
	UH_UA_MSIE_NEW,
};

struct http_request {
	enum http_method method;
	enum http_version version;
	enum http_user_agent ua;
	int redirect_status;
	int content_length;
	bool expect_cont;
	bool connection_close;
	bool disable_chunked;
	uint8_t transfer_chunked;
	const struct auth_realm *realm;
};

enum client_state {
	CLIENT_STATE_INIT,
	CLIENT_STATE_HEADER,
	CLIENT_STATE_DATA,
	CLIENT_STATE_DONE,
	CLIENT_STATE_CLOSE,
	CLIENT_STATE_CLEANUP,
};

struct interpreter {
	struct list_head list;
	const char *path;
	const char *ext;
};

struct path_info {
	const char *root;
	const char *phys;
	const char *name;
	const char *info;
	const char *query;
	bool redirected;
	struct stat stat;
	const struct interpreter *ip;
};

struct env_var {
	const char *name;
	const char *value;
};

struct relay {
	struct ustream_fd sfd;
	struct uloop_process proc;
	struct uloop_timeout timeout;
	struct client *cl;

	bool process_done;
	bool error;
	bool skip_data;

	int ret;
	int header_ofs;

	void (*header_cb)(struct relay *r, const char *name, const char *value);
	void (*header_end)(struct relay *r);
	void (*close)(struct relay *r, int ret);
};

struct dispatch_proc {
	struct uloop_timeout timeout;
	struct blob_buf hdr;
	struct uloop_fd wrfd;
	struct relay r;
	int status_code;
	char *status_msg;
};

struct dispatch_handler {
	struct list_head list;
	bool script;

	bool (*check_url)(const char *url);
	bool (*check_path)(struct path_info *pi, const char *url);
	void (*handle_request)(struct client *cl, char *url, struct path_info *pi);
};

#ifdef HAVE_UBUS
struct dispatch_ubus {
	struct ubus_request req;

	struct uloop_timeout timeout;
	struct json_tokener *jstok;
	struct json_object *jsobj;
	struct json_object *jsobj_cur;
	int post_len;

	uint32_t obj;
	const char *func;

	struct blob_buf buf;
	bool req_pending;
	bool array;
	int array_idx;
};
#endif

struct dispatch {
	int (*data_send)(struct client *cl, const char *data, int len);
	void (*data_done)(struct client *cl);
	void (*write_cb)(struct client *cl);
	void (*close_fds)(struct client *cl);
	void (*free)(struct client *cl);

	void *req_data;
	void (*req_free)(struct client *cl);

	bool data_blocked;
	bool no_cache;

	union {
		struct {
			struct blob_attr **hdr;
			int fd;
		} file;
		struct dispatch_proc proc;
#ifdef HAVE_UBUS
		struct dispatch_ubus ubus;
#endif
	};
};

struct client {
	struct list_head list;
	int refcount;
	int id;

	struct ustream *us;
	struct ustream_fd sfd;
#ifdef HAVE_TLS
	struct ustream_ssl ssl;
#endif
	struct uloop_timeout timeout;
	int requests;

	enum client_state state;
	bool tls;

	int http_code;
	struct http_request request;
	struct uh_addr srv_addr, peer_addr;

	struct blob_buf hdr;
	struct blob_buf hdr_response;
	struct dispatch dispatch;
};

extern char uh_buf[4096];
extern int n_clients;
extern struct config conf;
extern const char * const http_versions[];
extern const char * const http_methods[];
extern struct dispatch_handler cgi_dispatch;

void uh_index_add(const char *filename);

bool uh_accept_client(int fd, bool tls);

void uh_unblock_listeners(void);
void uh_setup_listeners(void);
int uh_socket_bind(const char *host, const char *port, bool tls);

int uh_first_tls_port(int family);

bool uh_use_chunked(struct client *cl);
void uh_chunk_write(struct client *cl, const void *data, int len);
void uh_chunk_vprintf(struct client *cl, const char *format, va_list arg);

void __printf(2, 3)
uh_chunk_printf(struct client *cl, const char *format, ...);

void uh_chunk_eof(struct client *cl);
void uh_request_done(struct client *cl);

void uh_http_header(struct client *cl, int code, const char *summary);
void __printf(4, 5)
uh_client_error(struct client *cl, int code, const char *summary, const char *fmt, ...);

void uh_handle_request(struct client *cl);
void client_poll_post_data(struct client *cl);
void uh_client_read_cb(struct client *cl);
void uh_client_notify_state(struct client *cl);

void uh_auth_add(const char *path, const char *user, const char *pass);
bool uh_auth_check(struct client *cl, const char *path, const char *auth,
                   char **uptr, char **pptr);

void uh_close_listen_fds(void);
void uh_close_fds(void);

void uh_interpreter_add(const char *ext, const char *path);
void uh_dispatch_add(struct dispatch_handler *d);

void uh_relay_open(struct client *cl, struct relay *r, int fd, int pid);
void uh_relay_close(struct relay *r, int ret);
void uh_relay_free(struct relay *r);
void uh_relay_kill(struct client *cl, struct relay *r);

struct env_var *uh_get_process_vars(struct client *cl, struct path_info *pi);
bool uh_create_process(struct client *cl, struct path_info *pi, char *url,
		       void (*cb)(struct client *cl, struct path_info *pi, char *url));

int uh_plugin_init(const char *name);
void uh_plugin_post_init(void);

int uh_handler_add(const char *file);
int uh_handler_run(struct client *cl, char **url, bool fallback);

struct path_info *uh_path_lookup(struct client *cl, const char *url);

static inline void uh_client_ref(struct client *cl)
{
	cl->refcount++;
}

static inline void uh_client_unref(struct client *cl)
{
	if (--cl->refcount)
		return;

	if (cl->state == CLIENT_STATE_CLEANUP)
		ustream_state_change(cl->us);
}

#endif
