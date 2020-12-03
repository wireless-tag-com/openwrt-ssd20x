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

#ifndef _DEFAULT_SOURCE
# define _DEFAULT_SOURCE
#endif

#define _BSD_SOURCE
#define _DARWIN_C_SOURCE
#define _XOPEN_SOURCE 700

#include <sys/types.h>
#include <sys/dir.h>
#include <time.h>
#include <strings.h>
#include <dirent.h>
#include <inttypes.h>

#include <libubox/blobmsg.h>

#include "uhttpd.h"
#include "mimetypes.h"

#define MAX(a, b)	(((a) > (b)) ? (a) : (b))

static LIST_HEAD(index_files);
static LIST_HEAD(dispatch_handlers);
static LIST_HEAD(pending_requests);
static int n_requests;

struct deferred_request {
	struct list_head list;
	struct dispatch_handler *d;
	struct client *cl;
	struct path_info pi;
	bool called, path;
};

struct index_file {
	struct list_head list;
	const char *name;
};

enum file_hdr {
	HDR_AUTHORIZATION,
	HDR_IF_MODIFIED_SINCE,
	HDR_IF_UNMODIFIED_SINCE,
	HDR_IF_MATCH,
	HDR_IF_NONE_MATCH,
	HDR_IF_RANGE,
	__HDR_MAX
};

void uh_index_add(const char *filename)
{
	struct index_file *idx;

	idx = calloc(1, sizeof(*idx));
	idx->name = filename;
	list_add_tail(&idx->list, &index_files);
}

static char * canonpath(const char *path, char *path_resolved)
{
	const char *path_cpy = path;
	char *path_res = path_resolved;

	if (conf.no_symlinks)
		return realpath(path, path_resolved);

	/* normalize */
	while ((*path_cpy != '\0') && (path_cpy < (path + PATH_MAX - 2))) {
		if (*path_cpy != '/')
			goto next;

		/* skip repeating / */
		if (path_cpy[1] == '/') {
			path_cpy++;
			continue;
		}

		/* /./ or /../ */
		if (path_cpy[1] == '.') {
			/* skip /./ */
			if ((path_cpy[2] == '/') || (path_cpy[2] == '\0')) {
				path_cpy += 2;
				continue;
			}

			/* collapse /x/../ */
			if ((path_cpy[2] == '.') &&
			    ((path_cpy[3] == '/') || (path_cpy[3] == '\0'))) {
				while ((path_res > path_resolved) && (*--path_res != '/'));

				path_cpy += 3;
				continue;
			}
		}

next:
		*path_res++ = *path_cpy++;
	}

	/* remove trailing slash if not root / */
	if ((path_res > (path_resolved+1)) && (path_res[-1] == '/'))
		path_res--;
	else if (path_res == path_resolved)
		*path_res++ = '/';

	*path_res = '\0';

	return path_resolved;
}

