// Step 5/6: free a middle block repeatedly and re-allocate it.
// p2 sits between p1 and p3, so freeing p2 can't use step 4 (not last block).
// Without step 5: each malloc(200) goes to new memory --> 100 * 200 = 20000+ bytes.
// With step 5:    p2's slot gets reused each time --> ~600 bytes total.
#include "testharness.h"

const char *mytest(allocator *a) {
    void *p1 = a->malloc(200);
    void *p2 = a->malloc(200);
    void *p3 = a->malloc(200);  // anchor: makes p2 not the last block
    if (!p1 || !p2 || !p3) return "malloc failed";
    for (int i = 0; i < 100; i++) {
        a->free(p2);
        p2 = a->malloc(200);
        if (!p2) return "malloc failed";
    }
    a->free(p1);
    a->free(p2);
    a->free(p3);
    return 0;
}
