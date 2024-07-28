#include "perfume.h"
#include <assert.h>
#include <stdio.h>
#include <inttypes.h>
#include <unistd.h>

void test_probe_write2(void)
{
	uint32_t var1 = 23, var2 = 666;
	struct perfume_ctx *ctx;
	struct perfume_probe *probe;

	ctx = perfume_init(0);
	assert(ctx != NULL);

	probe = perfume_register(ctx, "test_y", "u32 var1", "u32 var2", NULL);
	assert(probe != NULL);

	perfume_write(probe, &var1, sizeof(var1), &var2, sizeof(var2));

	perfume_deregister(ctx, probe);

	perfume_probe_free(probe);
	perfume_free(ctx);
}

void test_probe_write(void)
{
	uint32_t variable = 23;
	struct perfume_ctx *ctx;
	struct perfume_probe *probe;

	ctx = perfume_init(0);
	assert(ctx != NULL);

	probe = perfume_register(ctx, "test_x", "u32 count", NULL);
	assert(probe != NULL);

	perfume_write(probe, &variable, sizeof(variable));

	perfume_deregister(ctx, probe);

	perfume_probe_free(probe);
	perfume_free(ctx);
}

void test_probe_multiple(void)
{
	struct perfume_ctx *ctx;
	struct perfume_probe *probe_c, *probe_d;

	ctx = perfume_init(0);
	assert(ctx != NULL);

	probe_c = perfume_register(ctx, "test.c", "u32 count", NULL);
	assert(probe_c != NULL);

	probe_d = perfume_register(ctx, "test.d", "u32 count", NULL);
	assert(probe_d != NULL);

	perfume_deregister(ctx, probe_c);
	perfume_deregister(ctx, probe_d);

	perfume_probe_free(probe_c);
	perfume_probe_free(probe_d);
	perfume_free(ctx);
}

void test_probe_register(void)
{
	struct perfume_ctx *ctx;
	struct perfume_probe *probe;

	ctx = perfume_init(0);
	assert(ctx != NULL);

	probe = perfume_register(ctx, "test.b", "u32 count", NULL);
	assert(probe != NULL);
	perfume_deregister(ctx, probe);
	perfume_probe_free(probe);
	perfume_free(ctx);
}

void test_startup_teardown_probe(void)
{
	struct perfume_ctx *ctx;
	struct perfume_probe *probe;

	ctx = perfume_init(0);
	assert(ctx != NULL);

	probe = perfume_register(ctx, "test.a", NULL);
	assert(probe != NULL);
	perfume_deregister(ctx, probe);
	perfume_probe_free(probe);
	perfume_free(ctx);
}

void test_startup_teardown(void)
{
	struct perfume_ctx *ctx;

	ctx = perfume_init(0);
	assert(ctx != NULL);

	perfume_free(ctx);
}

int main(void)
{
	test_startup_teardown();
	test_startup_teardown_probe();
	test_probe_register();
	test_probe_multiple();
	test_probe_write();
	test_probe_write2();

	return 0;
}

