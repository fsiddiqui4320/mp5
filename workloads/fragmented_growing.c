// Step 9: size-aware free list (extra credit).
// Many differently-sized alloc/free cycles on a fragmented heap.
// A size-aware free list (e.g. segregated or sorted by size) finds the right
// size block without scanning all free blocks.
#include "testharness.h"

#define SLOTS 200

const char *mytest(allocator *a) {
    void *ptrs[SLOTS];

    // allocate blocks of varying sizes
    for (int i = 0; i < SLOTS; i++) {
        ptrs[i] = a->malloc((size_t)((i % 16 + 1) * 64));
    }

    // free every third block, leaving a fragmented heap
    for (int i = 0; i < SLOTS; i += 3) {
        a->free(ptrs[i]);
        ptrs[i] = NULL;
    }

    // repeatedly alloc and free at those spots
    for (int round = 0; round < 5000; round++) {
        for (int i = 0; i < SLOTS; i += 3) {
            ptrs[i] = a->malloc((size_t)((i % 16 + 1) * 64));
            a->free(ptrs[i]);
            ptrs[i] = NULL;
        }
    }

    // clean up
    for (int i = 0; i < SLOTS; i++) {
        if (ptrs[i]) a->free(ptrs[i]);
    }
    return 0;
}
