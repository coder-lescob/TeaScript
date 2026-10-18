#ifndef VEC_H
#define VEC_H

#include <stddef.h>

struct Vec {
  void* data; 
  size_t count; 
  size_t capacity;
  size_t sizeof_item;
};

#define vec_new(T) \
  ( (struct Vec) { .data = NULL, .count = 0, .capacity = 0, .sizeof_item = sizeof(T) } )

/**
 * grow the vector to make it's capacity `capacity`.
 */
int vec_with_capacity(struct Vec *vec, size_t capacity);

/**
 * push an item to a vector.
 * WARNING: the sizeof *item should always be vec.sizeof_item
 * NOTE: NULL makes uninitilized slot
 */
int vec_push(struct Vec *vec, void *item);

/**
 * pops an element from the vector
 * WARNING: the sizeof *item should always be vec.sizeof_item
 */
int vec_pop(struct Vec *vec, void *item);

/**
 * free a vector
 */
void vec_free(struct Vec *vec);

#endif
