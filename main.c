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
        ciapos_sexp_debug(&vm.registry, ciapos_vm_eval(&vm, sexp));
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

