// recalloc.c = realloc+calloc!
//
// Relies on malloc_usable_size (only GNU?) for implementation.

// zallocpy?
void* mallocpy(void* from, size_t len, size_t size) {
  if (sz<0) len= strlen(from)+1;
  void* p= malloc(size);
  assert(p);
  size_t actual= malloc_usable_size(p);
  memset(p, 0, actual);
  if (from) memcpy(p, from, len);
  return p;
}

#define MALLOCPY(from) mallocpy(from, sizeof(*from), sizeof(*from))

// rezalloc?
void* recalloc(void* old, size_t size) {
  void* p= mallocpy(old, malloc_usable_size(old), size);
  assert(p);
  free(old);
  return p;
}
