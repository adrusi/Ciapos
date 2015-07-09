#ifndef CIAPOS_UTF8_H
#define CIAPOS_UTF8_H

#include <stdint.h>
#include "chargen.h"

typedef uint32_t ciapos_codepoint;

typedef struct ciapos_utf8gen {
    int state;
    ciapos_chargen *source;
    unsigned char byte;
    int hasnext;
} ciapos_utf8gen;

void ciapos_utf8gen_init(ciapos_utf8gen *utf8, ciapos_chargen *src);
void ciapos_utf8gen_deinit(ciapos_utf8gen *utf8);

int ciapos_utf8gen_next(ciapos_utf8gen *self, ciapos_codepoint *out);

#endif
