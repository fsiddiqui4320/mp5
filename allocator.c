#include "allocator.h"
#include <string.h>

static void *base;
static size_t used;

void allocator_init(void *newbase) {
  base = newbase;
  used = 0;
}

void allocator_reset() { used = 0; }


void *mymalloc(size_t size) {
  void *ans = base + used;
  used += size;
  return ans;
}

void myfree(void *ptr) {
}

void *myrealloc(void *ptr, size_t size) {
  if (!size) { myfree(ptr); return NULL; }
  void *ans = mymalloc(size);
  if (ptr && ans) {
    memcpy(ans, ptr, size);
    myfree(ptr);
  }
  return ans;
}
