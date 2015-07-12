#ifndef CIAPOS_TOKEN_H
#define CIAPOS_TOKEN_H

#include <stdint.h>
#include "unicode.h"
#include "lexutil.h"

typedef struct ciapos_token {
    enum {
        CIAPOS_TOKSYM, // ex: foo
        CIAPOS_TOKINT, // ex: 54
        CIAPOS_TOKREAL, // ex: 3.14
        CIAPOS_TOKSTR, // ex: "foo bar"
        CIAPOS_TOKBEG, // ex: (
        CIAPOS_TOKEND, // ex: )
        CIAPOS_TOKDOT, // .
        CIAPOS_TOKQUOT, // '
        CIAPOS_TOKQQUOT, // `
        CIAPOS_TOKUQUOT, // ,
        CIAPOS_TOKSQUOT // ,@
    } tag;
    int line;
    int col;
    int fileid;
    union {
        char *symbol;
        int64_t integer;
        double real;
        char *string;
        ciapos_codepoint *bracket;
    };
} ciapos_token;

typedef struct ciapos_tokengen {
    int state;
    int fileid;
    int line;
    int col;
    int readerid;
    ciapos_codepoint *probe;
    ciapos_graphemerewinder src;
} ciapos_tokengen;

void ciapos_tokengen_init(
    ciapos_tokengen *lexer,
    ciapos_graphemegen *src,
    int fileid);
void ciapos_tokengen_deinit(ciapos_tokengen *lexer);

int ciapos_tokengen_next(ciapos_tokengen *lexer, ciapos_token *token);

void ciapos_token_deinit(ciapos_token *token);

#endif
