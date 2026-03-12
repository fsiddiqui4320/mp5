// Step 5/6: more chaotic free/reuse pattern with multiple blocks.
// Without step 5: 500 rounds * 10 blocks * 500 bytes = 2.5 MB.
// With step 5:    ~10 blocks * 500 bytes = ~5000 bytes.
#include "testharness.h"

#define N 10

const char *mytest(allocator *a) {
    void *anchor = a->malloc(8);  // so none of ptrs[N-1] is always the last block
    if (!anchor) return "malloc failed";
    void *ptrs[N];
    for (int i = 0; i < N; i++) {
        ptrs[i] = a->malloc(500);
        if (!ptrs[i]) return "malloc failed";
    }
    for (int round = 0; round < 500; round++) {
        for (int i = 0; i < N; i++) {
            a->free(ptrs[i]);
            ptrs[i] = a->malloc(500);
            if (!ptrs[i]) return "malloc failed";
        }
    }
    for (int i = 0; i < N; i++) a->free(ptrs[i]);
    a->free(anchor);
    return 0;
}
