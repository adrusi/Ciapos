#include <stdio.h>
#include <stdlib.h>
#include "chargen.h"
#include "utf8.h"
#include "unicode.h"
#include "lexutil.h"
#include "token.h"
#include "sexp.h"
#include "read.h"
#include "symbol.h"
#include "vm.h"

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

static void print_sexp(ciapos_symreg *registry, ciapos_sexp sexp);

static void print_tuple(ciapos_symreg *registry, ciapos_sexp sexp) {
    for (ptrdiff_t i = 0; i < sexp.tuple->length - 1; i++) {
        print_sexp(registry, sexp.tuple->buffer[i]);
        printf(" . ");
    }
    print_sexp(registry, sexp.tuple->buffer[sexp.tuple->length - 1]);
    printf(")");
}
static void print_sexp(ciapos_symreg *registry, ciapos_sexp sexp) {
    switch (sexp.tag) {
    case CIAPOS_TAGNIL:
        printf("()");
        break;
    case CIAPOS_TAGSYM:
        printf("%s", ciapos_sym2str_get(&registry->sym2str, sexp.symbol));
        break;
    case CIAPOS_TAGINT:
        printf("%ld", sexp.integer);
        break;
    case CIAPOS_TAGREAL:
        printf("%f", sexp.real);
        break;
    case CIAPOS_TAGSTR:
        printf("\"");
        for (ptrdiff_t i = 0; i < sexp.string->length; i++) {
            printf("%c", sexp.string->buffer[i]);
        }
        printf("\"");
        break;
    case CIAPOS_TAGFN:
        printf("<FUNCTION>");
        break;
    case CIAPOS_TAGOPAQUE:
        printf("<OPAQUE>");
        break;
    case CIAPOS_TAGENV:
        printf("<ENVIRONMENT>");
        break;
    default:
        printf("#%s", ciapos_sym2str_get(&registry->sym2str, sexp.tag));
        // fallthrough
    case CIAPOS_TAGTUP:
        printf("(");
        print_tuple(registry, sexp);
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

    ciapos_vm vm;
    ciapos_vm_init(&vm);

    ciapos_reader reader;
    ciapos_reader_init(&reader, &lexer, &vm.registry, &vm.top_of_heap);
    
    ciapos_sexp sexp;
    ciapos_reader_error err;
    while (!(err = ciapos_reader_next(&reader, &sexp))) {
        print_sexp(&vm.registry, ciapos_vm_eval(&vm, sexp));
        printf("\n");
    }
    if (err) printf("ERROR %d\n", err);

    ciapos_reader_deinit(&reader);
    ciapos_vm_deinit(&vm);
    ciapos_tokengen_deinit(&lexer);
    ciapos_graphemegen_deinit(&unicode);
    ciapos_utf8gen_deinit(&utf8gen);
    ciapos_chargen_deinit(&chargen);

    return 0;
}

