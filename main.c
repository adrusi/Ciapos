#include <stdio.h>
#include <stdlib.h>
#include "chargen.h"
#include "utf8.h"

int main(int argc, char const *argv[argc]) {
    ciapos_chargen chargen;
    ciapos_file_chargen_init(&chargen, stdin);
    ciapos_utf8gen utf8gen;
    ciapos_utf8gen_init(&utf8gen, &chargen);
    ciapos_codepoint rune;
    while (ciapos_utf8gen_next(&utf8gen, &rune)) {
        printf("U+%04X\n", rune);
    }
    ciapos_utf8gen_deinit(&utf8gen);
    ciapos_chargen_deinit(&chargen);

    return 0;
}

