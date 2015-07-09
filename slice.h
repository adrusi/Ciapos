#ifndef CIAPOS_SLICE_H
#define CIAPOS_SLICE_H

#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>

#define CIAPOS_SLICE(ID, VAL) \
typedef void (*ID##_val_deinit)(VAL val); \
 \
typedef struct ID { \
    size_t cap; \
    size_t len; \
    ID##_val_deinit val_deinit; \
    VAL *buf; \
} ID; \
 \
static inline void ID##_init( \
    ID *slice, \
    size_t cap, \
    ID##_val_deinit deinit) \
{ \
    if (!cap) cap = 1; \
    slice->cap = cap; \
    slice->len = 0; \
    slice->val_deinit = deinit; \
    slice->buf = malloc(cap * sizeof(VAL)); \
} \
 \
static inline void ID##_deinit(ID *slice) { \
    if (slice->val_deinit) \
    for (int i = 0; i < slice->len; i++) slice->val_deinit(slice->buf[i]); \
    free(slice->buf); \
} \
 \
static inline VAL ID##_get(ID *slice, off_t idx) { \
    assert(idx < slice->len); \
    return slice->buf[idx]; \
} \
 \
static inline void ID##_put(ID *slice, off_t idx, VAL val) { \
    assert(idx < slice->len); \
    slice->buf[idx] = val; \
} \
 \
static inline void ID##_append(ID *slice, VAL val) { \
    slice->buf[slice->len] = val; \
    if (++slice->len > slice->cap) { \
        slice->cap *= 2; \
        slice->buf = realloc(slice->buf, slice->cap * sizeof(VAL)); \
    } \
}

#endif
