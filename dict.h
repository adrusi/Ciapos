#ifndef CIAPOS_DICT_H
#define CIAPOS_DICT_H

#include <stdlib.h>

#define CIAPOS_DICT(ID, KEY, VAL, HASH, EQ) \
typedef struct ID##_bucket { \
    KEY key; \
    VAL val; \
    struct ID##_bucket *next; \
} ID##_bucket; \
 \
typedef void (*ID##_key_deinit)(KEY key); \
typedef void (*ID##_val_deinit)(VAL val); \
     \
typedef struct ID { \
    size_t nbuckets; \
    ID##_key_deinit key_deinit; \
    ID##_val_deinit val_deinit; \
    ID##_bucket **buckets; \
} ID; \
 \
static inline void ID##_init( \
    ID *dict, \
    int nbuckets, \
    ID##_key_deinit key_deinit, \
    ID##_val_deinit val_deinit) \
{ \
    dict->nbuckets = nbuckets; \
    dict->buckets = calloc(sizeof(ID##_bucket *), nbuckets); \
    dict->key_deinit = key_deinit; \
    dict->val_deinit = val_deinit; \
} \
 \
static inline void ID##_deinit(ID *dict) { \
    for (int i = 0; i < dict->nbuckets; i++) { \
        ID##_bucket *b = dict->buckets[i]; \
        while (b) { \
            if (dict->key_deinit) dict->key_deinit(b->key); \
            if (dict->val_deinit) dict->val_deinit(b->val); \
            ID##_bucket *b_ = b->next; \
            free(b); \
            b = b_; \
        } \
    } \
    free(dict->buckets); \
} \
 \
static inline void ID##_put(ID *dict, KEY key, VAL val) { \
    ID##_bucket **bucket = &dict->buckets[HASH(key) % dict->nbuckets]; \
    while (*bucket) { \
        if (EQ(key, bucket[0]->key)) { \
            bucket[0]->val = val; \
            return; \
        } \
        bucket = &bucket[0]->next; \
    } \
    *bucket = malloc(sizeof(ID##_bucket)); \
    bucket[0]->key = key; \
    bucket[0]->val = val; \
    bucket[0]->next = NULL; \
} \
 \
static inline VAL ID##_get(ID *dict, KEY key) { \
    ID##_bucket *b = dict->buckets[HASH(key) % dict->nbuckets]; \
    while (b) { \
        if (EQ(key, b->key)) return b->val; \
        b = b->next; \
    } \
    VAL nil; \
    unsigned char *byte = (unsigned char *) &nil; \
    for (size_t i = 0; i < sizeof(VAL); i++, byte++) *byte = 0; \
    return nil; \
} \
 \
static inline VAL *ID##_ref(ID *dict, KEY key) { \
    ID##_bucket *b = dict->buckets[HASH(key) % dict->nbuckets]; \
    while (b) { \
        if (EQ(key, b->key)) return &b->val; \
        b = b->next; \
    } \
    return NULL; \
} \
 \
static inline int ID##_has(ID *dict, KEY key) { \
    ID##_bucket *b = dict->buckets[HASH(key) % dict->nbuckets]; \
    while (b) { \
        if (EQ(key, b->key)) return 1; \
        b = b->next; \
    } \
    return 0; \
} \
 \
typedef struct ID##_iterator { \
    ID *src; \
    ptrdiff_t bucketid; \
    ID##_bucket *bucket; \
} ID##_iterator; \
 \
static inline void ID##_iterator_init(ID##_iterator *self, ID *src) { \
    self->src = src; \
    self->bucketid = 0; \
    self->bucket = src->buckets[self->bucketid]; \
} \
 \
static inline void ID##_iterator_deinit(ID##_iterator *self) {} \
 \
static inline int ID##_iterator_next(ID##_iterator *self, KEY *key, VAL *val) { \
    if (self->bucketid == self->src->nbuckets) return 0; \
    while (!self->bucket) { \
        self->bucketid++; \
        if (self->bucketid == self->src->nbuckets) return 0; \
        self->bucket = self->src->buckets[self->bucketid]; \
    } \
    *key = self->bucket->key; \
    *val = self->bucket->val; \
    self->bucket = self->bucket->next; \
    return 1; \
}


#endif
