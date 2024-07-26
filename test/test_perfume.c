#include "perfume.h"
#include <assert.h>
#include <stdio.h>

int main() {
    struct perfume_ctx *ctx = perfume_init(0);
    assert(ctx != NULL);

    int result = perfume_add(ctx, "probe1");
    assert(result == 0);

    perfume_free(ctx);
    printf("All tests passed!\n");
    return 0;
}

