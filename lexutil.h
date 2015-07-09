#ifndef CIAPOS_LEXUTIL_H
#define CIAPOS_LEXUTIL_H

#include "unicode.h"
#include <stddef.h>
#include "slice.h"

CIAPOS_SLICE(ciapos_graphemebuf, ciapos_codepoint *)

typedef struct ciapos_graphemerewinder {
    ciapos_graphemebuf buffer;
    ptrdiff_t offset;
    ciapos_graphemegen *src;
} ciapos_graphemerewinder;

void ciapos_graphemerewinder_init(ciapos_graphemerewinder *self, ciapos_graphemegen *src);
void ciapos_graphemerewinder_deinit(ciapos_graphemerewinder *self);
ciapos_codepoint *ciapos_graphemerewinder_next(ciapos_graphemerewinder *self);
void ciapos_graphemerewinder_rewind(ciapos_graphemerewinder *self);
void ciapos_graphemerewinder_flush(ciapos_graphemerewinder *self);

#endif