/* Returns NULL on error.
** NB: improperly encoded URL should give client 400 [Bad Syntax]; returning
** NULL here causes 404 [Not Found], but that's not too unreasonable. */
struct path_info *
uh_path_lookup(struct client *cl, const char *url)
{
	static char path_phys[PATH_MAX];
	static char path_info[PATH_MAX];
	static char path_query[PATH_MAX];
	static struct path_info p;

	const char *docroot = conf.docroot;
	int docroot_len = strlen(docroot);
	char *pathptr = NULL;
	bool slash;

	int i = 0;
	int len;
	struct stat s;
	struct index_file *idx;

	/* back out early if url is undefined */
	if (url == NULL)
		return NULL;

	memset(&p, 0, sizeof(p));
	path_phys[0] = 0;
	path_info[0] = 0;

	strcpy(uh_buf, docroot);

	/* separate query string from url */
	if ((pathptr = strchr(url, '?')) != NULL) {
		if (pathptr[1]) {
			p.query = path_query;
			snprintf(path_query, sizeof(path_query), "%s",
				 pathptr + 1);
		}

		/* urldecode component w/o query */
		if (pathptr > url) {
			if (uh_urldecode(&uh_buf[docroot_len],
					 sizeof(uh_buf) - docroot_len - 1,
					 url, pathptr - url ) < 0)
				return NULL;
		}
	}

	/* no query string, decode all of url */
	else if (uh_urldecode(&uh_buf[docroot_len],
			      sizeof(uh_buf) - docroot_len - 1,
			      url, strlen(url) ) < 0)
		return NULL;

	/* create canon path */
	len = strlen(uh_buf);
	slash = len && uh_buf[len - 1] == '/';
	len = min(len, sizeof(path_phys) - 1);

	for (i = len; i >= 0; i--) {
		char ch = uh_buf[i];
		bool exists;

		if (ch != 0 && ch != '/')
			continue;

		uh_buf[i] = 0;
		exists = !!canonpath(uh_buf, path_phys);
		uh_buf[i] = ch;

		if (!exists)
			continue;

		/* test current path */
		if (stat(path_phys, &p.stat))
			continue;

		snprintf(path_info, sizeof(path_info), "%s", uh_buf + i);
		break;
	}

	/* check whether found path is within docroot */
	if (strncmp(path_phys, docroot, docroot_len) != 0 ||
	    (path_phys[docroot_len] != 0 &&
	     path_phys[docroot_len] != '/'))
		return NULL;

	/* is a regular file */
	if (p.stat.st_mode & S_IFREG) {
		p.root = docroot;
		p.phys = path_phys;
		p.name = &path_phys[docroot_len];
		p.info = path_info[0] ? path_info : NULL;
		return &p;
	}

	if (!(p.stat.st_mode & S_IFDIR))
		return NULL;

	if (path_info[0])
	    return NULL;

	pathptr = path_phys + strlen(path_phys);

	/* ensure trailing slash */
	if (pathptr[-1] != '/') {
		pathptr[0] = '/';
		pathptr[1] = 0;
		pathptr++;
	}

	/* if requested url resolves to a directory and a trailing slash
	   is missing in the request url, redirect the client to the same
	   url with trailing slash appended */
	if (!slash) {
		uh_http_header(cl, 302, "Found");
		if (!uh_use_chunked(cl))
			ustream_printf(cl->us, "Content-Length: 0\r\n");
		ustream_printf(cl->us, "Location: %s%s%s\r\n\r\n",
				&path_phys[docroot_len],
				p.query ? "?" : "",
				p.query ? p.query : "");
		uh_request_done(cl);
		p.redirected = 1;
		return &p;
	}

	/* try to locate index file */
	len = path_phys + sizeof(path_phys) - pathptr - 1;
	list_for_each_entry(idx, &index_files, list) {
		if (strlen(idx->name) > len)
			continue;

		strcpy(pathptr, idx->name);
		if (!stat(path_phys, &s) && (s.st_mode & S_IFREG)) {
			memcpy(&p.stat, &s, sizeof(p.stat));
			break;
		}

		*pathptr = 0;
	}

	p.root = docroot;
	p.phys = path_phys;
	p.name = &path_phys[docroot_len];

	return p.phys ? &p : NULL;
}

static const char * uh_file_mime_lookup(const char *path)
{
	const struct mimetype *m = &uh_mime_types[0];
	const char *e;

	while (m->extn) {
		e = &path[strlen(path)-1];

		while (e >= path) {
			if ((*e == '.' || *e == '/') && !strcasecmp(&e[1], m->extn))
				return m->mime;

			e--;
		}

		m++;
	}

	return "application/octet-stream";
}

static const char * uh_file_mktag(struct stat *s, char *buf, int len)
{
	snprintf(buf, len, "\"%" PRIx64 "-%" PRIx64 "-%" PRIx64 "\"",
	         s->st_ino, s->st_size, (uint64_t)s->st_mtime);

	return buf;
}

static time_t uh_file_date2unix(const char *date)
{
	struct tm t;

	memset(&t, 0, sizeof(t));

	if (strptime(date, "%a, %d %b %Y %H:%M:%S %Z", &t) != NULL)
		return timegm(&t);

	return 0;
}

static char * uh_file_unix2date(time_t ts, char *buf, int len)
{
	struct tm *t = gmtime(&ts);

	strftime(buf, len, "%a, %d %b %Y %H:%M:%S GMT", t);

	return buf;
}

static char *uh_file_header(struct client *cl, int idx)
{
	if (!cl->dispatch.file.hdr[idx])
		return NULL;

	return (char *) blobmsg_data(cl->dispatch.file.hdr[idx]);
}

