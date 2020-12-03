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

#include <ctype.h>
#include "uhttpd.h"

bool uh_use_chunked(struct client *cl)
{
	if (cl->request.version != UH_HTTP_VER_1_1)
		return false;

	if (cl->request.method == UH_HTTP_MSG_HEAD || cl->request.method == UH_HTTP_MSG_OPTIONS)
		return false;

	/* RFC2616 10.2.5, 10.3.5 */
	if (cl->http_code == 204 || cl->http_code == 304)
		return false;

	return !cl->request.disable_chunked;
}

void uh_chunk_write(struct client *cl, const void *data, int len)
{
	bool chunked = uh_use_chunked(cl);

	if (cl->state == CLIENT_STATE_CLEANUP)
		return;

	uloop_timeout_set(&cl->timeout, conf.network_timeout * 1000);
	if (chunked)
		ustream_printf(cl->us, "%X\r\n", len);
	ustream_write(cl->us, data, len, true);
	if (chunked)
		ustream_printf(cl->us, "\r\n");
}

void uh_chunk_vprintf(struct client *cl, const char *format, va_list arg)
{
	char buf[256];
	va_list arg2;
	int len;

	if (cl->state == CLIENT_STATE_CLEANUP)
		return;

	uloop_timeout_set(&cl->timeout, conf.network_timeout * 1000);
	if (!uh_use_chunked(cl)) {
		ustream_vprintf(cl->us, format, arg);
		return;
	}

	va_copy(arg2, arg);
	len = vsnprintf(buf, sizeof(buf), format, arg2);
	va_end(arg2);

	ustream_printf(cl->us, "%X\r\n", len);
	if (len < sizeof(buf))
		ustream_write(cl->us, buf, len, true);
	else
		ustream_vprintf(cl->us, format, arg);
	ustream_printf(cl->us, "\r\n");
}

void uh_chunk_printf(struct client *cl, const char *format, ...)
{
	va_list arg;

	va_start(arg, format);
	uh_chunk_vprintf(cl, format, arg);
	va_end(arg);
}

void uh_chunk_eof(struct client *cl)
{
	if (!uh_use_chunked(cl))
		return;

	if (cl->state == CLIENT_STATE_CLEANUP)
		return;

	ustream_printf(cl->us, "0\r\n\r\n");
}

/* blen is the size of buf; slen is the length of src.  The input-string need
** not be, and the output string will not be, null-terminated.  Returns the
** length of the decoded string, -1 on buffer overflow, -2 on malformed string. */
int uh_urldecode(char *buf, int blen, const char *src, int slen)
{
	int i;
	int len = 0;

#define hex(x) \
	(((x) <= '9') ? ((x) - '0') : \
		(((x) <= 'F') ? ((x) - 'A' + 10) : \
			((x) - 'a' + 10)))

	for (i = 0; (i < slen) && (len < blen); i++)
	{
		if (src[i] != '%') {
			buf[len++] = src[i];
			continue;
		}

		if (i + 2 >= slen || !isxdigit(src[i + 1]) || !isxdigit(src[i + 2]))
			return -2;

		buf[len++] = (char)(16 * hex(src[i+1]) + hex(src[i+2]));
		i += 2;
	}
	buf[len] = 0;

	return (i == slen) ? len : -1;
}

/* blen is the size of buf; slen is the length of src.  The input-string need
** not be, and the output string will not be, null-terminated.  Returns the
** length of the encoded string, or -1 on error (buffer overflow) */
int uh_urlencode(char *buf, int blen, const char *src, int slen)
{
	int i;
	int len = 0;
	static const char hex[] = "0123456789abcdef";

	for (i = 0; (i < slen) && (len < blen); i++)
	{
		if( isalnum(src[i]) || (src[i] == '-') || (src[i] == '_') ||
		    (src[i] == '.') || (src[i] == '~') )
		{
			buf[len++] = src[i];
		}
		else if ((len+3) <= blen)
		{
			buf[len++] = '%';
			buf[len++] = hex[(src[i] >> 4) & 15];
			buf[len++] = hex[ src[i]       & 15];
		}
		else
		{
			len = -1;
			break;
		}
	}

	return (i == slen) ? len : -1;
}

int uh_b64decode(char *buf, int blen, const void *src, int slen)
{
	const unsigned char *str = src;
	unsigned int cout = 0;
	unsigned int cin  = 0;
	int len = 0;
	int i = 0;

	for (i = 0; (i <= slen) && (str[i] != 0); i++)
	{
		cin = str[i];

		if ((cin >= '0') && (cin <= '9'))
			cin = cin - '0' + 52;
		else if ((cin >= 'A') && (cin <= 'Z'))
			cin = cin - 'A';
		else if ((cin >= 'a') && (cin <= 'z'))
			cin = cin - 'a' + 26;
		else if (cin == '+')
			cin = 62;
		else if (cin == '/')
			cin = 63;
		else if (cin == '=')
			cin = 0;
		else
			continue;

		cout = (cout << 6) | cin;

		if ((i % 4) != 3)
			continue;

		if ((len + 3) >= blen)
			break;

		buf[len++] = (char)(cout >> 16);
		buf[len++] = (char)(cout >> 8);
		buf[len++] = (char)(cout);
	}

	buf[len++] = 0;
	return len;
}

bool uh_path_match(const char *prefix, const char *url)
{
	int len = strlen(prefix);

	/* A prefix of "/" will - by definition - match any url */
	if (prefix[0] == '/' && len == 1)
		return true;

	if (strncmp(url, prefix, len) != 0)
		return false;

	return url[len] == '/' || url[len] == 0;
}

char *uh_split_header(char *str)
{
	char *val;

	val = strchr(str, ':');
	if (!val)
		return NULL;

	*val = 0;
	val++;

	while (isspace(*val))
		val++;

	return val;
}

bool uh_addr_rfc1918(struct uh_addr *addr)
{
	uint32_t a;

	if (addr->family != AF_INET)
		return false;

	a = htonl(addr->in.s_addr);
	return ((a >= 0x0A000000) && (a <= 0x0AFFFFFF)) ||
	       ((a >= 0xAC100000) && (a <= 0xAC1FFFFF)) ||
	       ((a >= 0xC0A80000) && (a <= 0xC0A8FFFF));

	return 0;
}


static bool is_html_special_char(char c)
{
	switch (c)
	{
	case 0x22:
	case 0x26:
	case 0x27:
	case 0x3C:
	case 0x3E:
		return true;

	default:
		return false;
	}
}

char *uh_htmlescape(const char *str)
{
	size_t i, len;
	char *p, *copy;

	for (i = 0, len = 1; str[i]; i++)
		if (is_html_special_char(str[i]))
			len += 6; /* &#x??; */
		else
			len++;

	copy = calloc(1, len);

	if (!copy)
		return NULL;

	for (i = 0, p = copy; str[i]; i++)
		if (is_html_special_char(str[i]))
			p += sprintf(p, "&#x%02x;", (unsigned int)str[i]);
		else
			*p++ = str[i];

	return copy;
}
