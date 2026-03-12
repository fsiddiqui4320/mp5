#include "allocator.h"
#include <string.h>  // memcpy

/* =============================================================================
 * METADATA LAYOUT (Step 1)
 *
 * Every allocation is prefixed by a `block_t` header stored just before
 * the pointer returned to the caller:
 *
 *   [ block_t header | ... user data ... ]
 *   ^                ^
 *   internal ptr     returned ptr (header + 1)
 *
 * To get the header from a user pointer:  (block_t *)ptr - 1
 * To get the user pointer from a header:  (void *)(header + 1)
 * =============================================================================
 *
 * STEP-BY-STEP IMPLEMENTATION GUIDE
 * -----------------------------------
 * Step 1 – Metadata:         Store block size in header; enable pointer math.
 * Step 2 – Shrink efficiently: realloc shrinking → do nothing (return same ptr).
 * Step 3 – Grow at end of heap: realloc growing last block → extend in-place.
 * Step 4 – Shrink at end of heap: free/shrink last block → reclaim memory.
 * Step 5 – Reuse free memory: mark freed blocks; walk all blocks on malloc.
 * Step 6 – Free list: maintain a linked list of free blocks; O(n_free) search.
 * Step 7 – Block merging [EC]: merge adjacent free blocks on free.
 * Step 8 – Block splitting [EC]: split oversized free blocks on malloc.
 * =============================================================================
 */

/* ---------------------------------------------------------------------------
 * Metadata struct (grows as you add steps)
 * --------------------------------------------------------------------------- */
typedef struct block {
    size_t size;        // usable bytes in this block (NOT including this header)
    int    is_free;     // 1 if this block is available for reuse, 0 if in use

    /* Step 6: free-list links (stored inside unused blocks) */
    struct block *free_next;  // next block in free list
    struct block *free_prev;  // prev block in free list
} block_t;

#define HEADER_SIZE (sizeof(block_t))

/* ---------------------------------------------------------------------------
 * Global state
 * --------------------------------------------------------------------------- */
static void    *base;       // smallest usable address (start of heap pool)
static size_t   used;       // bytes used so far (including all headers)

/* Step 6: head of the free list (most-recently-freed block first) */
static block_t *free_list_head;

/* ---------------------------------------------------------------------------
 * Internal helpers
 * --------------------------------------------------------------------------- */

/** Return the header for a user pointer. */
static inline block_t *get_header(void *ptr) {
    return (block_t *)ptr - 1;
}

/** Return the user pointer for a header. */
static inline void *get_user_ptr(block_t *hdr) {
    return (void *)(hdr + 1);
}

/** Return the header of the block that starts immediately after hdr. */
static inline block_t *next_block(block_t *hdr) {
    return (block_t *)((char *)get_user_ptr(hdr) + hdr->size);
}

/** Return the byte offset (from base) of the byte just past hdr's user data. */
static inline size_t block_end_offset(block_t *hdr) {
    return (size_t)((char *)get_user_ptr(hdr) + hdr->size - (char *)base);
}

/** True if hdr is the last (highest-address) block in the heap. */
static inline int is_last_block(block_t *hdr) {
    return block_end_offset(hdr) == used;
}

/* ---------------------------------------------------------------------------
 * Free-list helpers (Step 6)
 * --------------------------------------------------------------------------- */

/** Prepend hdr to the free list (LIFO — most-recently-freed first). */
static void free_list_push(block_t *hdr) {
    /* TODO (Step 6): insert hdr at the front of free_list_head */
    hdr->free_next = free_list_head;
    hdr->free_prev = NULL;
    if (free_list_head) free_list_head->free_prev = hdr;
    free_list_head = hdr;
}

/** Remove hdr from the free list (it's about to be allocated or merged). */
static void free_list_remove(block_t *hdr) {
    /* TODO (Step 6): unlink hdr from wherever it sits in free_list_head */
    if (hdr->free_prev) hdr->free_prev->free_next = hdr->free_next;
    else                free_list_head            = hdr->free_next;
    if (hdr->free_next) hdr->free_next->free_prev = hdr->free_prev;
    hdr->free_next = hdr->free_prev = NULL;
}

/** Search the free list for the first block with size >= requested_size.
 *  Returns NULL if none found.                                             */
static block_t *free_list_find(size_t requested_size) {
    /* TODO (Step 6): walk free_list_head looking for hdr->size >= requested_size */
    for (block_t *cur = free_list_head; cur; cur = cur->free_next) {
        if (cur->size >= requested_size) return cur;
    }
    return NULL;
}

