#ifndef CIAPOS_CHARGEN_H
#define CIAPOS_CHARGEN_H

#include <stdio.h>

typedef struct ciapos_chargen {
    int (*next)(void *ctx, char *out);
    void *ctx;
} ciapos_chargen;

void ciapos_file_chargen_init(ciapos_chargen *chargen, FILE *f);
void ciapos_string_chargen_init(ciapos_chargen *chargen, char const *str);
void ciapos_chargen_deinit(ciapos_chargen *chargen);

static inline int ciapos_chargen_next(ciapos_chargen *chargen, char *out) {
    return chargen->next(chargen->ctx, out);
}

#endif
