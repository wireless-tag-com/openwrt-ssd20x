#ifndef __UAPI_PROCESS_H
#define __UAPI_PROCESS_H

#include "uhttpd.h"
#include <libubox/list.h>
#include <libubox/uloop.h>
#include <libubox/ustream.h>

struct uapi_process {
        struct list_head list;
        struct uloop_process uloop;

        struct ustream_fd log;
        bool log_overflow;
	char process_name[64];
	struct client *cl;
};

int uapi_process_data(const char *process_argv, struct client *cl);

#endif
