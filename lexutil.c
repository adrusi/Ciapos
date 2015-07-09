#include "lexutil.h"
#include <stdlib.h>

void ciapos_graphemerewinder_init(ciapos_graphemerewinder *self, ciapos_graphemegen *src) {
    ciapos_graphemebuf_init(&self->buffer);
    self->offset = 0;
    self->src = src;
}
    
void ciapos_graphemerewinder_deinit(ciapos_graphemerewinder *self) {
    ciapos_graphemerewinder_flush(self);
    ciapos_graphemebuf_deinit(&self->buffer);
}

ciapos_codepoint *ciapos_graphemerewinder_next(ciapos_graphemerewinder *self) {
    if (self->offset < self->buffer.len) {
        return ciapos_graphemebuf_get(&self->buffer, self->offset++);
    }
    ciapos_codepoint *result = ciapos_graphemegen_next(self->src);
    ciapos_graphemebuf_append(&self->buffer, result);
    self->offset++;
    return result;
}

void ciapos_graphemerewinder_rewind(ciapos_graphemerewinder *self) {
    self->offset = 0;
}

void ciapos_graphemerewinder_flush(ciapos_graphemerewinder *self) {
    // TODO is shifting the array efficient enough?
    for (ptrdiff_t i = 0; i < self->offset && i + self->offset < self->buffer.len; i++) {
        ciapos_graphemebuf_put(
            &self->buffer, i,
            ciapos_graphemebuf_get(&self->buffer, i + self->offset));
    }
    self->buffer.len -= self->offset;
    self->offset = 0;
}
