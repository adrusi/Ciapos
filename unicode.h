#ifndef CIAPOS_UNICODE_H
#define CIAPOS_UNICODE_H

#include <stdlib.h>
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
    ciapos_codepoint *result = malloc(sizeof(ciapos_codepoint));
    if (!ciapos_utf8gen_next(self->src, result)) {
        free(result);
        return NULL;
    }
    return result;
}

#endif
