#include "perfume.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>

int main() {
	struct perfume_ctx *ctx;
	struct perfume_probe *probe;
	uint32_t data = 23;

	// Initialize the perfume context
	ctx = perfume_init(0);
	if (!ctx) {
		fprintf(stderr, "Failed to initialize perfume context\n");
		return -1;
	}

	// Register a new probe
	probe = perfume_register(ctx, "hello_world_event", "u32 hellostr", NULL);
	if (!probe) {
		fprintf(stderr, "Failed to register probe\n");
		perfume_free(ctx);
		return -1;
	}

	fprintf(stderr, "Now open user event consumer and attack to \"hello_world_event\"\n");
	fprintf(stderr, "\tE.g. \"sudo perf record -a -e \"user_events:hello_world_event\" -- sleep 5\"\n");

	sleep(5);

	if (perfume_write(probe, &data, sizeof(data)) < 0) {
		fprintf(stderr, "Failed to write to probe\n");
		perfume_deregister(ctx, probe);
		perfume_free(ctx);
		return -1;
	}

	printf("Successfully wrote to probe\n");

	// Cleanup
	perfume_deregister(ctx, probe);
	perfume_free(ctx);

	return 0;
}
