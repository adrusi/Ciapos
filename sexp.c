#include "sexp.h"
#include "symbol.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

// This function doesn't perform garbage collection, it is meant to be called in the sweep stage of the gc to free
// associated memory.
void ciapos_gc_deinit(ciapos_gc_header *obj) {
    assert(obj->tag != CIAPOS_TAGSYM && obj->tag != CIAPOS_TAGINT && obj->tag != CIAPOS_TAGREAL);
    if (obj->tag == CIAPOS_TAGENV) {
        ciapos_sym2sexp_deinit(&((ciapos_environment *) obj)->bindings);
    }
    free(obj);
}

ciapos_sexp ciapos_mkstring(ciapos_gc_header **heap, size_t len, char *buf) {
    ciapos_string *str = malloc(sizeof(ciapos_string) + len);
    str->header = (ciapos_gc_header) { .tag = CIAPOS_TAGSTR, .mark = 0, .next = *heap };
    str->length = len;
    *heap = (ciapos_gc_header *) str;
    memcpy(str->buffer, buf, len);
    return (ciapos_sexp) { .tag = CIAPOS_TAGSTR, .debug_info = 0, .string = str };
}

ciapos_sexp ciapos_mkfunction(
    ciapos_gc_header **heap,
    ciapos_function_evaluator eval,
    ciapos_sexp fbody,
    ciapos_sexp env)
{
    ciapos_function *f = malloc(sizeof(ciapos_function));
    f->header = (ciapos_gc_header) { .tag = CIAPOS_TAGFN, .mark = 0, .next = *heap };
    f->evaluator = eval;
    f->fbody = fbody;
    f->env = env;
    *heap = (ciapos_gc_header *) f;
    return (ciapos_sexp) { .tag = CIAPOS_TAGFN, .debug_info = 0, .function = f };
}

ciapos_sexp ciapos_mkopaque(ciapos_gc_header **heap, ciapos_symbol description, size_t len) {
    ciapos_opaque *o = malloc(sizeof(ciapos_opaque) + len);
    o->header = (ciapos_gc_header) { .tag = CIAPOS_TAGOPAQUE, .mark = 0, .next = *heap };
    o->description = description;
    o->size = len;
    *heap = (ciapos_gc_header *) o;
    return (ciapos_sexp) { .tag = CIAPOS_TAGOPAQUE, .debug_info = 0, .opaque = o };
}

ciapos_sexp ciapos_mkenvironment(ciapos_gc_header **heap, size_t nbuckets) {
    ciapos_environment *env = malloc(sizeof(ciapos_environment));
    env->header = (ciapos_gc_header) { .tag = CIAPOS_TAGENV, .mark = 0, .next = *heap };
    ciapos_sym2sexp_init(&env->bindings, nbuckets, NULL, NULL);
    *heap = (ciapos_gc_header *) env;
    return (ciapos_sexp) { .tag = CIAPOS_TAGENV, .debug_info = 0, .environment = env };
}

ciapos_sexp ciapos_mktuple(ciapos_gc_header **heap, size_t len) {
    ciapos_tuple *tup = malloc(sizeof(ciapos_tuple) + len * sizeof(ciapos_sexp));
    tup->header = (ciapos_gc_header) { .tag = CIAPOS_TAGTUP, .mark = 0, .next = *heap };
    tup->length = len;
    memset(tup->buffer, 0, len * sizeof(ciapos_sexp));
    *heap = (ciapos_gc_header *) tup;
    return (ciapos_sexp) { .tag = CIAPOS_TAGTUP, .debug_info = 0, .tuple = tup };
}

#ifndef NDEBUG
#include <stdio.h>
static void print_tuple(ciapos_symreg *registry, ciapos_sexp sexp) {
    for (ptrdiff_t i = 0; i < sexp.tuple->length - 1; i++) {
        ciapos_sexp_debug(registry, sexp.tuple->buffer[i]);
        printf(" . ");
    }
    ciapos_sexp_debug(registry, sexp.tuple->buffer[sexp.tuple->length - 1]);
    printf(")");
}
static void print_environment(ciapos_symreg *registry, ciapos_sexp sexp) {
    printf("{ENV");
    ciapos_sym2sexp_iterator i;
    ciapos_sym2sexp_iterator_init(&i, &sexp.environment->bindings);
    ciapos_symbol identifier;
    ciapos_sexp value;
    while (ciapos_sym2sexp_iterator_next(&i, &identifier, &value)) {
        printf(" ");
        ciapos_sexp_debug(registry, ciapos_mksymbol(identifier));
        printf("=");
        ciapos_sexp_debug(registry, value);
    }
    ciapos_sym2sexp_iterator_deinit(&i);
    printf("}");
}
void ciapos_sexp_debug(ciapos_symreg *registry, ciapos_sexp sexp) {
    switch (sexp.tag) {
    case CIAPOS_TAGNIL:
        printf("()");
        break;
    case CIAPOS_TAGSYM:
        if (!registry) {
            printf("<SYM %d>", sexp.symbol);
            break;
        }
        printf("%s", ciapos_symname(registry, sexp.symbol));
        break;
    case CIAPOS_TAGINT:
        printf("%ld", sexp.integer);
        break;
    case CIAPOS_TAGREAL:
        printf("%f", sexp.real);
        break;
    case CIAPOS_TAGSTR:
        printf("\"");
        for (ptrdiff_t i = 0; i < sexp.string->length; i++) {
            printf("%c", sexp.string->buffer[i]);
        }
        printf("\"");
        break;
    case CIAPOS_TAGFN:
        printf("<FUNCTION>");
        break;
    case CIAPOS_TAGOPAQUE:
        printf("<OPAQUE>");
        break;
    case CIAPOS_TAGENV:
        print_environment(registry, sexp);
        break;
    default:
        printf("#%s", ciapos_sym2str_get(&registry->sym2str, sexp.tag));
        // fallthrough
    case CIAPOS_TAGTUP:
        printf("(");
        print_tuple(registry, sexp);
    }
}
#endif
