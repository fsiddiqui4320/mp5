// Step 6: free list must exist so finding free blocks is fast.
// Without free list (step 5 only): each malloc scans all N blocks = O(N) per alloc.
//   50000 allocs * 50000 blocks scanned = 2.5 billion ops = several seconds.
// With free list (step 6): each malloc pops from free list = O(1) per alloc = fast.
#include "testharness.h"
#include <stdlib.h>

#define N 50000

const char *mytest(allocator *a) {
    void **ptrs = malloc(N * sizeof(void *));  // system malloc for tracking array
    if (!ptrs) return "system malloc failed";

    // allocate N blocks with the custom allocator
    for (int i = 0; i < N; i++) {
        ptrs[i] = a->malloc(24);
    }

    // free every other block (they end up in the middle of the heap, not last)
    for (int i = 0; i < N - 1; i += 2) {
        a->free(ptrs[i]);
    }

    // allocate N/2 more -- this is the slow part without a free list
    for (int i = 0; i < N / 2; i++) {
        a->malloc(24);
    }

    free(ptrs);
    return 0;
}
