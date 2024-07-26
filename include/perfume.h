#ifndef PERFUME_H
#define PERFUME_H

#ifdef __cplusplus
extern "C" {
#endif

struct perfume_ctx;

struct perfume_ctx *perfume_init(int flags);
void perfume_free(struct perfume_ctx *ctx);
int perfume_add(struct perfume_ctx *ctx, const char *name);

#ifdef __cplusplus
}
#endif

#endif // PERFUME_H

