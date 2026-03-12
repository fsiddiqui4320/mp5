// Step 1: tests that metadata exists.
// Allocates exactly 240000 bytes of user data (10000 * 24).
// Without metadata headers, memuse == 240000 --> test FAILS (expected for naive).
// With metadata headers, memuse > 240000 --> test PASSES (step 1 done).
#include "testharness.h"

const char *mytest(allocator *a) {
    void *ptrs[10000];
    for (int i = 0; i < 10000; i++) {
        ptrs[i] = a->malloc(24);
        if (!ptrs[i]) return "malloc returned NULL";
    }
    for (int i = 0; i < 10000; i++) {
        a->free(ptrs[i]);
    }
    return 0;
}
