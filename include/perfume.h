#ifndef PERFUME_H
#define PERFUME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

struct perfume_ctx;
struct perfume_probe;

/**
 * @brief Enumeration for perfume check results.
 */
enum {
    PERFUME_CHECK_OK = 0,
    PERFUME_CHECK_TRACINGFS_SUPPORT,
    PERFUME_CHECK_USER_EVENTS_SUPPORT,
};

/**
 * @brief Checks if the tracing user event is readable and writable.
 *
 * @return int PERFUME_CHECK_OK if supported, otherwise PERFUME_CHECK_TRACINGFS_SUPPORT or PERFUME_CHECK_USER_EVENTS_SUPPORT.
 */
int perfume_check(void);

/**
 * @brief Returns a string description of the error code from perfume_check.
 *
 * @param error_code The error code returned from perfume_check.
 * @return const char* A string description of the error code.
 */
const char *perfume_check_str(int error_code);

/**
 * @brief Initializes a perfume context.
 *
 * @param flags Initialization flags.
 * @return struct perfume_ctx* Pointer to the initialized perfume context, or NULL on failure.
 */
struct perfume_ctx *perfume_init(int flags);

/**
 * @brief Frees a perfume context.
 *
 * @param ctx Pointer to the perfume context to free.
 */
void perfume_free(struct perfume_ctx *ctx);

/**
 * @brief Adds a probe to the perfume context.
 *
 * @param ctx Pointer to the perfume context.
 * @param name Name of the probe.
 * @return int 0 on success, or -EINVAL on error.
 */
int perfume_add(struct perfume_ctx *ctx, const char *name);

/**
 * @brief Registers a probe in the perfume context.
 *
 * @param ctx Pointer to the perfume context.
 * @param name Name of the probe.
 * @param ... Additional arguments for the probe.
 * @return struct perfume_probe* Pointer to the registered probe, or NULL on failure.
 */
struct perfume_probe *perfume_register(struct perfume_ctx *ctx, const char *name, ...);

/**
 * @brief Frees a perfume probe.
 *
 * @param probe Pointer to the perfume probe to free.
 */
void perfume_probe_free(struct perfume_probe *probe);

/**
 * @brief Deregisters a probe from the perfume context.
 *
 * @param ctx Pointer to the perfume context.
 * @param probe Pointer to the perfume probe to deregister.
 * @return int 0 on success, or an error code on failure.
 */
int perfume_deregister(struct perfume_ctx *ctx, struct perfume_probe *probe);

/**
 * @brief Writes data to the probe.
 *
 * @param probe Pointer to the perfume probe.
 * @param ... Additional arguments representing data to write.
 * @return int Number of bytes written on success, or an error code on failure.
 */
int perfume_write(struct perfume_probe *probe, ...);

/**
 * @brief Check if a particular probe has active listener
 *
 * @param probe Pointer to the perfume probe.
 * @return int positive number if, 0 if not, negaive for error
 */
int perfume_probe_enabled(struct perfume_probe *probe);

#ifdef __cplusplus
}
#endif

#endif // PERFUME_H

