#include "allocator.h"
#include <string.h>

typedef struct chunk {
    size_t sz;
    int used;
    struct chunk *next;
} chunk_t;

static void *base;
static size_t top;
static chunk_t *recycle;

void allocator_init(void *newbase) {
    base = newbase;
    top = 0;
    recycle = NULL;
}

void allocator_reset() {
    top = 0;
    recycle = NULL;
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
            c->used = 1;
            c->next = NULL;
            return (void *)(c + 1);
        }
        prev = c;
        c = c->next;
    }

    chunk_t *newchunk = (chunk_t *)((char *)base + top);
    newchunk->sz = size;
    newchunk->used = 1;
    newchunk->next = NULL;
    top += sizeof(chunk_t) + size;
    return (void *)(newchunk + 1);
}

void myfree(void *ptr) {
    if (ptr == NULL) return;

    chunk_t *c = (chunk_t *)ptr - 1;

    // if this chunk is at the end of the heap, just shrink top
    if ((char *)ptr + c->sz == (char *)base + top) {
        top = (size_t)((char *)c - (char *)base);
        return;
    }

    c->used = 0;
    c->next = recycle;
    recycle = c;
}

void *myrealloc(void *ptr, size_t size) {
    if (size == 0) { myfree(ptr); return NULL; }
    if (ptr == NULL) return mymalloc(size);

    chunk_t *c = (chunk_t *)ptr - 1;

    if (size <= c->sz) {
        if ((char *)ptr + c->sz == (char *)base + top) {
            top -= c->sz - size;
            c->sz = size;
        }
        return ptr;
    }

    // growing -- if its at the end just bump top
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
