/**
 * mytest.c – write your own test cases here.
 *
 * Compile & run via the test harness:
 *   make && ./tester workloads/mytest.so
 *
 * Or build a standalone binary for printf-debugging:
 *   gcc -g -o mytest_standalone mytest.c allocator.c && ./mytest_standalone
 *
 * The test harness expects a single function:
 *   int mytest(void *heap, size_t heap_size);
 * returning 0 on success, non-zero on failure.
 *
 * When compiled standalone (STANDALONE defined), main() runs it directly.
 */

#include "allocator.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Helper: check that two pointers don't overlap given their sizes.
 * (The test harness does this automatically, but it's handy here too.)
 * ------------------------------------------------------------------------- */
static int overlaps(void *a, size_t sa, void *b, size_t sb) {
    char *ca = a, *cb = b;
    return !(ca + sa <= cb || cb + sb <= ca);
}

/* -------------------------------------------------------------------------
 * mytest – called by the test harness
 * ------------------------------------------------------------------------- */
int mytest(void *heap, size_t heap_size) {
    (void)heap_size;          // suppress unused-parameter warning
    allocator_init(heap);

    /* ------------------------------------------------------------------
     * TODO: write your debugging test cases here.
     *
     * Suggested progression:
     *
     * 1. Basic malloc / free
     *    void *a = mymalloc(16);
     *    void *b = mymalloc(32);
     *    assert(a && b && !overlaps(a, 16, b, 32));
     *    myfree(a);
     *    myfree(b);
     *
     * 2. Realloc shrink (Step 2) — pointer should stay the same
     *    void *p = mymalloc(64);
     *    void *q = myrealloc(p, 32);
     *    assert(p == q);   // no move on shrink
     *
     * 3. Realloc grow at end (Step 3) — pointer should stay the same
     *    void *p = mymalloc(32);
     *    void *q = myrealloc(p, 64);
     *    assert(p == q);   // in-place grow (p is last block)
     *
     * 4. Free reclaims last block (Step 4)
     *    void *p = mymalloc(100);
     *    void *q = mymalloc(100);
     *    myfree(q);           // q is last block — heap should shrink
     *    void *r = mymalloc(100);
     *    assert(r == q);      // should reuse same address
     *
     * 5. Reuse freed block (Step 5/6)
     *    void *a = mymalloc(64);
     *    void *b = mymalloc(64);
     *    myfree(a);           // a is NOT last block — goes to free list
     *    void *c = mymalloc(32);
     *    assert(c == a);      // should reuse a's memory
     * ------------------------------------------------------------------ */

    /* Placeholder: always pass until you add real assertions */
    return 0;
}

/* -------------------------------------------------------------------------
 * Standalone entry point (for printf-debugging without the harness)
 * ------------------------------------------------------------------------- */
#ifdef STANDALONE
#include <stdlib.h>
#define HEAP_SIZE (1 << 20)  // 1 MB

int main(void) {
    void *heap = malloc(HEAP_SIZE);
    if (!heap) { perror("malloc"); return 1; }

    int result = mytest(heap, HEAP_SIZE);
    if (result == 0)
        printf("mytest PASSED\n");
    else
        printf("mytest FAILED (returned %d)\n", result);

    free(heap);
    return result;
}
#endif
