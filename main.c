#include <stdio.h>
#include <stdlib.h>
#include "chargen.h"
#include "utf8.h"
#include "unicode.h"
#include "lexutil.h"
#include "token.h"

static void print_token(ciapos_token tok) {
    printf("<stdin>:%d:%d ", tok.line, tok.col);
    switch (tok.tag) {
    case CIAPOS_TOKSYM:
        printf("sym(%s)\n", tok.symbol);
        break;
    case CIAPOS_TOKINT:
        printf("int(%ld)\n", tok.integer);
        break;
    case CIAPOS_TOKREAL:
        printf("real(%f)\n", tok.real);
        break;
    case CIAPOS_TOKSTR:
        printf("str(%s)\n", tok.string);
        break;
    case CIAPOS_TOKBEG:
        printf("beg\n");
        break;
    case CIAPOS_TOKEND:
        printf("end\n");
        break;
    case CIAPOS_TOKDOT:
        printf("dot\n");
        break;
    case CIAPOS_TOKQUOT:
        printf("quot\n");
        break;
    case CIAPOS_TOKQQUOT:
        printf("qquot\n");
        break;
    case CIAPOS_TOKUQUOT:
        printf("uquot\n");
        break;
    case CIAPOS_TOKSQUOT:
        printf("squot\n");
        break;
    }
}

int main(int argc, char const *argv[argc]) {
    ciapos_chargen chargen;
    ciapos_file_chargen_init(&chargen, stdin);

    ciapos_utf8gen utf8gen;
    ciapos_utf8gen_init(&utf8gen, &chargen);

    ciapos_graphemegen unicode;
    ciapos_graphemegen_init(&unicode, &utf8gen);

    ciapos_tokengen lexer;
    ciapos_tokengen_init(&lexer, &unicode, 0);
    
    ciapos_token tok;
    while (ciapos_tokengen_next(&lexer, &tok)) {
        print_token(tok);
        ciapos_token_deinit(&tok);
    }

    ciapos_tokengen_deinit(&lexer);
    ciapos_graphemegen_deinit(&unicode);
    ciapos_utf8gen_deinit(&utf8gen);
    ciapos_chargen_deinit(&chargen);

    return 0;
}

