#ifndef PERFUME_H
#define PERFUME_H

#ifdef __cplusplus
extern "C" {
#endif

struct perfume_ctx;

enum {
	PERFUME_CHECK_OK = 0,
	PERFUME_CHECK_NO_KERNEL_SUPPORT,
};

int perfume_check(void);
const char *perfume_check_str(int);
struct perfume_ctx *perfume_init(int flags);
void perfume_free(struct perfume_ctx *ctx);
int perfume_add(struct perfume_ctx *ctx, const char *name);

#ifdef __cplusplus
}
#endif

#endif // PERFUME_H

