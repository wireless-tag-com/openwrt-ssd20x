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
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdio.h>
#include <poll.h>

#include "uhttpd.h"
#include "plugin.h"

#define UH_LUA_CB	"handle_request"

static const struct uhttpd_ops *ops;
static struct config *_conf;
#define conf (*_conf)

static lua_State *_L;

static int uh_lua_recv(lua_State *L)
{
	static struct pollfd pfd = {
		.fd = STDIN_FILENO,
		.events = POLLIN,
	};
	luaL_Buffer B;
	int data_len = 0;
	int len;
	int r;

	len = luaL_optnumber(L, 1, LUAL_BUFFERSIZE);
	luaL_buffinit(L, &B);
	while(len > 0) {
		char *buf;

		buf = luaL_prepbuffer(&B);
		r = read(STDIN_FILENO, buf,
		         len < LUAL_BUFFERSIZE ? len : LUAL_BUFFERSIZE);
		if (r < 0) {
			if (errno == EWOULDBLOCK || errno == EAGAIN) {
				pfd.revents = 0;
				poll(&pfd, 1, 1000);
				if (pfd.revents & POLLIN)
					continue;
			}
			if (errno == EINTR)
				continue;

			if (!data_len)
				data_len = -1;
			break;
		}
		if (!r)
			break;

		luaL_addsize(&B, r);
		data_len += r;
		len -= r;
		if (r != LUAL_BUFFERSIZE)
			break;
	}

	luaL_pushresult(&B);
	lua_pushnumber(L, data_len);
	if (data_len > 0) {
		lua_pushvalue(L, -2);
		lua_remove(L, -3);
		return 2;
	} else {
		lua_remove(L, -2);
		return 1;
	}
}

static int uh_lua_send(lua_State *L)
{
	const char *buf;
	size_t len;

	buf = luaL_checklstring(L, 1, &len);
	if (len > 0)
		len = write(STDOUT_FILENO, buf, len);

	lua_pushnumber(L, len);
	return 1;
}

static int
uh_lua_strconvert(lua_State *L, int (*convert)(char *, int, const char *, int))
{
	const char *in_buf;
	static char out_buf[4096];
	size_t in_len;
	int out_len;

	in_buf = luaL_checklstring(L, 1, &in_len);
	out_len = convert(out_buf, sizeof(out_buf), in_buf, in_len);

	if (out_len < 0) {
		const char *error;

		if (out_len == -1)
			error = "buffer overflow";
		else
			error = "malformed string";

		luaL_error(L, "%s on URL conversion\n", error);
	}

	lua_pushlstring(L, out_buf, out_len);
	return 1;
}

static int uh_lua_urldecode(lua_State *L)
{
	return uh_lua_strconvert(L, ops->urldecode);
}

static int uh_lua_urlencode(lua_State *L)
{
	return uh_lua_strconvert(L, ops->urlencode);
}

static lua_State *uh_lua_state_init(struct lua_prefix *lua)
{
	const char *msg = "(unknown error)";
	const char *status;
	lua_State *L;
	int ret;

	L = luaL_newstate();
	luaL_openlibs(L);

	/* build uhttpd api table */
	lua_newtable(L);

	lua_pushcfunction(L, uh_lua_send);
	lua_setfield(L, -2, "send");

	lua_pushcfunction(L, uh_lua_send);
	lua_setfield(L, -2, "sendc");

	lua_pushcfunction(L, uh_lua_recv);
	lua_setfield(L, -2, "recv");

	lua_pushcfunction(L, uh_lua_urldecode);
	lua_setfield(L, -2, "urldecode");

	lua_pushcfunction(L, uh_lua_urlencode);
	lua_setfield(L, -2, "urlencode");

	lua_pushstring(L, conf.docroot);
	lua_setfield(L, -2, "docroot");

	lua_setglobal(L, "uhttpd");

	ret = luaL_loadfile(L, lua->handler);
	if (ret) {
		status = "loading";
		goto error;
	}

	ret = lua_pcall(L, 0, 0, 0);
	if (ret) {
		status = "initializing";
		goto error;
	}

	lua_getglobal(L, UH_LUA_CB);
	if (!lua_isfunction(L, -1)) {
		fprintf(stderr, "Error: Lua handler %s provides no "
		                UH_LUA_CB "() callback.\n", lua->handler);
		exit(1);
	}

	lua->ctx = L;

	return L;

error:
	if (!lua_isnil(L, -1))
		msg = lua_tostring(L, -1);

	fprintf(stderr, "Error %s %s Lua handler: %s\n",
	        status, lua->handler, msg);
	exit(1);
	return NULL;
}

