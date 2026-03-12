#include "allocator.h"
#include <string.h>

typedef struct chunk {
    size_t sz;
    int used;
    struct chunk *next;     // free list
    struct chunk *mp;       // previous chunk in memory (needed for merging)
} chunk_t;

static void *base;
static size_t top;
static chunk_t *recycle;
static chunk_t *tail;  // last carved chunk, so new chunks can set mp

void allocator_init(void *newbase) {
    base = newbase;
    top = 0;
    recycle = NULL;
    tail = NULL;
}

void allocator_reset() {
    top = 0;
    recycle = NULL;
    tail = NULL;
}

// pull a specific chunk out of the recycle list
static void recycle_remove(chunk_t *target) {
    chunk_t **p = &recycle;
    while (*p != NULL && *p != target)
        p = &(*p)->next;
    if (*p != NULL) {
        *p = target->next;
        target->next = NULL;
    }
}

// chunk that comes right after c in memory, or NULL if c is last
static chunk_t *mem_next(chunk_t *c) {
    chunk_t *n = (chunk_t *)((char *)(c + 1) + c->sz);
    if ((char *)n >= (char *)base + top) return NULL;
    return n;
}

void *mymalloc(size_t size) {
    if (size == 0) return NULL;

    // look through recycled chunks first
    chunk_t *c = recycle;
    chunk_t *prev = NULL;
    while (c != NULL) {
        if (c->sz >= size) {
            if (prev != NULL) prev->next = c->next;
            else recycle = c->next;
            c->next = NULL;

            // split if the leftover is big enough to be its own chunk
            if (c->sz >= size + sizeof(chunk_t) + 1) {
                chunk_t *right = (chunk_t *)((char *)(c + 1) + size);
                right->sz = c->sz - size - sizeof(chunk_t);
                right->used = 0;
                right->next = NULL;
                right->mp = c;
                c->sz = size;
                chunk_t *after = mem_next(right);
                if (after != NULL) after->mp = right;
                if (tail == c) tail = right;
                right->next = recycle;
                recycle = right;
            }

            c->used = 1;
            return (void *)(c + 1);
        }
        prev = c;
        c = c->next;
    }

    chunk_t *newchunk = (chunk_t *)((char *)base + top);
    newchunk->sz = size;
    newchunk->used = 1;
    newchunk->next = NULL;
    newchunk->mp = tail;
    top += sizeof(chunk_t) + size;
    tail = newchunk;
    return (void *)(newchunk + 1);
}

void myfree(void *ptr) {
    if (ptr == NULL) return;

    chunk_t *c = (chunk_t *)ptr - 1;
    c->used = 0;

    // merge with next chunk in memory if its free
    chunk_t *nx = mem_next(c);
    if (nx != NULL && !nx->used) {
        recycle_remove(nx);
        c->sz += sizeof(chunk_t) + nx->sz;
        chunk_t *after = mem_next(c);
        if (after != NULL) after->mp = c;
        if (tail == nx) tail = c;
    }

    // merge with prev chunk in memory if its free
    if (c->mp != NULL && !c->mp->used) {
        chunk_t *pv = c->mp;
        recycle_remove(pv);
        pv->sz += sizeof(chunk_t) + c->sz;
        chunk_t *after = mem_next(pv);
        if (after != NULL) after->mp = pv;
        if (tail == c) tail = pv;
        c = pv;
    }

    // if merged chunk is at the end just reclaim it
    if ((char *)(c + 1) + c->sz == (char *)base + top) {
        top = (size_t)((char *)c - (char *)base);
        tail = c->mp;
        return;
    }

    c->next = recycle;
    recycle = c;
}

void *myrealloc(void *ptr, size_t size) {
    if (size == 0) { myfree(ptr); return NULL; }
    if (ptr == NULL) return mymalloc(size);

    chunk_t *c = (chunk_t *)ptr - 1;

    if (size <= c->sz) {
        // shrink -- split the tail off if theres room
        if (c->sz >= size + sizeof(chunk_t) + 1) {
            chunk_t *right = (chunk_t *)((char *)(c + 1) + size);
            right->sz = c->sz - size - sizeof(chunk_t);
            right->used = 0;
            right->next = NULL;
            right->mp = c;
            c->sz = size;
            chunk_t *after = mem_next(right);
            if (after != NULL) after->mp = right;
            if (tail == c) tail = right;
            myfree((void *)(right + 1));  // reclaims if at end, otherwise recycles
        } else if ((char *)ptr + c->sz == (char *)base + top) {
            top -= c->sz - size;
            c->sz = size;
        }
        return ptr;
    }

    // growing -- extend in place if its the last chunk
    if ((char *)ptr + c->sz == (char *)base + top) {
        top += size - c->sz;
        c->sz = size;
        return ptr;
    }

    void *newptr = mymalloc(size);
    if (newptr != NULL) {
        memcpy(newptr, ptr, c->sz);
        myfree(ptr);
    }
    return newptr;
}
