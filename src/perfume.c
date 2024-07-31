#include "perfume.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdarg.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/user_events.h>


#define container_of(ptr, type, member) ({                      \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);        \
    (type *)((char *)__mptr - offsetof(type, member));})


#define	ENABLE_DISABLE_BIT 31

#define SYS_TRACINGFS_PATH "/sys/kernel/tracing"
#define SYS_USER_EVENTS_DATA_NAME_PATH "/sys/kernel/tracing/user_events_data"

struct perfume_ctx {
	int flags;
	int trace_fd;
	struct perfume_probe **probes;
	int probe_count;
};


struct perfume_probe {
	char *name;
	char **args;
	int arg_count;
	char *command;
	struct perfume_ctx *ctx;
	int write_index;
	int enabled;
	int (*write)(void *ptr, size_t size, ...);
};

static int is_tracing_tracingfs_rw(void)
{
	struct stat buf;
	int ret;

	ret = stat(SYS_USER_EVENTS_DATA_NAME_PATH, &buf);
	if (ret == -1)
		return 1;
	return 0;
}

static int is_tracing_user_event_rw(void)
{
	if (access(SYS_USER_EVENTS_DATA_NAME_PATH, R_OK | W_OK) == 0)
		return 1;
	return 0;
}


#define PERFUME_CHECK_TRACINGFS_SUPPORT_STR "no " SYS_TRACINGFS_PATH " found, no kernel support"
#define PERFUME_CHECK_USER_EVENTS_SUPPORT_STR "no " SYS_USER_EVENTS_DATA_NAME_PATH " found, no kernel support"

#define PERFUME_CHECK_OK_STR "no error"

int perfume_check(void)
{
	if (!is_tracing_tracingfs_rw())
		return PERFUME_CHECK_TRACINGFS_SUPPORT;
	if (!is_tracing_user_event_rw())
		return PERFUME_CHECK_USER_EVENTS_SUPPORT;
	return PERFUME_CHECK_OK;
}

const char *perfume_check_str(int error_code)
{
	switch (error_code) {
		case PERFUME_CHECK_TRACINGFS_SUPPORT:
			return PERFUME_CHECK_TRACINGFS_SUPPORT_STR;
		case PERFUME_CHECK_USER_EVENTS_SUPPORT:
			return PERFUME_CHECK_USER_EVENTS_SUPPORT_STR;
		case PERFUME_CHECK_OK:
		default:
			return PERFUME_CHECK_OK_STR;
	}
}


struct perfume_ctx *perfume_init(int flags)
{
	struct perfume_ctx *ctx;

	if (flags != 0)
		return NULL;

	ctx = malloc(sizeof(*ctx));
	if (!ctx)
		goto err;

	ctx->trace_fd = open(SYS_USER_EVENTS_DATA_NAME_PATH, O_RDWR);
	if (ctx->trace_fd < 0)
		goto err_free;

	ctx->flags = flags;
	ctx->probes = NULL;
	ctx->probe_count = 0;
	return ctx;

err_free:
	free(ctx);

err:
	return NULL;
}


void perfume_free(struct perfume_ctx *ctx)
{
	if (!ctx)
		return;

	for (int i = 0; i < ctx->probe_count; ++i) {
		free(ctx->probes[i]);
	}
	free(ctx->probes);
	close(ctx->trace_fd);
	free(ctx);
}


int perfume_write(struct perfume_probe *probe, ...)
{
	va_list args;
	int i, ret = 0;
	struct iovec *io = NULL;
	int io_count;
	unsigned index;
	void *ptr;
	size_t size;

	if (!probe)
		return -EINVAL;

	if (!probe->enabled)
		return 0;

	io_count = 1 + probe->arg_count;

	io = malloc(io_count * sizeof(struct iovec));
	if (!io) {
		ret = -ENOMEM;
		goto out;
	}

	io[0].iov_base = &probe->write_index;
	io[0].iov_len = sizeof(probe->write_index);

	va_start(args, probe);

	for (i = 0; i < probe->arg_count; i++) {
		index = 1 + i;
		ptr = va_arg(args, void *);
		size = va_arg(args, size_t);
		io[index].iov_base = ptr;
		io[index].iov_len = size;
	}

	va_end(args);

	ret = writev(probe->ctx->trace_fd, io, io_count);
	if (ret < 0) {
		ret = -EINVAL;
		goto free_io;
	}

free_io:
	free(io);
out:
	return ret;
}


int perfume_add_probe(struct perfume_ctx *ctx, struct perfume_probe *probe)
{
	size_t new_size;
	struct perfume_probe **new_probes;

	if (!ctx || !probe) {
		return -EINVAL;
	}
	new_size = (ctx->probe_count + 1) * sizeof(struct perfume_probe *);
	new_probes = realloc(ctx->probes, new_size);
	if (!new_probes) {
		return -EINVAL;
	}
	ctx->probes = new_probes;
	ctx->probes[ctx->probe_count] = probe;
	ctx->probe_count++;
	return 0;
}


static inline struct perfume_probe *allocate_probe(const char *name, size_t command_len)
{
	struct perfume_probe *probe;

	probe = malloc(sizeof(struct perfume_probe));
	if (!probe)
		return NULL;

	probe->name = strdup(name);
	if (!probe->name)
		goto free_probe;

	probe->command = malloc(command_len);
	if (!probe->command)
		goto free_name;

	probe->enabled = 0;
	return probe;

free_name:
	free(probe->name);
free_probe:
	free(probe);
	return NULL;
}