static void uh_file_response_ok_hdrs(struct client *cl, struct stat *s)
{
	char buf[128];

	if (s) {
		ustream_printf(cl->us, "ETag: %s\r\n", uh_file_mktag(s, buf, sizeof(buf)));
		ustream_printf(cl->us, "Last-Modified: %s\r\n",
			       uh_file_unix2date(s->st_mtime, buf, sizeof(buf)));
	}
	ustream_printf(cl->us, "Date: %s\r\n",
		       uh_file_unix2date(time(NULL), buf, sizeof(buf)));
}

static void uh_file_response_200(struct client *cl, struct stat *s)
{
	uh_http_header(cl, 200, "OK");
	return uh_file_response_ok_hdrs(cl, s);
}

static void uh_file_response_304(struct client *cl, struct stat *s)
{
	uh_http_header(cl, 304, "Not Modified");

	return uh_file_response_ok_hdrs(cl, s);
}

static void uh_file_response_405(struct client *cl)
{
	uh_http_header(cl, 405, "Method Not Allowed");
}

static void uh_file_response_412(struct client *cl)
{
	uh_http_header(cl, 412, "Precondition Failed");
}

static bool uh_file_if_match(struct client *cl, struct stat *s)
{
	char buf[128];
	const char *tag = uh_file_mktag(s, buf, sizeof(buf));
	char *hdr = uh_file_header(cl, HDR_IF_MATCH);
	char *p;
	int i;

	if (!hdr)
		return true;

	p = &hdr[0];
	for (i = 0; i < strlen(hdr); i++)
	{
		if ((hdr[i] == ' ') || (hdr[i] == ',')) {
			hdr[i++] = 0;
			p = &hdr[i];
		} else if (!strcmp(p, "*") || !strcmp(p, tag)) {
			return true;
		}
	}

	uh_file_response_412(cl);
	return false;
}

static int uh_file_if_modified_since(struct client *cl, struct stat *s)
{
	char *hdr = uh_file_header(cl, HDR_IF_MODIFIED_SINCE);

	if (!hdr)
		return true;

	if (uh_file_date2unix(hdr) >= s->st_mtime) {
		uh_file_response_304(cl, s);
		return false;
	}

	return true;
}

static int uh_file_if_none_match(struct client *cl, struct stat *s)
{
	char buf[128];
	const char *tag = uh_file_mktag(s, buf, sizeof(buf));
	char *hdr = uh_file_header(cl, HDR_IF_NONE_MATCH);
	char *p;
	int i;

	if (!hdr)
		return true;

	p = &hdr[0];
	for (i = 0; i < strlen(hdr); i++) {
		if ((hdr[i] == ' ') || (hdr[i] == ',')) {
			hdr[i++] = 0;
			p = &hdr[i];
		} else if (!strcmp(p, "*") || !strcmp(p, tag)) {
			if ((cl->request.method == UH_HTTP_MSG_GET) ||
				(cl->request.method == UH_HTTP_MSG_HEAD))
				uh_file_response_304(cl, s);
			else
				uh_file_response_412(cl);

			return false;
		}
	}

	return true;
}

static int uh_file_if_range(struct client *cl, struct stat *s)
{
	char *hdr = uh_file_header(cl, HDR_IF_RANGE);

	if (hdr) {
		uh_file_response_412(cl);
		return false;
	}

	return true;
}

static int uh_file_if_unmodified_since(struct client *cl, struct stat *s)
{
	char *hdr = uh_file_header(cl, HDR_IF_UNMODIFIED_SINCE);

	if (hdr && uh_file_date2unix(hdr) <= s->st_mtime) {
		uh_file_response_412(cl);
		return false;
	}

	return true;
}

static int dirent_cmp(const struct dirent **a, const struct dirent **b)
{
	bool dir_a = !!((*a)->d_type & DT_DIR);
	bool dir_b = !!((*b)->d_type & DT_DIR);

	/* directories first */
	if (dir_a != dir_b)
		return dir_b - dir_a;

	return alphasort(a, b);
}

