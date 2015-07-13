#include "read.h"
#include "token.h"
#include "sexp.h"
#include "symbol.h"
#include "unicode.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// TODO ownership of token memory is convoluted. I think it's sound, but fragile. Clean up sometime.

void ciapos_reader_init(ciapos_reader *self, ciapos_tokengen *src, ciapos_symreg *registry, ciapos_gc_header **heap) {
    self->registry = registry;
    self->heap = heap;
    self->src = src;
}

void ciapos_reader_deinit(ciapos_reader *self) {}

static inline uint32_t debuginfo(ciapos_token tok) {
    // 11 bits of fileid
    // 13 bits of line
    // 8 bits of col
    // ffffffffffflllllllllllllcccccccc
    register uint32_t fileid = ~(~0<<11) & tok.fileid;
    register uint32_t line   = ~(~0<<13) & (tok.line - 1);
    register uint32_t col    = ~(~0<<8)  & (tok.col - 1);
    return (fileid << (13+8)) | (line << 8) | col;
}

static ciapos_reader_error read_tuple(
    ciapos_reader *self,
    ciapos_codepoint *bracket,
    ciapos_token tok,
    ciapos_sexp *sexp);
static ciapos_reader_error read(ciapos_reader *self, ciapos_token tok, ciapos_sexp *sexp);

ciapos_reader_error ciapos_reader_next(ciapos_reader *self, ciapos_sexp *sexp) {
    ciapos_token tok;
    if (!ciapos_tokengen_next(self->src, &tok)) {
        return CIAPOS_READERR_END_OF_STREAM;
    }
    ciapos_reader_error err = read(self, tok, sexp);
    ciapos_token_deinit(&tok);
    return err;
}

static ciapos_reader_error quote(ciapos_reader *self, ciapos_sexp *sexp, char const *quotefname) {
    *sexp = ciapos_mktuple(self->heap, 2);
    ciapos_tuple_put(*sexp, 0, ciapos_mksymbol(ciapos_symbolof(self->registry, quotefname)));
    ciapos_tuple_put(*sexp, 1, ciapos_mktuple(self->heap, 2));
    ciapos_tuple_put(ciapos_tuple_get(*sexp, 1), 1, ciapos_mknil());
    return ciapos_reader_next(self, ciapos_tuple_ref(ciapos_tuple_get(*sexp, 1), 0));
}
static ciapos_reader_error read(ciapos_reader *self, ciapos_token tok, ciapos_sexp *sexp) {
    ciapos_reader_error err = 0;
    ciapos_token tok2;
    switch (tok.tag) {
    case CIAPOS_TOKSYM:
        *sexp = ciapos_mksymbol(ciapos_symbolof(self->registry, tok.symbol));
        break;
    case CIAPOS_TOKINT:
        *sexp = ciapos_mkinteger(tok.integer);
        break;
    case CIAPOS_TOKREAL:
        *sexp = ciapos_mkreal(tok.real);
        break;
    case CIAPOS_TOKSTR:
        *sexp = ciapos_mkstring(self->heap, strlen(tok.string), tok.string);
        break;
    case CIAPOS_TOKBEG:
        if (!ciapos_tokengen_next(self->src, &tok2)) {
            err = CIAPOS_READERR_UNMATCHED_TOKBEG;
            goto error;
        }
        if ((err = read_tuple(self, tok.bracket, tok2, sexp))) goto error;
        ciapos_token_deinit(&tok2);
        break;
    case CIAPOS_TOKEND:
        err = CIAPOS_READERR_UNEXPECTED_TOKEND;
        goto error;
    case CIAPOS_TOKDOT:
        err = CIAPOS_READERR_UNEXPECTED_TOKDOT;
        goto error;
    case CIAPOS_TOKQUOT:
        if ((err = quote(self, sexp, "quote"))) goto error;
        break;
    case CIAPOS_TOKQQUOT:
        if ((err = quote(self, sexp, "quasi-quote"))) goto error;
        break;
    case CIAPOS_TOKUQUOT:
        if ((err = quote(self, sexp, "unquote"))) goto error;
        break;
    case CIAPOS_TOKSQUOT:
        if ((err = quote(self, sexp, "unquote-splicing"))) goto error;
        break;
    }
    sexp->debug_info = debuginfo(tok);
error:
    return err;
}

static ciapos_reader_error read_tuple(
    ciapos_reader *self,
    ciapos_codepoint *bracket,
    ciapos_token tok,
    ciapos_sexp *sexp)
{
    if (tok.tag == CIAPOS_TOKEND) {
        *sexp = ciapos_mknil();
        return 0;
    }
    if (tok.tag == CIAPOS_TOKDOT) return CIAPOS_READERR_UNEXPECTED_TOKDOT;

    ciapos_reader_error err = 0;
    int hasnext;
    int supressnil = 0;

    size_t cap = 2, len = 0;
    ciapos_sexp *members = calloc(cap, sizeof(ciapos_sexp));

    // read the car
    if ((err = read(self, tok, &members[len++]))) goto error1;

    while ((hasnext = ciapos_tokengen_next(self->src, &tok))) {
        if (tok.tag == CIAPOS_TOKEND) {
            if (!ciapos_are_matching_brackets(bracket, tok.bracket)) {
                err = CIAPOS_READERR_BRACKET_MISMATCH;
                goto error2;
            }
            break;
        }
        if (tok.tag == CIAPOS_TOKDOT) {
            ciapos_token_deinit(&tok);
            ciapos_tokengen_next(self->src, &tok);
            if (tok.tag == CIAPOS_TOKEND && len == 1) {
                // a tuple ending in ".)", supress terminating nil
                supressnil = 1;
                if (!ciapos_are_matching_brackets(bracket, tok.bracket)) {
                    err = CIAPOS_READERR_BRACKET_MISMATCH;
                    goto error2;
                }
                break;
            } else {
                if (len == cap) members = realloc(members, (cap *= 2) * sizeof(ciapos_sexp));
                if ((err = read(self, tok, &members[len++]))) goto error2;
                ciapos_token_deinit(&tok);
            }
        } else {
            if (len == cap) members = realloc(members, (cap *= 2) * sizeof(ciapos_sexp));
            if ((err = read_tuple(self, bracket, tok, &members[len++]))) goto error2;
            break;
        }
    }
    if (!hasnext) {
        err = CIAPOS_READERR_UNMATCHED_TOKBEG;
        goto error1;
    }

    if (!supressnil && len == 1) {
        // add terminating nil
        if (len == cap) members = realloc(members, (cap *= 2) * sizeof(ciapos_sexp));
        members[len++] = (ciapos_sexp) { .tag = CIAPOS_TAGNIL };
    }
    *sexp = ciapos_mktuple(self->heap, len);
    for (ptrdiff_t i = 0; i < len; i++) ciapos_tuple_put(*sexp, i, members[i]);
    
error2:
    ciapos_token_deinit(&tok);
error1:
    free(members);
    return err;
}