static inline int allocate_args(struct perfume_probe *probe, int count)
{
	probe->args = malloc(count * sizeof(char *));
	if (!probe->args)
		return -ENOMEM;
	probe->arg_count = count;
	return 0;
}


static inline void free_probe(struct perfume_probe *probe)
{
	if (!probe)
		return;

	if (probe->args) {
		for (int i = 0; i < probe->arg_count; i++) {
			free(probe->args[i]);
		}
		free(probe->args);
	}
	free(probe->command);
	free(probe->name);
	free(probe);
}


static inline void build_command(struct perfume_probe *probe, const char *name, va_list args)
{
	const char *arg;
	snprintf(probe->command, strlen(name) + 1, "%s", name);

	for (int i = 0; i < probe->arg_count; i++) {
		arg = va_arg(args, const char *);
		probe->args[i] = strdup(arg);
		strcat(probe->command, " ");
		strcat(probe->command, arg);
		if (i < probe->arg_count - 1) {
			strcat(probe->command, ";");
		}
	}
}


int perfume_write_raw(void *ptr, size_t size, ...)
{
	va_list args;
	int i, ret = 0;
	struct iovec *io = NULL;
	int io_count;
	unsigned index;
	struct perfume_probe *probe;

	if (!ptr)
		return -EINVAL;

	probe = container_of(ptr, struct perfume_probe, write);

	if (!probe->enabled)
		return 0;

	io_count = 1 + probe->arg_count;

	io = malloc(io_count * sizeof(struct iovec));
	if (!io) {
		ret = -ENOMEM;
		goto out;
	}

	io[0].iov_base = &probe->write_index;
	io[0].iov_len = sizeof(probe->write_index);

	va_start(args, size);

	for (i = 0; i < probe->arg_count; i++) {
		index = 1 + i;
		void *arg_ptr = va_arg(args, void *);
		size_t arg_size = va_arg(args, size_t);
		io[index].iov_base = arg_ptr;
		io[index].iov_len = arg_size;
	}

	va_end(args);

	ret = writev(probe->ctx->trace_fd, io, io_count);
	if (ret < 0) {
		ret = -EINVAL;
		goto free_io;
	}

free_io:
	free(io);
out:
	return ret;
}

struct perfume_probe *perfume_register(struct perfume_ctx *ctx, const char *name, ...)
{
	va_list args;
	int count = 0;
	const char *arg;
	struct perfume_probe *probe;
	size_t command_len = strlen(name) + 1;
	struct user_reg reg = { 0 };
	int ret = -ENOMEM;

	va_start(args, name);
	while ((arg = va_arg(args, const char *)) != NULL) {
		command_len += strlen(arg) + 2;
		count++;
	}
	va_end(args);

	probe = allocate_probe(name, command_len);
	if (!probe)
		goto out;

	ret = allocate_args(probe, count);
	if (ret)
		goto free_probe;

	va_start(args, name);
	build_command(probe, name, args);
	va_end(args);

	reg.size = sizeof(reg);
	reg.enable_bit = ENABLE_DISABLE_BIT;
	reg.enable_size = sizeof(probe->enabled);
	reg.enable_addr = (__u64)&probe->enabled;
	reg.name_args = (__u64)probe->command;

	if (ioctl(ctx->trace_fd, DIAG_IOCSREG, &reg) == -1) {
		ret = -EINVAL;
		goto free_probe;
	}

	probe->write_index = reg.write_index;
	probe->ctx = ctx;
	probe->write = perfume_write_raw;

	return probe;

free_probe:
	free_probe(probe);
out:
	return NULL;
}


static inline int unregister_event(struct perfume_ctx *ctx, struct perfume_probe *probe)
{
	struct user_unreg unreg = { 0 };

	unreg.size = sizeof(unreg);
	unreg.disable_bit = ENABLE_DISABLE_BIT;
	unreg.disable_addr = (__u64)&probe->enabled;

	if (ioctl(ctx->trace_fd, DIAG_IOCSUNREG, &unreg) == -1) {
		return -EINVAL;
	}

	return 0;
}


int perfume_deregister(struct perfume_ctx *ctx, struct perfume_probe *probe)
{
	int i, j;
	size_t new_size;
	struct perfume_probe **new_probes;
	int ret;

	if (!ctx || !probe)
		return -EINVAL;

	ret = unregister_event(ctx, probe);
	if (ret)
		return ret;

	for (i = 0; i < ctx->probe_count; ++i) {
		if (ctx->probes[i] == probe) {
			/* shift to front, fill gap */
			for (j = i; j < ctx->probe_count - 1; ++j) {
				ctx->probes[j] = ctx->probes[j + 1];
			}
			ctx->probe_count--;
			new_size = ctx->probe_count * sizeof(struct perfume_probe *);
			new_probes = realloc(ctx->probes, new_size);
			if (ctx->probe_count > 0 && !new_probes) {
				return -ENOBUFS;
			}
			ctx->probes = new_probes;
			free_probe(probe);
			return 0;
		}
	}
	return -EINVAL;
}


void perfume_probe_free(struct perfume_probe *probe)
{
	int i;

	if (!probe)
		return;

	free(probe->name);
	for (i = 0; i < probe->arg_count; i++) {
		free(probe->args[i]);
	}
	free(probe->command);
	free(probe->args);
	free(probe);
}


int perfume_probe_enabled(struct perfume_probe *probe)
{
	if (!probe)
		return -EINVAL;

	if (!probe->enabled)
		return 0;

	return 1;
}
