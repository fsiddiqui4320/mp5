// Steps 7+8: block merging AND splitting (extra credit).
// Free two adjacent 100-byte blocks; with merging they become one ~200-byte block.
// Then malloc(150) uses it; with splitting the leftover stays as a free block.
// Without merge+split: malloc(150) can't fit in either 100-byte slot, always new memory.
//   50 iters * ~270 bytes = 13500+ bytes > 4000.
// With merge+split: same memory reused each iteration --> ~300 bytes < 4000.
#include "testharness.h"

const char *mytest(allocator *a) {
    for (int i = 0; i < 50; i++) {
        void *p1     = a->malloc(100);
        void *p2     = a->malloc(100);
        void *anchor = a->malloc(8);   // so p2 is not the last block
        if (!p1 || !p2 || !anchor) return "malloc failed";
        a->free(p1);
        a->free(p2);
        // p1 and p2 are adjacent; with merging they form a ~200-byte free block
        void *big = a->malloc(150);
        if (!big) return "malloc after merge failed";
        a->free(big);
        a->free(anchor);
    }
    return 0;
}
