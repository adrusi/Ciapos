#ifndef CIAPOS_READ_H
#define CIAPOS_READ_H

#include "token.h"
#include "sexp.h"
#include "symbol.h"

typedef enum ciapos_reader_error {
    CIAPOS_READERR_END_OF_STREAM = 1,
    CIAPOS_READERR_UNEXPECTED_TOKEND,
    CIAPOS_READERR_UNEXPECTED_TOKDOT,
    CIAPOS_READERR_UNMATCHED_TOKBEG,
    CIAPOS_READERR_BRACKET_MISMATCH
} ciapos_reader_error;

typedef struct ciapos_reader {
    ciapos_tokengen *src;
    ciapos_symreg *registry;
    ciapos_gc_header **heap;
} ciapos_reader;

void ciapos_reader_init(ciapos_reader *self, ciapos_tokengen *src, ciapos_symreg *registry, ciapos_gc_header **heap);
void ciapos_reader_deinit(ciapos_reader *self);

ciapos_reader_error ciapos_reader_next(ciapos_reader *self, ciapos_sexp *sexp);

#endif