static void lua_main(struct client *cl, struct path_info *pi, char *url)
{
	struct blob_attr *cur;
	const char *error;
	struct env_var *var;
	lua_State *L = _L;
	int path_len, prefix_len;
	char *str;
	int rem;

	lua_getglobal(L, UH_LUA_CB);

	/* new env table for this request */
	lua_newtable(L);

	prefix_len = strlen(pi->name);
	path_len = strlen(url);
	str = strchr(url, '?');
	if (str) {
		if (*(str + 1))
			pi->query = str + 1;
		path_len = str - url;
	}

	if (prefix_len > 0 && pi->name[prefix_len - 1] == '/')
		prefix_len--;

	if (path_len > prefix_len) {
		lua_pushlstring(L, url + prefix_len,
				path_len - prefix_len);
		lua_setfield(L, -2, "PATH_INFO");
	}

	for (var = ops->get_process_vars(cl, pi); var->name; var++) {
		if (!var->value)
			continue;

		lua_pushstring(L, var->value);
		lua_setfield(L, -2, var->name);
	}

	lua_pushnumber(L, 0.9 + (cl->request.version / 10.0));
	lua_setfield(L, -2, "HTTP_VERSION");

	lua_newtable(L);
	blob_for_each_attr(cur, cl->hdr.head, rem) {
		lua_pushstring(L, blobmsg_data(cur));
		lua_setfield(L, -2, blobmsg_name(cur));
	}
	lua_setfield(L, -2, "headers");

	switch(lua_pcall(L, 1, 0, 0)) {
	case LUA_ERRMEM:
	case LUA_ERRRUN:
		error = luaL_checkstring(L, -1);
		if (!error)
			error = "(unknown error)";

		printf("Status: 500 Internal Server Error\r\n\r\n"
	       "Unable to launch the requested Lua program:\n"
	       "  %s: %s\n", pi->phys, error);
	}

	exit(0);
}

static void lua_handle_request(struct client *cl, char *url, struct path_info *pi)
{
	struct lua_prefix *p;
	static struct path_info _pi;

	list_for_each_entry(p, &conf.lua_prefix, list) {
		if (!ops->path_match(p->prefix, url))
			continue;

		pi = &_pi;
		pi->name = p->prefix;
		pi->phys = p->handler;

		_L = p->ctx;

		if (!ops->create_process(cl, pi, url, lua_main)) {
			ops->client_error(cl, 500, "Internal Server Error",
			                  "Failed to create CGI process: %s",
			                  strerror(errno));
		}

		return;
	}

	ops->client_error(cl, 500, "Internal Server Error",
	                  "Failed to lookup matching handler");
}

static bool check_lua_url(const char *url)
{
	struct lua_prefix *p;

	list_for_each_entry(p, &conf.lua_prefix, list)
		if (ops->path_match(p->prefix, url))
			return true;

	return false;
}

static struct dispatch_handler lua_dispatch = {
	.script = true,
	.check_url = check_lua_url,
	.handle_request = lua_handle_request,
};

static int lua_plugin_init(const struct uhttpd_ops *o, struct config *c)
{
	struct lua_prefix *p;

	ops = o;
	_conf = c;

	list_for_each_entry(p, &conf.lua_prefix, list)
		uh_lua_state_init(p);

	ops->dispatch_add(&lua_dispatch);
	return 0;
}

struct uhttpd_plugin uhttpd_plugin = {
	.init = lua_plugin_init,
};
