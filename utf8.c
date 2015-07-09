#include "utf8.h"
#include "generator.h"

void ciapos_utf8gen_init(ciapos_utf8gen *utf8, ciapos_chargen *src) {
    utf8->state = 0;
    utf8->source = src;
    utf8->byte = '\0';
    utf8->hasnext = 0;
}

static void read_in_continuation_bytes(int nbytes, ciapos_chargen *src, ciapos_codepoint *out) {
    while (nbytes--) {
        char _c;
        if (ciapos_chargen_next(src, &_c)) return;
        ciapos_codepoint c = _c;
        c <<= 6 * nbytes;
        *out |= c;
    }
}

int ciapos_utf8gen_next(ciapos_utf8gen *self, ciapos_codepoint *out) {
    begin_generator(self->state);
    // note that switch statements are forbidden within a generator
    
    *out = 0x00;
    while (ciapos_chargen_next(self->source, (char *) &self->byte)) {
        if ((self->byte & 0x80) == 0x00) { // single byte sequence
            *out = self->byte;
            yield (1);
        } else if ((self->byte & 0xE0) == 0xC0) { // 2-byte sequence
            self->byte &= ~0xE0;
            *out = self->byte;
            *out <<= 6;
            read_in_continuation_bytes(1, self->source, out);
            yield (1);
        } else if ((self->byte & 0xF0) == 0xE0) { // 3-byte sequence
            self->byte &= ~0xF0;
            *out = self->byte;
            *out <<= 12;
            read_in_continuation_bytes(2, self->source, out);
            yield (1);
        } else if ((self->byte & 0xF8) == 0xF0) { // 4-byte sequence
            self->byte &= ~0xF8;
            *out = self->byte;
            *out <<= 18;
            read_in_continuation_bytes(3, self->source, out);
            yield (1);
        } else if ((self->byte & 0xFC) == 0xF8) { // 5-byte sequence
            self->byte &= ~0xFC;
            *out = self->byte;
            *out <<= 24;
            read_in_continuation_bytes(4, self->source, out);
            yield (1);
        } else if ((self->byte & 0xFE) == 0xFC) { // 6-byte sequence
            self->byte &= ~0xFE;
            *out = self->byte;
            *out <<= 30;
            read_in_continuation_bytes(5, self->source, out);
            yield (1);
        }
    }
    yield (0);

    end_generator;
    return 0;
}

void ciapos_utf8gen_deinit(ciapos_utf8gen *utf8gen) {
    // nothing to do
}
