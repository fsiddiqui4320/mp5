// Step 4: simulate the memory pattern of merge sort.
// Each call allocates a buffer, recurses on halves, then frees the buffer.
// With step 4, the freed last-block gets reclaimed so the next recursion reuses it.
// Without step 4: peak = sum of all buffers ever allocated ~ O(n log n).
// With step 4:    peak = sum along one path root->leaf ~ O(n).
#include "testharness.h"

static allocator *g;

static void sort(int n) {
    if (n <= 1) return;
    void *buf = g->malloc((size_t)n * 4);
    sort(n / 2);
    sort(n - n / 2);
    g->free(buf);
}

const char *mytest(allocator *a) {
    g = a;
    sort(25000);
    return 0;
}
