#include "vec.h"

#include <stdlib.h>
#include <errno.h>
#include <string.h>

/**
 * grow the vector to make it's capacity `capacity`.
 */
int vec_with_capacity(struct Vec *vec, size_t capacity) {
  if (vec == NULL || capacity <= vec->capacity) { 
    errno  = EINVAL;
    return -1; 
  }
  
  // reallocate the vector
  void *new_alloc = realloc(vec->data, capacity * vec->sizeof_item);
  if (new_alloc == NULL) {
    return -1; 
  }
  
  // mutate the vector
  vec->data     = new_alloc;
  vec->capacity = capacity;

  return 0;
}

/**
 * push an item to a vector.
 * WARNING: the sizeof *item should always be vec.sizeof_item
 * NOTE: NULL makes uninitialized slot
 */
int vec_push(struct Vec *vec, void *item) {
  if (vec == NULL) { 
    errno = EINVAL;
    return -1; 
  }

  if (vec->count + 1 > vec->capacity) {

    // grow the vector to about twice it's capacity, +1 to avoid edge case of 0
    size_t new_capacity = 2 * vec->capacity + 1;
    if (vec_with_capacity(vec, new_capacity) != 0) 
      return -1; // allocation failed
  }

  if (item != NULL) {
    // copy the item
    memcpy((char *)vec->data + vec->count * vec->sizeof_item, item, vec->sizeof_item);
  }

  // grow the count
  vec->count++;

  return 0;
}

/**
 * pops an element from the vector
 * WARNING: the sizeof *item should always be vec.sizeof_item
 */
int vec_pop(struct Vec *vec, void *item) {
  if (vec == NULL) return -1;
  
  // get the index of the item to pop and degrow the vector
  size_t item_idx = --vec->count;
  
  // copy the item if needed
  if (item != NULL) {
    memcpy(item, (char *)vec->data + item_idx * vec->sizeof_item, vec->sizeof_item);
  }

  return 0;
}

/**
 * free a vector
 */
void vec_free(struct Vec *vec) {
  if (vec == NULL) return;

  if (vec->data != NULL) {
    free(vec->data);
    vec->data = NULL;
  }

  vec->capacity = 0;
  vec->count    = 0;
}
