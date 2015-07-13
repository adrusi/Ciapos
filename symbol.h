#ifndef CIAPOS_SYMBOL_H
#define CIAPOS_SYMBOL_H

#include <stdint.h>
#include "dict.h"
#include "slice.h"

typedef uint32_t ciapos_symbol;

static inline unsigned long ciapos_str_hash(char const *_str) {
    unsigned char const *str = (unsigned char const *) _str;
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) hash = ((hash << 5) + hash) + c;
    return hash;
}
static inline int ciapos_str_eq(char const *a, char const *b) {
    if (!a && !b) return 1;
    if ((!a && b) || (a && !b)) return 0;
    while (*a == *b && *a) a++, b++;
    return *a == *b;
}
CIAPOS_DICT(
    ciapos_str2sym,
    char const *,
    ciapos_symbol,
    ciapos_str_hash,
    ciapos_str_eq)

CIAPOS_SLICE(ciapos_sym2str, char const *)

typedef struct {
    uint32_t symcnt;
    ciapos_sym2str sym2str;
    ciapos_str2sym str2sym;
} ciapos_symreg;

#define CIAPOS_TAGNIL 0
#define CIAPOS_TAGSYM 1
#define CIAPOS_TAGINT 2
#define CIAPOS_TAGREAL 3
#define CIAPOS_TAGSTR 4
#define CIAPOS_TAGFN 5
#define CIAPOS_TAGOPAQUE 6
#define CIAPOS_TAGENV 7
#define CIAPOS_TAGTUP 8

void ciapos_symreg_init(ciapos_symreg *registry);
void ciapos_symreg_deinit(ciapos_symreg *registry);
ciapos_symbol ciapos_symbolof(ciapos_symreg *registry, char const *str);

#endif

