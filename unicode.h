#ifndef CIAPOS_UNICODE_H
#define CIAPOS_UNICODE_H

#include <stdlib.h>
#include <stddef.h>
#include "utf8.h"

typedef struct ciapos_graphemegen {
    ciapos_utf8gen *src;
} ciapos_graphemegen;

static inline void ciapos_graphemegen_init(ciapos_graphemegen *self, ciapos_utf8gen *src) {
    self->src = src;
}

static inline void ciapos_graphemegen_deinit(ciapos_graphemegen *self) {}

static inline ciapos_codepoint *ciapos_graphemegen_next(ciapos_graphemegen *self) {
    // TODO actually implement this using unicode tables
    ciapos_codepoint *result = calloc(2, sizeof(ciapos_codepoint)); // '\0' terminated
    if (!ciapos_utf8gen_next(self->src, result)) {
        free(result);
        return NULL;
    }
    return result;
}

size_t ciapos_grapheme_len(ciapos_codepoint *grapheme);

int ciapos_is_whitespace(ciapos_codepoint *grapheme);
int ciapos_is_newline(ciapos_codepoint *grapheme);

int ciapos_is_opening_bracket(ciapos_codepoint *grapheme);
int ciapos_is_closing_bracket(ciapos_codepoint *grapheme);
int ciapos_are_matching_brackets(ciapos_codepoint *open, ciapos_codepoint *close);

int ciapos_is_opening_quote(ciapos_codepoint *grapheme);
int ciapos_is_closing_quote(ciapos_codepoint *grapheme);
int ciapos_are_matching_quotes(ciapos_codepoint *open, ciapos_codepoint *close);

int ciapos_is_numeric(ciapos_codepoint *grapheme);
int ciapos_is_sign(ciapos_codepoint *grapheme);

#endif
