// Step 3: growing the last block should happen in place.
// Without step 3: each realloc-grow allocates a new block --> sum 1+2+...+500 bytes.
// With step 3: same pointer extended in place --> ~500 bytes total.
#include "testharness.h"

const char *mytest(allocator *a) {
    void *p = a->malloc(1);
    if (!p) return "malloc failed";
    for (int i = 2; i <= 500; i++) {
        p = a->realloc(p, i);
        if (!p) return "realloc failed";
    }
    a->free(p);
    return 0;
}
