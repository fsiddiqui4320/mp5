#pragma once
#include <stddef.h>  // for size_t

/**
 * Called once before any other function here.
 * Argument is the smallest usable address (start of the heap pool).
 */
void allocator_init(void *newbase);

/**
 * Called once before each test case.
 * Free any used memory and reset for the next test.
 */
void allocator_reset(void);

/**
 * Like malloc but using the memory provided to allocator_init.
 * Returns a pointer to at least `size` bytes of usable memory,
 * or NULL if allocation fails.
 */
void *mymalloc(size_t size);

/**
 * Like free but using the memory provided to allocator_init.
 * ptr must be a pointer previously returned by mymalloc/myrealloc,
 * or NULL (in which case this is a no-op).
 */
void myfree(void *ptr);

/**
 * Like realloc but using the memory provided to allocator_init.
 * - If ptr is NULL, behaves like mymalloc(size).
 * - If size is 0, behaves like myfree(ptr) and returns NULL.
 * - Otherwise resizes the block pointed to by ptr to size bytes
 *   and returns a pointer to the (possibly moved) block.
 */
void *myrealloc(void *ptr, size_t size);
