#include "allocator.h"
#include <string.h>

// metadata stored before every block
typedef struct header {
    size_t size;          // how many bytes the user asked for
    int is_free;          // 1 = free, 0 = in use
    struct header *next;  // next block in the free list (only valid when is_free=1)
} header_t;

#define HDR sizeof(header_t)

static void *base;
static size_t used;
static header_t *freelist;  // head of free list (most recently freed = first)

void allocator_init(void *newbase) {
    base = newbase;
    used = 0;
    freelist = NULL;
}

void allocator_reset() {
    used = 0;
    freelist = NULL;
}

// get the header for a user pointer
static header_t *get_hdr(void *ptr) {
    return (header_t *)ptr - 1;
}

// is this block the last one on the heap?
static int is_last(header_t *h) {
    return (char *)h + HDR + h->size == (char *)base + used;
}

void *mymalloc(size_t size) {
    if (size == 0) return NULL;

    // step 6: search free list for a big enough block
    header_t *prev = NULL;
    header_t *cur = freelist;
    while (cur) {
        if (cur->size >= size) {
            // remove from free list
            if (prev) prev->next = cur->next;
            else       freelist  = cur->next;
            cur->is_free = 0;
            cur->next = NULL;
            return (void *)(cur + 1);
        }
        prev = cur;
        cur = cur->next;
    }

    // no free block found -- carve new space off the heap
    header_t *h = (header_t *)((char *)base + used);
    h->size    = size;
    h->is_free = 0;
    h->next    = NULL;
    used += HDR + size;
    return (void *)(h + 1);
}

void myfree(void *ptr) {
    if (!ptr) return;
    header_t *h = get_hdr(ptr);

    // step 4: if this is the last block just reclaim it
    if (is_last(h)) {
        used = (size_t)((char *)h - (char *)base);
        return;
    }

    // step 5/6: mark free and push onto free list
    h->is_free = 1;
    h->next = freelist;
    freelist = h;
}

void *myrealloc(void *ptr, size_t size) {
    if (!size) { myfree(ptr); return NULL; }
    if (!ptr)  return mymalloc(size);

    header_t *h = get_hdr(ptr);

    // step 2: shrinking (or same size) -- just return same pointer
    if (size <= h->size) return ptr;

    // step 3: growing the last block -- extend in place, no copy needed
    if (is_last(h)) {
        used    += size - h->size;
        h->size  = size;
        return ptr;
    }

    // otherwise: malloc new block, copy old data, free old block
    void *ans = mymalloc(size);
    if (ans) {
        memcpy(ans, ptr, h->size);
        myfree(ptr);
    }
    return ans;
}
