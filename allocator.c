#include "allocator.h"
#include <string.h>

// block header stored right before the pointer we return
typedef struct block {
    size_t size;
    int free;
    struct block *next; // next free block
} block_t;

static void *base;
static size_t heap_used;
static block_t *freelist;

void allocator_init(void *newbase) {
    base = newbase;
    heap_used = 0;
    freelist = NULL;
}

void allocator_reset() {
    heap_used = 0;
    freelist = NULL;
}

void *mymalloc(size_t size) {
    if (size == 0) return NULL;

    // search freelist for a block thats big enough
    block_t *prev = NULL;
    block_t *cur = freelist;
    while (cur != NULL) {
        if (cur->size >= size) {
            // take it out of the free list
            if (prev != NULL) prev->next = cur->next;
            else freelist = cur->next;
            cur->free = 0;
            cur->next = NULL;
            return (void *)(cur + 1);
        }
        prev = cur;
        cur = cur->next;
    }

    // no free block found, grab new memory from heap
    block_t *b = (block_t *)((char *)base + heap_used);
    b->size = size;
    b->free = 0;
    b->next = NULL;
    heap_used += sizeof(block_t) + size;
    return (void *)(b + 1);
}

void myfree(void *ptr) {
    if (ptr == NULL) return;

    block_t *b = (block_t *)ptr - 1;

    // if its the last block just move the heap pointer back
    char *block_end = (char *)ptr + b->size;
    if (block_end == (char *)base + heap_used) {
        heap_used = (size_t)((char *)b - (char *)base);
        return;
    }

    b->free = 1;
    b->next = freelist;
    freelist = b;
}

void *myrealloc(void *ptr, size_t size) {
    if (size == 0) { myfree(ptr); return NULL; }
    if (ptr == NULL) return mymalloc(size);

    block_t *b = (block_t *)ptr - 1;

    if (size <= b->size) {
        // shrinking -- give memory back if its the last block
        char *block_end = (char *)ptr + b->size;
        if (block_end == (char *)base + heap_used) {
            heap_used -= b->size - size;
            b->size = size;
        }
        return ptr;
    }

    // growing -- if its the last block just extend it
    char *block_end = (char *)ptr + b->size;
    if (block_end == (char *)base + heap_used) {
        heap_used += size - b->size;
        b->size = size;
        return ptr;
    }

    // otherwise alloc new block and copy
    void *newptr = mymalloc(size);
    if (newptr != NULL) {
        memcpy(newptr, ptr, b->size);
        myfree(ptr);
    }
    return newptr;
}