static void list_entries(struct client *cl, struct dirent **files, int count,
			 const char *path, char *local_path)
{
	const char *suffix = "/";
	const char *type = "directory";
	unsigned int mode = S_IXOTH;
	struct stat s;
	char *escaped;
	char *file;
	char buf[128];
	int i;

	file = local_path + strlen(local_path);
	for (i = 0; i < count; i++) {
		const char *name = files[i]->d_name;
		bool dir = !!(files[i]->d_type & DT_DIR);

		if (name[0] == '.' && name[1] == 0)
			goto next;

		sprintf(file, "%s", name);
		if (stat(local_path, &s))
			goto next;

		if (!dir) {
			suffix = "";
			mode = S_IROTH;
			type = uh_file_mime_lookup(local_path);
		}

		if (!(s.st_mode & mode))
			goto next;

		escaped = uh_htmlescape(name);

		if (!escaped)
			goto next;

		uh_chunk_printf(cl,
				"<li><strong><a href='%s%s%s'>%s</a>%s"
				"</strong><br /><small>modified: %s"
				"<br />%s - %.02f kbyte<br />"
				"<br /></small></li>",
				path, escaped, suffix,
				escaped, suffix,
				uh_file_unix2date(s.st_mtime, buf, sizeof(buf)),
				type, s.st_size / 1024.0);

		free(escaped);
		*file = 0;
next:
		free(files[i]);
	}
}

static void uh_file_dirlist(struct client *cl, struct path_info *pi)
{
	struct dirent **files = NULL;
	char *escaped_path = uh_htmlescape(pi->name);
	int count = 0;

	if (!escaped_path)
	{
		uh_client_error(cl, 500, "Internal Server Error", "Out of memory");
		return;
	}

	uh_file_response_200(cl, NULL);
	ustream_printf(cl->us, "Content-Type: text/html\r\n\r\n");

	uh_chunk_printf(cl,
		"<html><head><title>Index of %s</title></head>"
		"<body><h1>Index of %s</h1><hr /><ol>",
		escaped_path, escaped_path);

	count = scandir(pi->phys, &files, NULL, dirent_cmp);
	if (count > 0) {
		strcpy(uh_buf, pi->phys);
		list_entries(cl, files, count, escaped_path, uh_buf);
	}
	free(escaped_path);
	free(files);

	uh_chunk_printf(cl, "</ol><hr /></body></html>");
	uh_request_done(cl);
}

static void file_write_cb(struct client *cl)
{
	int fd = cl->dispatch.file.fd;
	int r;

	while (cl->us->w.data_bytes < 256) {
		r = read(fd, uh_buf, sizeof(uh_buf));
		if (r < 0) {
			if (errno == EINTR)
				continue;
		}

		if (!r) {
			uh_request_done(cl);
			return;
		}

		uh_chunk_write(cl, uh_buf, r);
	}
}

static void uh_file_free(struct client *cl)
{
	close(cl->dispatch.file.fd);
}

static void uh_file_data(struct client *cl, struct path_info *pi, int fd)
{
	/* test preconditions */
	if (!cl->dispatch.no_cache &&
	    (!uh_file_if_modified_since(cl, &pi->stat) ||
	     !uh_file_if_match(cl, &pi->stat) ||
	     !uh_file_if_range(cl, &pi->stat) ||
	     !uh_file_if_unmodified_since(cl, &pi->stat) ||
	     !uh_file_if_none_match(cl, &pi->stat))) {
		ustream_printf(cl->us, "\r\n");
		uh_request_done(cl);
		close(fd);
		return;
	}

	/* write status */
	uh_file_response_200(cl, &pi->stat);

	ustream_printf(cl->us, "Content-Type: %s\r\n",
			   uh_file_mime_lookup(pi->name));

	ustream_printf(cl->us, "Content-Length: %" PRIu64 "\r\n\r\n",
			   pi->stat.st_size);


	/* send body */
	if (cl->request.method == UH_HTTP_MSG_HEAD) {
		uh_request_done(cl);
		close(fd);
		return;
	}

	cl->dispatch.file.fd = fd;
	cl->dispatch.write_cb = file_write_cb;
	cl->dispatch.free = uh_file_free;
	cl->dispatch.close_fds = uh_file_free;
	file_write_cb(cl);
}

static bool __handle_file_request(struct client *cl, char *url);

static void uh_file_request(struct client *cl, const char *url,
			    struct path_info *pi, struct blob_attr **tb)
{
	int fd;
	struct http_request *req = &cl->request;
	char *error_handler, *escaped_url;

	switch (cl->request.method) {
	case UH_HTTP_MSG_GET:
	case UH_HTTP_MSG_POST:
	case UH_HTTP_MSG_HEAD:
	case UH_HTTP_MSG_OPTIONS:
		break;

	default:
		uh_file_response_405(cl);
		ustream_printf(cl->us, "\r\n");
		uh_request_done(cl);
		return;
	}

