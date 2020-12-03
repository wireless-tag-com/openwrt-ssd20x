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
#define _GNU_SOURCE
#define _XOPEN_SOURCE	700
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <getopt.h>
#include <errno.h>
#include <netdb.h>
#include <signal.h>

#include <libubox/usock.h>
#include <libubox/utils.h>

#include "uhttpd.h"
#include "tls.h"

char uh_buf[4096];

static int run_server(void)
{
	uloop_init();
	uh_setup_listeners();
	uh_plugin_post_init();
	uloop_run();

	return 0;
}

static void uh_config_parse(void)
{
	const char *path = conf.file;
	FILE *c;
	char line[512];
	char *col1;
	char *col2;
	char *eol;

	if (!path)
		path = "/etc/httpd.conf";

	c = fopen(path, "r");
	if (!c)
		return;

	memset(line, 0, sizeof(line));

	while (fgets(line, sizeof(line) - 1, c)) {
		if ((line[0] == '/') && (strchr(line, ':') != NULL)) {
			if (!(col1 = strchr(line, ':')) || (*col1++ = 0) ||
				!(col2 = strchr(col1, ':')) || (*col2++ = 0) ||
				!(eol = strchr(col2, '\n')) || (*eol++  = 0))
				continue;

			uh_auth_add(line, col1, col2);
		} else if (!strncmp(line, "I:", 2)) {
			if (!(col1 = strchr(line, ':')) || (*col1++ = 0) ||
				!(eol = strchr(col1, '\n')) || (*eol++  = 0))
				continue;

			uh_index_add(strdup(col1));
		} else if (!strncmp(line, "E404:", 5)) {
			if (!(col1 = strchr(line, ':')) || (*col1++ = 0) ||
				!(eol = strchr(col1, '\n')) || (*eol++  = 0))
				continue;

			conf.error_handler = strdup(col1);
		}
		else if ((line[0] == '*') && (strchr(line, ':') != NULL)) {
			if (!(col1 = strchr(line, '*')) || (*col1++ = 0) ||
				!(col2 = strchr(col1, ':')) || (*col2++ = 0) ||
				!(eol = strchr(col2, '\n')) || (*eol++  = 0))
				continue;

			uh_interpreter_add(col1, col2);
		}
	}

	fclose(c);
}

static int add_listener_arg(char *arg, bool tls)
{
	char *host = NULL;
	char *port = arg;
	char *s;
	int l;

	s = strrchr(arg, ':');
	if (s) {
		host = arg;
		port = s + 1;
		*s = 0;
	}

	if (host && *host == '[') {
		l = strlen(host);
		if (l >= 2) {
			host[l-1] = 0;
			host++;
		}
	}

	return uh_socket_bind(host, port, tls);
}

static int usage(const char *name)
{
	fprintf(stderr,
		"Usage: %s -p [addr:]port -h docroot\n"
		"	-f              Do not fork to background\n"
		"	-c file         Configuration file, default is '/etc/httpd.conf'\n"
		"	-p [addr:]port  Bind to specified address and port, multiple allowed\n"
#ifdef HAVE_TLS
		"	-s [addr:]port  Like -p but provide HTTPS on this port\n"
		"	-C file         ASN.1 server certificate file\n"
		"	-K file         ASN.1 server private key file\n"
		"	-q              Redirect all HTTP requests to HTTPS\n"
#endif
		"	-h directory    Specify the document root, default is '.'\n"
		"	-E string       Use given virtual URL as 404 error handler\n"
		"	-I string       Use given filename as index for directories, multiple allowed\n"
		"	-S              Do not follow symbolic links outside of the docroot\n"
		"	-D              Do not allow directory listings, send 403 instead\n"
		"	-R              Enable RFC1918 filter\n"
		"	-n count        Maximum allowed number of concurrent script requests\n"
		"	-N count        Maximum allowed number of concurrent connections\n"
#ifdef HAVE_LUA
		"	-l string       URL prefix for Lua handler, default is '/lua'\n"
		"	-L file         Lua handler script, omit to disable Lua\n"
#endif
#ifdef HAVE_UBUS
		"	-u string       URL prefix for UBUS via JSON-RPC handler\n"
		"	-U file         Override ubus socket path\n"
		"	-a              Do not authenticate JSON-RPC requests against UBUS session api\n"
		"	-X		Enable CORS HTTP headers on JSON-RPC api\n"
#endif
		"	-x string       URL prefix for CGI handler, default is '/cgi-bin'\n"
		"	-y alias[=path]	URL alias handle\n"
		"	-i .ext=path    Use interpreter at path for files with the given extension\n"
		"	-t seconds      CGI, Lua and UBUS script timeout in seconds, default is 60\n"
		"	-T seconds      Network timeout in seconds, default is 30\n"
		"	-k seconds      HTTP keepalive timeout\n"
		"	-A seconds      TCP keepalive timeout, default is unset\n"
		"	-d string       URL decode given string\n"
		"	-r string       Specify basic auth realm\n"
		"	-m string       MD5 crypt given string\n"
		"\n", name
	);
	return 1;
}

