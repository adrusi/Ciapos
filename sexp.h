#ifndef CIAPOS_SEXP_H
#define CIAPOS_SEXP_H

#include <stdint.h>
#include <sys/types.h>
#include <assert.h>
#include "symbol.h"
#include "dict.h"

typedef ciapos_symbol tagged;

typedef struct ciapos_gc_header {
    ciapos_symbol tag;
    unsigned char mark;
    struct ciapos_gc_header *next;
} ciapos_gc_header;

typedef int64_t ciapos_integer;

typedef double ciapos_real;

typedef struct ciapos_string {
    ciapos_gc_header header;
    size_t length;
    char buffer[];
} ciapos_string;

typedef struct ciapos_opaque {
    ciapos_gc_header header;
    ciapos_symbol description;
    size_t size;
    uint8_t buffer[]; // follows 64-bit int, so should sufficiently aligned for whatever data is put here
} ciapos_opaque;

typedef struct ciapos_sexp {
    ciapos_symbol tag;
    uint32_t debug_info; // line/col/file info for sexps read from source code
    union {
        ciapos_symbol symbol;
        ciapos_integer integer;
        ciapos_real real;
        ciapos_string *string;
        struct ciapos_function *function;
        ciapos_opaque *opaque;
        struct ciapos_environment *environment;
        struct ciapos_tuple *tuple;
    };
} ciapos_sexp;

typedef ciapos_sexp (*ciapos_function_evaluator)(
    ciapos_sexp fbody,
    ciapos_sexp env,
    ciapos_sexp args);

typedef struct ciapos_function {
    ciapos_gc_header header;
    ciapos_function_evaluator evaluator;
    ciapos_sexp fbody;
    ciapos_sexp env;
} ciapos_function;

typedef struct ciapos_tuple {
    ciapos_gc_header header;
    size_t length;
    ciapos_sexp buffer[];
} ciapos_tuple;

// TODO better hash
#define CIAPOS_DICT_HASH(a) (a)
#define CIAPOS_DICT_EQ(a, b) ((a) == (b))
CIAPOS_DICT(
    ciapos_sym2sexp,
    ciapos_symbol,
    ciapos_sexp,
    CIAPOS_DICT_HASH,
    CIAPOS_DICT_EQ)

typedef struct ciapos_environment {
    ciapos_gc_header header;
    ciapos_sym2sexp bindings;
} ciapos_environment;

void ciapos_gc_deinit(ciapos_gc_header *obj);

static inline ciapos_sexp ciapos_mknil(void) {
    return (ciapos_sexp) { .tag = CIAPOS_TAGNIL, .debug_info = 0 };
}
static inline ciapos_sexp ciapos_mksymbol(ciapos_symbol a) {
    return (ciapos_sexp) { .tag = CIAPOS_TAGSYM, .debug_info = 0, .symbol = a };
}
static inline ciapos_sexp ciapos_mkinteger(ciapos_integer a) {
    return (ciapos_sexp) { .tag = CIAPOS_TAGINT, .debug_info = 0, .integer = a };
}
static inline ciapos_sexp ciapos_mkreal(ciapos_real a) {
    return (ciapos_sexp) { .tag = CIAPOS_TAGREAL, .debug_info = 0, .real = a };
}
ciapos_sexp ciapos_mkstring(ciapos_gc_header **heap, size_t len, char *buf);
ciapos_sexp ciapos_mkfunction(ciapos_gc_header **heap, ciapos_function_evaluator, ciapos_sexp fbody, ciapos_sexp env);
ciapos_sexp ciapos_mkopaque(ciapos_gc_header **heap, ciapos_symbol description, size_t len);
ciapos_sexp ciapos_mkenvironment(ciapos_gc_header **heap, size_t nbuckets);
ciapos_sexp ciapos_mktuple(ciapos_gc_header **heap, size_t len);

static inline ciapos_sexp ciapos_function_eval(ciapos_sexp function, ciapos_sexp args) {
    assert(function.tag == CIAPOS_TAGFN);
    return function.function->evaluator(function.function->fbody, function.function->env, args);
}

static inline void *ciapos_opaque_get(ciapos_sexp a) {
    assert(a.tag == CIAPOS_TAGOPAQUE);
    return a.opaque->buffer;
}

static inline int ciapos_environment_has(ciapos_sexp a, ciapos_symbol key) {
    assert(a.tag == CIAPOS_TAGENV);
    return ciapos_sym2sexp_has(&a.environment->bindings, key);
}
static inline ciapos_sexp ciapos_environment_get(ciapos_sexp a, ciapos_symbol key) {
    assert(a.tag == CIAPOS_TAGENV);
    return ciapos_sym2sexp_get(&a.environment->bindings, key);
}
static inline void ciapos_environment_put(ciapos_sexp a, ciapos_symbol key, ciapos_sexp val) {
    assert(a.tag == CIAPOS_TAGENV);
    ciapos_sym2sexp_put(&a.environment->bindings, key, val);
}

static inline ciapos_sexp ciapos_tuple_get(ciapos_sexp a, off_t idx) {
    assert(a.tag >= CIAPOS_TAGTUP);
    return a.tuple->buffer[idx];
}
static inline ciapos_sexp *ciapos_tuple_ref(ciapos_sexp a, off_t idx) {
    assert(a.tag >= CIAPOS_TAGTUP);
    return &a.tuple->buffer[idx];
}
static inline void ciapos_tuple_put(ciapos_sexp a, off_t idx, ciapos_sexp b) {
    assert(a.tag >= CIAPOS_TAGTUP);
    a.tuple->buffer[idx] = b;
}

static inline void ciapos_sexp_set_type(ciapos_sexp *sexp, ciapos_symbol type) {
    assert(sexp->tag >= CIAPOS_TAGTUP);
    sexp->tag = type;
    sexp->tuple->header.tag = type;
}

ciapos_sexp ciapos_show(ciapos_sexp sexp);

#endif