	if (!(pi->stat.st_mode & S_IROTH))
		goto error;

	if (pi->stat.st_mode & S_IFREG) {
		fd = open(pi->phys, O_RDONLY);
		if (fd < 0)
			goto error;

		req->disable_chunked = true;
		cl->dispatch.file.hdr = tb;
		uh_file_data(cl, pi, fd);
		cl->dispatch.file.hdr = NULL;
		return;
	}

	if ((pi->stat.st_mode & S_IFDIR)) {
		if (conf.no_dirlists)
			goto error;

		uh_file_dirlist(cl, pi);
		return;
	}

error:
	/* check for a previously set 403 redirect status to prevent infinite
	   recursion when the error page itself lacks sufficient permissions */
	if (conf.error_handler && req->redirect_status != 403) {
		req->redirect_status = 403;
		error_handler = alloca(strlen(conf.error_handler) + 1);
		strcpy(error_handler, conf.error_handler);
		if (__handle_file_request(cl, error_handler))
			return;
	}

	escaped_url = uh_htmlescape(url);

	uh_client_error(cl, 403, "Forbidden",
			"You don't have permission to access %s on this server.",
			escaped_url ? escaped_url : "the url");

	if (escaped_url)
		free(escaped_url);
}

void uh_dispatch_add(struct dispatch_handler *d)
{
	list_add_tail(&d->list, &dispatch_handlers);
}

static struct dispatch_handler *
dispatch_find(const char *url, struct path_info *pi)
{
	struct dispatch_handler *d;

	list_for_each_entry(d, &dispatch_handlers, list) {
		if (pi) {
			if (d->check_url)
				continue;

			if (d->check_path(pi, url))
				return d;
		} else {
			if (d->check_path)
				continue;

			if (d->check_url(url))
				return d;
		}
	}

	return NULL;
}

static void
uh_invoke_script(struct client *cl, struct dispatch_handler *d, struct path_info *pi)
{
	char *url = blobmsg_data(blob_data(cl->hdr.head));

	n_requests++;
	d->handle_request(cl, url, pi);
}

static void uh_complete_request(struct client *cl)
{
	struct deferred_request *dr;

	n_requests--;

	while (!list_empty(&pending_requests)) {
		if (n_requests >= conf.max_script_requests)
			return;

		dr = list_first_entry(&pending_requests, struct deferred_request, list);
		list_del(&dr->list);

		cl = dr->cl;
		dr->called = true;
		cl->dispatch.data_blocked = false;
		uh_invoke_script(cl, dr->d, dr->path ? &dr->pi : NULL);
		client_poll_post_data(cl);
		ustream_poll(cl->us);
	}
}


static void
uh_free_pending_request(struct client *cl)
{
	struct deferred_request *dr = cl->dispatch.req_data;

	if (dr->called)
		uh_complete_request(cl);
	else
		list_del(&dr->list);
	free(dr);
}

static int field_len(const char *ptr)
{
	if (!ptr)
		return 0;

	return strlen(ptr) + 1;
}

#define path_info_fields \
	_field(root) \
	_field(phys) \
	_field(name) \
	_field(info) \
	_field(query)

static void
uh_defer_script(struct client *cl, struct dispatch_handler *d, struct path_info *pi)
{
	struct deferred_request *dr;
	char *_root, *_phys, *_name, *_info, *_query;

	cl->dispatch.req_free = uh_free_pending_request;

	if (pi) {
		/* allocate enough memory to duplicate all path_info strings in one block */
#undef _field
#define _field(_name) &_##_name, field_len(pi->_name),
		dr = calloc_a(sizeof(*dr), path_info_fields NULL);

		memcpy(&dr->pi, pi, sizeof(*pi));
		dr->path = true;

		/* copy all path_info strings */
#undef _field
#define _field(_name) if (pi->_name) dr->pi._name = strcpy(_##_name, pi->_name);
		path_info_fields
	} else {
		dr = calloc(1, sizeof(*dr));
	}

	cl->dispatch.req_data = dr;
	cl->dispatch.data_blocked = true;
	dr->cl = cl;
	dr->d = d;
	list_add(&dr->list, &pending_requests);
}

