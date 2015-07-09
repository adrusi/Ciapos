#include "chargen.h"
#include <stdlib.h>
#include "generator.h"

typedef struct string_chargen {
    int state;
    int cursor;
    char const *str;
} string_chargen;

static int string_chargen_next(string_chargen *self, char *out) {
    begin_generator (self->state);

    while ((*out = self->str[self->cursor])) {
        yield (1);
        self->cursor++;
    }
    yield (0);

    end_generator;
    return 0;
}

void ciapos_string_chargen_init(ciapos_chargen *chargen, char const *str) {
    chargen->next = (int (*)(void *, char *)) string_chargen_next;
    string_chargen *ctx = malloc(sizeof(string_chargen));
    *ctx = (string_chargen) {
        .state = 0,
        .cursor = 0,
        .str = str
    };
    chargen->ctx = ctx;
}

typedef struct file_chargen {
    int state;
    FILE *f;
} file_chargen;

static int file_chargen_next(file_chargen *self, char *out) {
    begin_generator (self->state);

    while (!feof(self->f)) {
        *out = fgetc(self->f);
        yield (1);
    }
    yield (0);

    end_generator;
    return 0;
}

void ciapos_file_chargen_init(ciapos_chargen *chargen, FILE *f) {
    chargen->next = (int (*)(void *, char *)) file_chargen_next;
    file_chargen *ctx = malloc(sizeof(file_chargen));
    *ctx = (file_chargen) {
        .state = 0,
        .f = f
    };
    chargen->ctx = ctx;
}

void ciapos_chargen_deinit(ciapos_chargen *chargen) {
    free(chargen->ctx);
}