/* ---------------------------------------------------------------------------
 * Public API
 * --------------------------------------------------------------------------- */

void allocator_init(void *newbase) {
    base           = newbase;
    used           = 0;
    free_list_head = NULL;
}

void allocator_reset(void) {
    used           = 0;
    free_list_head = NULL;
}

/* ---------------------------------------------------------------------------
 * mymalloc
 * --------------------------------------------------------------------------- */
void *mymalloc(size_t size) {
    if (size == 0) return NULL;

    /* -----------------------------------------------------------------------
     * Step 6: search the free list before going to the heap
     * (Replace the naive always-allocate-new approach once free list exists)
     * ----------------------------------------------------------------------- */

    /* TODO (Step 5 / 6): look for a free block large enough */
    block_t *found = free_list_find(size);  // returns NULL until Step 6
    if (found) {
        /* TODO (Step 8 – Block Splitting [EC]):
         *   If found->size is significantly larger than `size`, split it:
         *     1. Shrink found->size to `size`.
         *     2. Write a new free block_t header immediately after found's
         *        user data with the leftover bytes.
         *     3. Add the new leftover block to the free list.
         *   Only split if there is enough room for HEADER_SIZE + at least
         *   1 byte of user data in the remainder.
         */

        /* Mark the recycled block as in-use and remove from free list */
        free_list_remove(found);
        found->is_free = 0;
        return get_user_ptr(found);
    }

    /* No suitable free block — carve a new one off the top of the heap */
    block_t *hdr = (block_t *)((char *)base + used);
    hdr->size      = size;
    hdr->is_free   = 0;
    hdr->free_next = NULL;
    hdr->free_prev = NULL;

    used += HEADER_SIZE + size;
    return get_user_ptr(hdr);
}

/* ---------------------------------------------------------------------------
 * myfree
 * --------------------------------------------------------------------------- */
void myfree(void *ptr) {
    if (!ptr) return;

    block_t *hdr = get_header(ptr);

    /* -----------------------------------------------------------------------
     * Step 4: if this is the last block, just reclaim it entirely
     * ----------------------------------------------------------------------- */
    if (is_last_block(hdr)) {
        /* TODO (Step 4): set used = offset of this block's header */
        used = (size_t)((char *)hdr - (char *)base);
        return;
    }

    /* -----------------------------------------------------------------------
     * Step 5 / 6: mark block free and add to free list
     * ----------------------------------------------------------------------- */
    hdr->is_free = 1;

    /* TODO (Step 7 – Block Merging [EC]):
     *   Before pushing to the free list, check whether the block immediately
     *   after this one in memory (next_block(hdr)) is also free.  If so,
     *   absorb its size (plus HEADER_SIZE) into hdr->size, remove it from
     *   the free list, then continue.
     *   Similarly, walk backwards (need a prev pointer or footer tag) to
     *   check if the block before this one is also free and merge that way.
     *   (Backward merging is harder without a footer; consider adding one.)
     *   After merging, re-check is_last_block in case the merged block is
     *   now at the end of the heap.
     */

    free_list_push(hdr);
}

/* ---------------------------------------------------------------------------
 * myrealloc
 * --------------------------------------------------------------------------- */
void *myrealloc(void *ptr, size_t size) {
    /* Edge cases matching standard realloc semantics */
    if (!size) { myfree(ptr); return NULL; }
    if (!ptr)  { return mymalloc(size); }

    block_t *hdr    = get_header(ptr);
    size_t   oldsize = hdr->size;

    /* -----------------------------------------------------------------------
     * Step 2: shrinking (or same size) — do nothing
     * ----------------------------------------------------------------------- */
    if (size <= oldsize) {
        /* TODO (Step 8 [EC] / optional): could split the block here to
         * return unused suffix to the free pool.  For now, just return ptr. */
        return ptr;
    }

    /* -----------------------------------------------------------------------
     * Step 3: growing the last block in-place (no copy needed)
     * ----------------------------------------------------------------------- */
    if (is_last_block(hdr)) {
        /* Extend the heap to cover the larger size */
        used         += size - oldsize;   // only the delta matters
        hdr->size     = size;
        return ptr;
    }

    /* -----------------------------------------------------------------------
     * General case: allocate new block, copy data, free old block
     * ----------------------------------------------------------------------- */
    void *ans = mymalloc(size);
    if (ans) {
        memcpy(ans, ptr, oldsize);  // copy min(old, new) bytes
        myfree(ptr);
    }
    return ans;
}
