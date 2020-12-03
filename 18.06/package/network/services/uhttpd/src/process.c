#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <limits.h>
#include <fcntl.h>
#include <libubus.h>
#include <libubox/list.h>

#include "process.h"
#include "uhttpd.h"

#define UAPI_PATH	"/usr/sbin/"
#define UAPI_PROCESS_NAME	"wapi"
#define UAPI_WEB_ARGV	"web"

extern void uh_ubus_send_header(struct client *cl);
extern void uh_ubus_send_response(struct client *cl, char* buf);

static struct list_head uapi_process_list = LIST_HEAD_INIT(uapi_process_list);

static void uapi_fd_set_cloexec(int fd)
{
	fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
}

static void uapi_delete_process(struct uapi_process *proc)
{
	list_del(&proc->list);
	ustream_free(&proc->log.stream);
	close(proc->log.fd.fd);
	free(proc);
}

static void uapi_process_log_read_cb(struct ustream *s, int bytes)
{
	struct uapi_process *proc;
	char *data;
	int len = 0;

	proc = container_of(s, struct uapi_process, log.stream);

	do {
		char *newline;

		data = ustream_get_read_buf(s, &len);
		if (!len)
			break;

		newline = strchr(data, '\n');

		if (proc->log_overflow) {
			if (newline) {
				len = newline + 1 - data;
				proc->log_overflow = false;
			}
		} else if (newline) {
			*newline = 0;
			len = newline + 1 - data;
			uh_ubus_send_header(proc->cl);
			uh_ubus_send_response(proc->cl, data);
		} else if (len == s->r.buffer_len) {
			proc->log_overflow = true;
		} else
			break;

		ustream_consume(s, len);
	} while (1);
}

static void uapi_process_cb(struct uloop_process *proc, int ret)
{
	struct uapi_process *lp;
	lp = container_of(proc, struct uapi_process, uloop);

	if(WIFEXITED(ret)) {
		printf("%s (%d): exit with %d\n",
			lp->process_name, lp->uloop.pid, WEXITSTATUS(ret));
	} else if (WIFSIGNALED(ret)) {
		printf("%s (%d): exit with signal %d\n",
			lp->process_name, lp->uloop.pid, WTERMSIG(ret));
	} else if (WIFSTOPPED(ret)) {
		printf("%s (%d): stop with signal %d\n",
			lp->process_name, lp->uloop.pid, WSTOPSIG(ret));
	}

	while (ustream_poll(&lp->log.stream));
	uapi_delete_process(lp);
	uh_client_unref(lp->cl);
}

int uapi_process_data(const char *process_argv, struct client *cl)
{
	int pfds[2];
	pid_t pid;
	struct uapi_process *l_proc = NULL;

	if (pipe(pfds) < 0) {
		fprintf(stderr, "pipe failed for (%d)", errno);
		return -1;
	}

	/*
	 * free after process stop
	 */
	l_proc = malloc(sizeof(struct uapi_process));

	memset(l_proc, 0, sizeof(struct uapi_process));

	if (!l_proc) {
		fprintf(stderr, "malloc uapi_process failed");
		goto error;
	}

	if ((pid = fork()) < 0) {
		fprintf(stderr, "fork uapi_process failed for");
		goto error;
	}

	if (!pid) {
		/* child */
		int i;
		char app_path[PATH_MAX];

		/*
		 * disable pipe read
		 */
		close(pfds[0]);

		/*
		 * stdin/sdtout/stderr to pipe write
		 */
		for (i = 0; i <= 2; i++) {
			if (pfds[1] == i)
			continue;

			dup2(pfds[1], i);
		}

		if (pfds[1] > 2)
			close(pfds[1]);

		memset(app_path, 0, sizeof(app_path));
		snprintf(app_path, sizeof(app_path), "%s%s", UAPI_PATH, UAPI_PROCESS_NAME);
		execl(app_path, UAPI_PROCESS_NAME, UAPI_WEB_ARGV, process_argv, (char *)NULL);
		fprintf(stderr, "failed execl %s", app_path);
		exit(127);
	}

	/*
	 * parent disable pipe write
	 */
	close(pfds[1]);
	strncpy(l_proc->process_name, UAPI_PROCESS_NAME, sizeof(l_proc->process_name) - 1);
	l_proc->uloop.cb = uapi_process_cb;
	l_proc->uloop.pid = pid;
	l_proc->cl = cl;
	uloop_process_add(&l_proc->uloop);
	list_add_tail(&l_proc->list, &uapi_process_list);

	uapi_fd_set_cloexec(pfds[0]);
	l_proc->log.stream.string_data = true;
	l_proc->log.stream.notify_read = uapi_process_log_read_cb;
	l_proc->log.stream.r.buffer_len = 16192;
	ustream_fd_init(&l_proc->log, pfds[0]);

	return 0;

error:
	if (l_proc) {
		free(l_proc);
	}

	close(pfds[0]);
	close(pfds[1]);
	return -1;
}
