#include "allocator.h"
#include <string.h>

typedef struct header {
    size_t size;
    int is_free;
    struct header *next;
} header_t;

#define HDR sizeof(header_t)

static void *base;
static size_t used;
static header_t *freelist;

void allocator_init(void *newbase) {
    base = newbase;
    used = 0;
    freelist = NULL;
}

void allocator_reset() {
    used = 0;
    freelist = NULL;
}

static header_t *get_hdr(void *ptr) {
    return (header_t *)ptr - 1;
}

static int is_last(header_t *h) {
    return (char *)h + HDR + h->size == (char *)base + used;
}

void *mymalloc(size_t size) {
    if (size == 0) return NULL;

    // check free list first
    header_t *prev = NULL;
    header_t *cur = freelist;
    while (cur) {
        if (cur->size >= size) {
            if (prev) prev->next = cur->next;
            else       freelist  = cur->next;
            cur->is_free = 0;
            cur->next = NULL;
            return (void *)(cur + 1);
        }
        prev = cur;
        cur = cur->next;
    }

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

    if (is_last(h)) {
        used = (size_t)((char *)h - (char *)base);
        return;
    }

    // not the last block, add to free list
    h->is_free = 1;
    h->next = freelist;
    freelist = h;
}

void *myrealloc(void *ptr, size_t size) {
    if (!size) { myfree(ptr); return NULL; }
    if (!ptr)  return mymalloc(size);

    header_t *h = get_hdr(ptr);

    if (size <= h->size) {
        // shrinking -- if it's the last block we can give memory back
        if (is_last(h)) {
            used -= h->size - size;
            h->size = size;
        }
        return ptr;
    }

    if (is_last(h)) {
        used    += size - h->size;
        h->size  = size;
        return ptr;
    }

    // have to move it
    void *ans = mymalloc(size);
    if (ans) {
        memcpy(ans, ptr, h->size);
        myfree(ptr);
    }
    return ans;
}