static void init_defaults_pre(void)
{
	conf.script_timeout = 60;
	conf.network_timeout = 30;
	conf.http_keepalive = 20;
	conf.max_script_requests = 3;
	conf.max_connections = 100;
	conf.realm = "Protected Area";
	conf.cgi_prefix = "/cgi-bin";
	conf.cgi_path = "/sbin:/usr/sbin:/bin:/usr/bin";
	INIT_LIST_HEAD(&conf.cgi_alias);
	INIT_LIST_HEAD(&conf.lua_prefix);
}

static void init_defaults_post(void)
{
	uh_index_add("index.html");
	uh_index_add("index.htm");
	uh_index_add("default.html");
	uh_index_add("default.htm");

	if (conf.cgi_prefix) {
		char *str = malloc(strlen(conf.docroot) + strlen(conf.cgi_prefix) + 1);

		strcpy(str, conf.docroot);
		strcat(str, conf.cgi_prefix);
		conf.cgi_docroot_path = str;
		conf.cgi_prefix_len = strlen(conf.cgi_prefix);
	};
}

static void fixup_prefix(char *str)
{
	int len;

	if (!str || !str[0])
		return;

	len = strlen(str) - 1;

	while (len >= 0 && str[len] == '/')
		len--;

	str[len + 1] = 0;
}

#ifdef HAVE_LUA
static void add_lua_prefix(const char *prefix, const char *handler) {
	struct lua_prefix *p;
	char *pprefix, *phandler;

	p = calloc_a(sizeof(*p),
	             &pprefix, strlen(prefix) + 1,
	             &phandler, strlen(handler) + 1);

	if (!p)
		return;

	p->prefix = strcpy(pprefix, prefix);
	p->handler = strcpy(phandler, handler);

	list_add_tail(&p->list, &conf.lua_prefix);
}
#endif

