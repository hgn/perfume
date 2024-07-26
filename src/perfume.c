#include "perfume.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <linux/user_events.h>

static const char sys_user_events[] = "/sys/kernel/tracing/user_events_data";
int enabled = 0;

static int is_tracing_user_event_rw(void)
{
	if (access(sys_user_events, R_OK | W_OK) == 0)
		return 1;
	return 0;
}

int perfume_check(void)
{
	if (!is_tracing_user_event_rw())
		return PERFUME_CHECK_NO_KERNEL_SUPPORT;
	return PERFUME_CHECK_OK;
}

const char *perfume_check_str(int error_code) {
	switch (error_code) {
		case PERFUME_CHECK_NO_KERNEL_SUPPORT:
			return "no /sys/kernel/tracing/user_events_data found, no kernel support";
			break;
	}

	return "no error";
}

static int event_reg(int fd, const char *command, int *write, int *enabled)
{
	struct user_reg reg = {0};

	reg.size = sizeof(reg);
	reg.enable_bit = 31;
	reg.enable_size = sizeof(*enabled);
	reg.enable_addr = (__u64)enabled;
	reg.name_args = (__u64)command;

	if (ioctl(fd, DIAG_IOCSREG, &reg) == -1)
		return -1;

	*write = reg.write_index;

	return 0;
}


struct perfume_ctx {
    int flags;
    int trace_fd;
};

struct perfume_ctx *perfume_init(int flags) {
	struct perfume_ctx *ctx;

	ctx = malloc(sizeof(*ctx));
	if (!ctx)
		goto err;

	ctx->trace_fd = open(sys_user_events, O_RDWR);
	if (ctx->trace_fd < 0)
		goto err_free;

	ctx->flags = flags;
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

	close(ctx->trace_fd);

	free(ctx);
}

int perfume_add(struct perfume_ctx *ctx, const char *name)
{
	int write;
	struct iovec io[2];
	uint32_t count = 0;

	if (!ctx || !name)
		return -1;

	if (event_reg(ctx->trace_fd, "test u32 count", &write, &enabled) == -1)
		return errno;

	/* Setup iovec */
	io[0].iov_base = &write;
	io[0].iov_len = sizeof(write);
	io[1].iov_base = &count;
	io[1].iov_len = sizeof(count);

	printf("Press enter to check status...\n");
	getchar();

	/* Check if anyone is listening */
	if (enabled) {
		ssize_t ret;
		/* Yep, trace out our data */
		ret = writev(ctx->trace_fd, (const struct iovec *)io, 2);
		if (ret < 0) {
			return -1;
		}

		/* Increase the count */
		count++;

		printf("Something was attached, wrote data\n");
	}

	return 0;
}

