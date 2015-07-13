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
    ID##_bucket *buckets; \
} ID; \
 \
static inline void ID##_init( \
    ID *dict, \
    int nbuckets, \
    ID##_key_deinit key_deinit, \
    ID##_val_deinit val_deinit) \
{ \
    dict->nbuckets = nbuckets; \
    dict->buckets = calloc(sizeof(ID##_bucket), nbuckets); \
    dict->key_deinit = key_deinit; \
    dict->val_deinit = val_deinit; \
} \
 \
static inline void ID##_deinit(ID *dict) { \
    for (int i = 0; i < dict->nbuckets; i++) { \
        ID##_bucket *b = &dict->buckets[i]; \
        if (dict->key_deinit) dict->key_deinit(b->key); \
        if (dict->val_deinit) dict->val_deinit(b->val); \
        b = b->next; \
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
    ID##_bucket *bucket = &dict->buckets[HASH(key) % dict->nbuckets], \
        *b = bucket, *oldbucket; \
    do { \
        if (EQ(key, b->key)) { \
            b->val = val; \
            return; \
        } \
    } while ((b = b->next)); \
    oldbucket = malloc(sizeof(ID##_bucket)); \
    *oldbucket = *bucket; \
    bucket->key = key; \
    bucket->val = val; \
    bucket->next = oldbucket; \
} \
 \
static inline VAL ID##_get(ID *dict, KEY key) { \
    ID##_bucket *b = &dict->buckets[HASH(key) % dict->nbuckets]; \
    do { \
        if (EQ(key, b->key)) return b->val; \
    } while ((b = b->next)); \
    VAL nil; \
    unsigned char *byte = (unsigned char *) &nil; \
    for (size_t i = 0; i < sizeof(VAL); i++, byte++) *byte = 0; \
    return nil; \
} \
 \
static inline VAL *ID##_ref(ID *dict, KEY key) { \
    ID##_bucket *b = &dict->buckets[HASH(key) % dict->nbuckets]; \
    do { \
        if (EQ(key, b->key)) return &b->val; \
    } while ((b = b->next)); \
    return NULL; \
} \
 \
static inline int ID##_has(ID *dict, KEY key) { \
    ID##_bucket *b = &dict->buckets[HASH(key) % dict->nbuckets]; \
    do { \
        if (EQ(key, b->key)) return 1; \
    } while ((b = b->next)); \
    return 0; \
}

#endif
