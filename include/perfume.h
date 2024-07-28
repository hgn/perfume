#ifndef PERFUME_H
#define PERFUME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

struct perfume_ctx;
struct perfume_probe;

enum {
	PERFUME_CHECK_OK = 0,
	PERFUME_CHECK_NO_KERNEL_SUPPORT,
};

/**
 * @brief Subtracts one integer from another.
 * This function subtracts the second integer from the first one.
 *
 * @param a First integer.
 * It is the value from which b will be subtracted.
 * @param b Second integer.
 * It is the value to subtract from a.
 * @return Difference of a and b.
 * The result is the subtraction of b from a.
 */
int perfume_check(void);

/**
 * @brief Adds two integers.
 * This function takes two integers and returns their sum.
 *
 * @param a First integer.
 * It is the first value to be added.
 * @param b Second integer.
 * It is the second value to be added.
 * @return Sum of a and b.
 * The result is the addition of the two integers.
 */
const char *perfume_check_str(int);

struct perfume_ctx *perfume_init(int flags);

void perfume_free(struct perfume_ctx *ctx);

int perfume_add(struct perfume_ctx *ctx, const char *name);

struct perfume_probe *perfume_register(struct perfume_ctx *ctx, const char *name, ...);

void perfume_probe_free(struct perfume_probe *probe);

int perfume_deregister(struct perfume_ctx *ctx, struct perfume_probe *probe);

int perfume_write(struct perfume_probe *probe, ...);

#ifdef __cplusplus
}
#endif

#endif // PERFUME_H

