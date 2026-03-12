// Step 2: realloc shrink should return the same pointer, not allocate new memory.
// Without step 2: each realloc to a smaller size allocates a new block --> lots of waste.
// With step 2: same pointer returned, memory stays at ~5000 bytes.
#include "testharness.h"

const char *mytest(allocator *a) {
    void *p = a->malloc(5000);
    if (!p) return "malloc failed";
    for (int i = 0; i < 1000; i++) {
        p = a->realloc(p, 5000 - i * 4);
        if (!p) return "realloc failed";
    }
    a->free(p);
    return 0;
}