static void
uh_invoke_handler(struct client *cl, struct dispatch_handler *d, char *url, struct path_info *pi)
{
	if (!d->script)
		return d->handle_request(cl, url, pi);

	if (n_requests >= conf.max_script_requests)
		return uh_defer_script(cl, d, pi);

	cl->dispatch.req_free = uh_complete_request;
	uh_invoke_script(cl, d, pi);
}

static bool __handle_file_request(struct client *cl, char *url)
{
	static const struct blobmsg_policy hdr_policy[__HDR_MAX] = {
		[HDR_AUTHORIZATION] = { "authorization", BLOBMSG_TYPE_STRING },
		[HDR_IF_MODIFIED_SINCE] = { "if-modified-since", BLOBMSG_TYPE_STRING },
		[HDR_IF_UNMODIFIED_SINCE] = { "if-unmodified-since", BLOBMSG_TYPE_STRING },
		[HDR_IF_MATCH] = { "if-match", BLOBMSG_TYPE_STRING },
		[HDR_IF_NONE_MATCH] = { "if-none-match", BLOBMSG_TYPE_STRING },
		[HDR_IF_RANGE] = { "if-range", BLOBMSG_TYPE_STRING },
	};
	struct dispatch_handler *d;
	struct blob_attr *tb[__HDR_MAX];
	struct path_info *pi;
	char *user, *pass, *auth;

	pi = uh_path_lookup(cl, url);
	if (!pi)
		return false;

	if (pi->redirected)
		return true;

	blobmsg_parse(hdr_policy, __HDR_MAX, tb, blob_data(cl->hdr.head), blob_len(cl->hdr.head));

	auth = tb[HDR_AUTHORIZATION] ? blobmsg_data(tb[HDR_AUTHORIZATION]) : NULL;

	if (!uh_auth_check(cl, pi->name, auth, &user, &pass))
		return true;

	if (user && pass) {
		blobmsg_add_string(&cl->hdr, "http-auth-user", user);
		blobmsg_add_string(&cl->hdr, "http-auth-pass", pass);
	}

	d = dispatch_find(url, pi);
	if (d)
		uh_invoke_handler(cl, d, url, pi);
	else
		uh_file_request(cl, url, pi, tb);

	return true;
}

static char *uh_handle_alias(char *old_url)
{
	struct alias *alias;
	static char *new_url;
	static int url_len;

	if (!list_empty(&conf.cgi_alias)) list_for_each_entry(alias, &conf.cgi_alias, list) {
		int old_len;
		int new_len;
		int path_len = 0;

		if (!uh_path_match(alias->alias, old_url))
			continue;

		if (alias->path)
			path_len = strlen(alias->path);

		old_len = strlen(old_url) + 1;
		new_len = old_len + MAX(conf.cgi_prefix_len, path_len);

		if (new_len > url_len) {
			new_url = realloc(new_url, new_len);
			url_len = new_len;
		}

		*new_url = '\0';

		if (alias->path)
			strcpy(new_url, alias->path);
		else if (conf.cgi_prefix)
			strcpy(new_url, conf.cgi_prefix);
		strcat(new_url, old_url);

		return new_url;
	}
	return old_url;
}

void uh_handle_request(struct client *cl)
{
	struct http_request *req = &cl->request;
	struct dispatch_handler *d;
	char *url = blobmsg_data(blob_data(cl->hdr.head));
	char *error_handler, *escaped_url;

	blob_buf_init(&cl->hdr_response, 0);
	url = uh_handle_alias(url);

	uh_handler_run(cl, &url, false);
	if (!url)
		return;

	req->redirect_status = 200;
	d = dispatch_find(url, NULL);
	if (d)
		return uh_invoke_handler(cl, d, url, NULL);

	if (__handle_file_request(cl, url))
		return;

	if (uh_handler_run(cl, &url, true)) {
		if (!url)
			return;

		uh_handler_run(cl, &url, false);
		if (__handle_file_request(cl, url))
			return;
	}

	req->redirect_status = 404;
	if (conf.error_handler) {
		error_handler = alloca(strlen(conf.error_handler) + 1);
		strcpy(error_handler, conf.error_handler);
		if (__handle_file_request(cl, error_handler))
			return;
	}

	escaped_url = uh_htmlescape(url);

	uh_client_error(cl, 404, "Not Found", "The requested URL %s was not found on this server.",
	                escaped_url ? escaped_url : "");

	if (escaped_url)
		free(escaped_url);
}
