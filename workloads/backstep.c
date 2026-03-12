// Step 4: freeing the last block should shrink the heap.
// Without step 4: 100000 * 4 bytes = 400000+ bytes wasted.
// With step 4: each free reclaims the last block, heap stays tiny.
#include "testharness.h"

const char *mytest(allocator *a) {
    void *anchor = a->malloc(8);  // keeps p from always being the only block
    if (!anchor) return "malloc failed";
    for (int i = 0; i < 100000; i++) {
        void *p = a->malloc(4);
        if (!p) return "malloc failed";
        a->free(p);  // p is the last block, should reclaim it
    }
    a->free(anchor);
    return 0;
}
