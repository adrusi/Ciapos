#ifndef CIAPOS_LEXUTIL_H
#define CIAPOS_LEXUTIL_H

#include "unicode.h"
#include <stddef.h>
#include "slice.h"

CIAPOS_SLICE(ciapos_graphemebuf, ciapos_codepoint *)

// ciapos_graphemerewinder simplifies the lookahead functionality of the lexer.
// It also manages the heap-allocated memory for graphemes.
typedef struct ciapos_graphemerewinder {
    ciapos_graphemebuf buffer;
    ptrdiff_t offset;
    ciapos_graphemegen *src;
} ciapos_graphemerewinder;

void ciapos_graphemerewinder_init(ciapos_graphemerewinder *self, ciapos_graphemegen *src);
void ciapos_graphemerewinder_deinit(ciapos_graphemerewinder *self);
// The grapheme returned is owned by the rewinder.
// It is lent to the caller until the next call to flush.
ciapos_codepoint *ciapos_graphemerewinder_next(ciapos_graphemerewinder *self);
int ciapos_graphemerewinder_hasnext(ciapos_graphemerewinder *self);
void ciapos_graphemerewinder_rewind(ciapos_graphemerewinder *self);
void ciapos_graphemerewinder_rewindby(ciapos_graphemerewinder *self, ptrdiff_t distance);
void ciapos_graphemerewinder_flush(ciapos_graphemerewinder *self);

#endif