int main(int argc, char **argv)
{
	struct alias *alias;
	bool nofork = false;
	char *port;
	int opt, ch;
	int cur_fd;
	int bound = 0;
#ifdef HAVE_TLS
	int n_tls = 0;
	const char *tls_key = NULL, *tls_crt = NULL;
#endif
#ifdef HAVE_LUA
	const char *lua_prefix = NULL, *lua_handler = NULL;
#endif

	BUILD_BUG_ON(sizeof(uh_buf) < PATH_MAX);

	uh_dispatch_add(&cgi_dispatch);
	init_defaults_pre();
	signal(SIGPIPE, SIG_IGN);

	while ((ch = getopt(argc, argv, "A:aC:c:Dd:E:fh:H:I:i:K:k:L:l:m:N:n:p:qRr:Ss:T:t:U:u:Xx:y:")) != -1) {
		switch(ch) {
#ifdef HAVE_TLS
		case 'C':
			tls_crt = optarg;
			break;

		case 'K':
			tls_key = optarg;
			break;

		case 'q':
			conf.tls_redirect = 1;
			break;

		case 's':
			n_tls++;
			/* fall through */
#else
		case 'C':
		case 'K':
		case 'q':
		case 's':
			fprintf(stderr, "uhttpd: TLS support not compiled, "
			                "ignoring -%c\n", ch);
			break;
#endif
		case 'p':
			optarg = strdup(optarg);
			bound += add_listener_arg(optarg, (ch == 's'));
			break;

		case 'h':
			if (!realpath(optarg, uh_buf)) {
				fprintf(stderr, "Error: Invalid directory %s: %s\n",
						optarg, strerror(errno));
				exit(1);
			}
			conf.docroot = strdup(uh_buf);
			break;

		case 'H':
			if (uh_handler_add(optarg)) {
				fprintf(stderr, "Error: Failed to load handler script %s\n",
					optarg);
				exit(1);
			}
			break;

		case 'E':
			if (optarg[0] != '/') {
				fprintf(stderr, "Error: Invalid error handler: %s\n",
						optarg);
				exit(1);
			}
			conf.error_handler = optarg;
			break;

		case 'I':
			if (optarg[0] == '/') {
				fprintf(stderr, "Error: Invalid index page: %s\n",
						optarg);
				exit(1);
			}
			uh_index_add(optarg);
			break;

		case 'S':
			conf.no_symlinks = 1;
			break;

		case 'D':
			conf.no_dirlists = 1;
			break;

		case 'R':
			conf.rfc1918_filter = 1;
			break;

		case 'n':
			conf.max_script_requests = atoi(optarg);
			break;

		case 'N':
			conf.max_connections = atoi(optarg);
			break;

		case 'x':
			fixup_prefix(optarg);
			conf.cgi_prefix = optarg;
			break;

		case 'y':
			alias = calloc(1, sizeof(*alias));
			if (!alias) {
				fprintf(stderr, "Error: failed to allocate alias\n");
				exit(1);
			}
			alias->alias = strdup(optarg);
			alias->path = strchr(alias->alias, '=');
			if (alias->path)
				*alias->path++ = 0;
			list_add(&alias->list, &conf.cgi_alias);
			break;

		case 'i':
			optarg = strdup(optarg);
			port = strchr(optarg, '=');
			if (optarg[0] != '.' || !port) {
				fprintf(stderr, "Error: Invalid interpreter: %s\n",
						optarg);
				exit(1);
			}

			*port++ = 0;
			uh_interpreter_add(optarg, port);
			break;

		case 't':
			conf.script_timeout = atoi(optarg);
			break;

		case 'T':
			conf.network_timeout = atoi(optarg);
			break;

		case 'k':
			conf.http_keepalive = atoi(optarg);
			break;

		case 'A':
			conf.tcp_keepalive = atoi(optarg);
			break;

		case 'f':
			nofork = 1;
			break;

		case 'd':
			optarg = strdup(optarg);
			port = alloca(strlen(optarg) + 1);
			if (!port)
				return -1;

			/* "decode" plus to space to retain compat */
			for (opt = 0; optarg[opt]; opt++)
				if (optarg[opt] == '+')
					optarg[opt] = ' ';

			/* opt now contains strlen(optarg) -- no need to re-scan */
			if (uh_urldecode(port, opt, optarg, opt) < 0) {
				fprintf(stderr, "uhttpd: invalid encoding\n");
				return -1;
			}

			printf("%s", port);
			return 0;
			break;

		/* basic auth realm */
		case 'r':
			conf.realm = optarg;
			break;

		/* md5 crypt */
		case 'm':
			printf("%s\n", crypt(optarg, "$1$"));
			return 0;
			break;

		/* config file */
		case 'c':
			conf.file = optarg;
			break;

#ifdef HAVE_LUA
		case 'l':
		case 'L':
			if (ch == 'l') {
				if (lua_prefix)
					fprintf(stderr, "uhttpd: Ignoring previous -%c %s\n",
					        ch, lua_prefix);

				lua_prefix = optarg;
			}
			else {
				if (lua_handler)
					fprintf(stderr, "uhttpd: Ignoring previous -%c %s\n",
					        ch, lua_handler);

				lua_handler = optarg;
			}

			if (lua_prefix && lua_handler) {
				add_lua_prefix(lua_prefix, lua_handler);
				lua_prefix = NULL;
				lua_handler = NULL;
			}

			break;
#else
		case 'l':
		case 'L':
			fprintf(stderr, "uhttpd: Lua support not compiled, "
			                "ignoring -%c\n", ch);
			break;
#endif
#ifdef HAVE_UBUS
		case 'a':
			conf.ubus_noauth = 1;
			break;

		case 'u':
			conf.ubus_prefix = optarg;
			break;

		case 'U':
			conf.ubus_socket = optarg;
			break;

		case 'X':
			conf.ubus_cors = 1;
			break;
#else
		case 'a':
		case 'u':
		case 'U':
		case 'X':
			fprintf(stderr, "uhttpd: UBUS support not compiled, "
			                "ignoring -%c\n", ch);
			break;
#endif
		default:
			return usage(argv[0]);
		}
	}

	uh_config_parse();

	if (!conf.docroot) {
		if (!realpath(".", uh_buf)) {
			fprintf(stderr, "Error: Unable to determine work dir\n");
			return 1;
		}
		conf.docroot = strdup(uh_buf);
	}

	init_defaults_post();

	if (!bound) {
		fprintf(stderr, "Error: No sockets bound, unable to continue\n");
		return 1;
	}

#ifdef HAVE_TLS
	if (n_tls) {
		if (!tls_crt || !tls_key) {
			fprintf(stderr, "Please specify a certificate and "
					"a key file to enable SSL support\n");
			return 1;
		}

		if (uh_tls_init(tls_key, tls_crt))
		    return 1;
	}
#endif

#ifdef HAVE_LUA
	if (lua_handler || lua_prefix) {
		fprintf(stderr, "Need handler and prefix to enable Lua support\n");
		return 1;
	}

	if (!list_empty(&conf.lua_prefix) && uh_plugin_init("uhttpd_lua.so"))
		return 1;
#endif
#ifdef HAVE_UBUS
	if (conf.ubus_prefix && uh_plugin_init("uhttpd_ubus.so"))
		return 1;
#endif

	/* fork (if not disabled) */
	if (!nofork) {
		switch (fork()) {
		case -1:
			perror("fork()");
			exit(1);

		case 0:
			/* daemon setup */
			if (chdir("/"))
				perror("chdir()");

			cur_fd = open("/dev/null", O_WRONLY);
			if (cur_fd > 0) {
				dup2(cur_fd, 0);
				dup2(cur_fd, 1);
				dup2(cur_fd, 2);
			}

			break;

		default:
			exit(0);
		}
	}

	return run_server();
}
