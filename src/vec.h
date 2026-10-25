#ifndef _GENERIC_VEC_H
#define _GENERIC_VEC_H

#include <stddef.h>
#include <stdlib.h>
#include <errno.h>

#define VEC_DECL(T) \
  typedef struct { T* data; size_t count; size_t capacity; } Vec(T);  \
  int VEC_METHOD(with_capacity, T)(Vec(T) *vec, size_t capacity);     \
  int VEC_METHOD(push, T)(Vec(T) *vec, T item);                       \
  int VEC_METHOD(pop, T)(Vec(T) *vec, T *out_item);                   \
  int VEC_METHOD(get_item, T)(Vec(T) *vec, size_t idx, T *out_item);  \
  int VEC_METHOD(set_item, T)(Vec(T) *vec, size_t idx, T item);

#define Vec(T) __Vec(T)
#define __Vec(T) __CONCAT(Vec_,T)
#define GLUE(A, B) A##B
#define CONCAT4(A, B, C, D) A ## B ## C ## D

#define VEC_METHOD(name, T) CONCAT4(vec_,name, _, T)
#define vec_new(T) (Vec(T)) { 0 } 

#define vec_free(vec) \
  do {                \
    if (vec.data != NULL) { \
      free(vec.data);   \
      vec.data = NULL;  \
    }                 \
    vec.count    = 0; \
    vec.capacity = 0; \
  } while (0)

#define VEC_IMPL(T) \
  int VEC_METHOD(with_capacity, T)(Vec(T) *vec, size_t capacity) {    \
    if (vec == NULL || capacity <= vec->capacity) {             \
      errno = EINVAL;                                           \
      return -1;                                                \
    }                                                           \
                                                                \
    /* reallocate it! */                                        \
    T* new_region = realloc(vec->data, capacity * sizeof(T));   \
    if (new_region == NULL) return -1; /* realloc failed */     \
                                                                \
    /* success, mutate the vector */                            \
    vec->data     = new_region;                                 \
    vec->capacity = capacity;                                   \
    return 0;                                                   \
  }                                                             \
                                                                \
  int VEC_METHOD(push, T)(Vec(T) *vec, T item) {          \
    if (vec == NULL) {                                    \
      errno = EINVAL;                                     \
      return -1;                                          \
    }                                                     \
                                                          \
    if (vec->count + 1 > vec->capacity) {                 \
      /* try reallocate, +1 to avoid edge case of 0 */    \
      size_t new_cap = 2 * vec->capacity + 1;             \
      if (VEC_METHOD(with_capacity, T)(vec, new_cap) != 0)\
        return -1; /* reallocation failed */              \
    }                                                     \
                                                          \
    /* push it!! */                                       \
    vec->data[vec->count++] = item;                       \
    return 0;                                             \
  }                                                       \
                                                          \
  int VEC_METHOD(pop, T)(Vec(T) *vec, T *out_item) {      \
    if (vec == NULL || vec->count == 0 || vec->data == NULL || vec->capacity == 0) {     \
      errno = EINVAL;                                     \
      return -1;                                          \
    }                                                     \
                                                          \
    /* step down the length */                            \
    vec->count--;                                         \
    if (out_item != NULL) {                               \
      *out_item = vec->data[vec->count];                  \
    }                                                     \
    return 0;                                             \
  }                                                       \
                                                          \
  int VEC_METHOD(get_item, T)(Vec(T) *vec, size_t idx, T *out_item) { \
    if (vec == NULL || out_item == NULL || idx >= vec->count) {       \
      errno = EINVAL;                                                 \
      return -1;                                                      \
    }                                                                 \
                                                                      \
    *out_item = vec->data[idx];                                       \
    return 0;                                                         \
  }                                                                   \
                                                                      \
  int VEC_METHOD(set_item, T)(Vec(T) *vec, size_t idx, T item) {      \
    if (vec == NULL || idx >= vec->count) {                           \
      errno = EINVAL;                                                 \
      return -1;                                                      \
    }                                                                 \
                                                                      \
    vec->data[idx] = item;                                            \
    return 0;                                                         \
  }

#endif /* _GENERIC_VEC_H */
