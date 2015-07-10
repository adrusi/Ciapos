#include "token.h"
#include "generator.h"
#include "unicode.h"

typedef int (*token_reader)(ciapos_graphemerewinder *, ciapos_token *);
static int is_splicing_unquote(ciapos_graphemerewinder *src, ciapos_token *tok);
static int is_unquote(ciapos_graphemerewinder *src, ciapos_token *tok);
static int is_quasiquote(ciapos_graphemerewinder *src, ciapos_token *tok);
static int is_dot(ciapos_graphemerewinder *src, ciapos_token *tok);
static int is_quote(ciapos_graphemerewinder *src, ciapos_token *tok);
static int is_begin(ciapos_graphemerewinder *src, ciapos_token *tok);
static int is_end(ciapos_graphemerewinder *src, ciapos_token *tok);
static int is_string(ciapos_graphemerewinder *src, ciapos_token *tok);
static int is_real(ciapos_graphemerewinder *src, ciapos_token *tok);
static int is_integer(ciapos_graphemerewinder *src, ciapos_token *tok);
static int is_symbol(ciapos_graphemerewinder *src, ciapos_token *tok);

void ciapos_tokengen_init(ciapos_tokengen *self, ciapos_graphemegen *src, int fileid) {
    self->state = 0;
    self->fileid = fileid;
    self->line = self->col = 1;
    ciapos_graphemerewinder_init(&self->src, src);
}

int ciapos_tokengen_next(ciapos_tokengen *self, ciapos_token *token) {
    static token_reader token_readers[] = {
        is_splicing_unquote,
        is_unquote,
        is_quasiquote,
        is_quote,
        is_begin,
        is_end,
        is_string,
        is_real,
        is_dot,
        is_integer,
        is_symbol
    };
    begin_generator(self->state);

    while (ciapos_graphemerewinder_hasnext(&self->src)) {
        ciapos_graphemerewinder_flush(&self->src);

        ciapos_codepoint *a = ciapos_graphemerewinder_next(&self->src);
        if (!a) {
            yield (0);
            return 0;
        }
        if (ciapos_is_whitespace(a)) continue;
        else ciapos_graphemerewinder_rewind(&self->src);

        for (self->readerid = 0; self->readerid < sizeof(token_readers); self->readerid++) {
            if (token_readers[self->readerid](&self->src, token)) {
                yield (1);
                break;
            }
            ciapos_graphemerewinder_rewind(&self->src);
        }
    }
    yield (0);

    end_generator;
    return 0;
}

void ciapos_tokengen_deinit(ciapos_tokengen *self) {
    ciapos_graphemerewinder_deinit(&self->src);
}

void ciapos_token_deinit(ciapos_token *token) {

}

static int is_splicing_unquote(ciapos_graphemerewinder *src, ciapos_token *tok) {
    ciapos_codepoint *a = ciapos_graphemerewinder_next(src);
    ciapos_codepoint *b = ciapos_graphemerewinder_next(src);
    if (!a || !b) return 0;
    if (*a != ',' || *b != '@') return 0;
    tok->tag = CIAPOS_TOKSQUOT;
    return 1;
}

static int is_unquote(ciapos_graphemerewinder *src, ciapos_token *tok) {
    ciapos_codepoint *a = ciapos_graphemerewinder_next(src);
    if (!a) return 0;
    if (*a != ',') return 0;
    tok->tag = CIAPOS_TOKUQUOT;
    return 1;
}

static int is_quasiquote(ciapos_graphemerewinder *src, ciapos_token *tok) {
    ciapos_codepoint *a = ciapos_graphemerewinder_next(src);
    if (!a) return 0;
    if (*a != '`') return 0;
    tok->tag = CIAPOS_TOKQQUOT;
    return 1;
}

static int is_dot(ciapos_graphemerewinder *src, ciapos_token *tok) {
    ciapos_codepoint *a = ciapos_graphemerewinder_next(src);
    if (!a) return 0;
    if (*a != '.') return 0;
    tok->tag = CIAPOS_TOKDOT;
    return 1;
}

static int is_quote(ciapos_graphemerewinder *src, ciapos_token *tok) {
    ciapos_codepoint *a = ciapos_graphemerewinder_next(src);
    if (!a) return 0;
    if (*a != '\'') return 0;
    tok->tag = CIAPOS_TOKQUOT;
    return 1;
}

static int is_begin(ciapos_graphemerewinder *src, ciapos_token *tok) {
    ciapos_codepoint *a = ciapos_graphemerewinder_next(src);
    if (!a) return 0;
    if (!ciapos_is_opening_bracket(a)) return 0;
    ciapos_codepoint *clone = calloc(ciapos_grapheme_len(a) + 1, sizeof(ciapos_codepoint));
    for (int i = 0; a[i]; i++) clone[i] = a[i];
    tok->tag = CIAPOS_TOKBEG;
    tok->bracket = clone;
    return 1;
}

static int is_end(ciapos_graphemerewinder *src, ciapos_token *tok) {
    ciapos_codepoint *a = ciapos_graphemerewinder_next(src);
    if (!a) return 0;
    if (!ciapos_is_closing_bracket(a)) return 0;
    ciapos_codepoint *clone = calloc(ciapos_grapheme_len(a) + 1, sizeof(ciapos_codepoint));
    for (int i = 0; a[i]; i++) clone[i] = a[i];
    tok->tag = CIAPOS_TOKEND;
    tok->bracket = clone;
    return 1;
}

static int is_string(ciapos_graphemerewinder *src, ciapos_token *tok) {
    ciapos_codepoint *a = ciapos_graphemerewinder_next(src);
    if (!a) return 0;
    if (!ciapos_is_opening_quote(a)) return 0;
    size_t cap = 16, len = 0;
    char *str = malloc(cap);
    ciapos_codepoint *b;
    for (b = ciapos_graphemerewinder_next(src);
         b && !ciapos_are_matching_quotes(a, b);
         b = ciapos_graphemerewinder_next(src))
    {
        // TODO use a proper utf8 encoder here
        if (len == cap) str = realloc(str, (cap *= 2));
        if (*b == '\\') {
            b = ciapos_graphemerewinder_next(src);
            if (!b) break;
            switch (*b) {
                case '\\': str[len++] = '\\'; break;
                case '"': str[len++] = '"'; break;
                case 'n': str[len++] = '\n'; break;
                case 'r': str[len++] = '\r'; break;
                case 't': str[len++] = '\t'; break;
                case 'v': str[len++] = '\v'; break;
                case 'f': str[len++] = '\f'; break;
                case 'a': str[len++] = '\a'; break;
                case 'b': str[len++] = '\b'; break;
                case '0': str[len++] = '\0'; break;
                default:
                    free(str);
                    return 0;
            }
        } else {
            str[len++] = (char) *b;
        }
    }
    if (!b) {
        free(str);
        return 0;
    }
    if (len == cap) str = realloc(str, (cap *= 2));
    str[len] = '\0';
    tok->tag = CIAPOS_TOKSTR;
    tok->string = str;
    return 1;
}

static int is_real(ciapos_graphemerewinder *src, ciapos_token *tok) {
    ciapos_codepoint *a = ciapos_graphemerewinder_next(src);
    if (!a) return 0;
    if (!ciapos_is_numeric(a) && !ciapos_is_sign(a) && *a != '.') return 0;
    size_t cap = 16, len = 0;
    char *str = malloc(cap);
    // TODO proper utf8 encoding
    str[len++] = (char) *a;
    if (*a == '.') goto fractional_part;
    for (a = ciapos_graphemerewinder_next(src);
         a && ciapos_is_numeric(a);
         a = ciapos_graphemerewinder_next(src))
    {
        if (len == cap) str = realloc(str, (cap *= 2));
        str[len++] = (char) *a;
    }
    if (!a || *a != '.') {
        free(str);
        return 0;
    }
    if (len == cap) str = realloc(str, (cap *= 2));
    str[len++] = '.';
fractional_part:
    for (a = ciapos_graphemerewinder_next(src);
         a && ciapos_is_numeric(a);
         a = ciapos_graphemerewinder_next(src))
    {
        if (len == cap) str = realloc(str, (cap *= 2));
        str[len++] = (char) *a;
    }
    // Handle the case where only a decimal point or sign is presnet, with no integer and no fraction parts.
    // In this case it's not a real number literal.
    // TODO this does not handle signs the same way as the are read, will fail with non-ascii sign designators.
    if (len == 1 && (*str == '.' || *str == '+' || *str == '-')) {
        free(str);
        return 0;
    }
    ciapos_graphemerewinder_rewindby(src, 1);
    if (len == cap) str = realloc(str, (cap *= 2));
    str[len] = '\0';
    tok->tag = CIAPOS_TOKREAL;
    tok->real = atof(str);
    free(str);
    return 1;
}

static int is_integer(ciapos_graphemerewinder *src, ciapos_token *tok) {
    ciapos_codepoint *a = ciapos_graphemerewinder_next(src);
    if (!a) return 0;
    if (!ciapos_is_numeric(a) && !ciapos_is_sign(a)) return 0;
    size_t cap = 16, len = 0;
    char *str = malloc(cap);
    // TODO proper utf8 encoding
    str[len++] = (char) *a;
    for (a = ciapos_graphemerewinder_next(src);
         a && ciapos_is_numeric(a);
         a = ciapos_graphemerewinder_next(src))
    {
        if (len == cap) str = realloc(str, (cap *= 2));
        str[len++] = (char) *a;
    }
    // Handle the case where only a sign is presnet, with no integer and no fraction parts.
    // In this case it's not a real number literal.
    // TODO this does not handle signs the same way as the are read, will fail with non-ascii sign designators.
    if (len == 1 && (*str == '+' || *str == '-')) {
        free(str);
        return 0;
    }
    ciapos_graphemerewinder_rewindby(src, 1);
    if (len == cap) str = realloc(str, (cap *= 2));
    str[len] = '\0';
    tok->tag = CIAPOS_TOKINT;
    tok->integer = atol(str);
    free(str);
    return 1;
}

static int is_symbol(ciapos_graphemerewinder *src, ciapos_token *tok) {
    size_t cap = 8, len = 0;
    char *str = malloc(cap);
    for (ciapos_codepoint *a = ciapos_graphemerewinder_next(src);
         a && !ciapos_is_closing_bracket(a) && !ciapos_is_whitespace(a);
         a = ciapos_graphemerewinder_next(src))
    {
        if (len == cap) str = realloc(str, (cap *= 2));
        // TODO proper utf8 encoding
        str[len++] = (char) *a;
    }
    if (len == 0) {
        free(str);
        return 0;
    }
    ciapos_graphemerewinder_rewindby(src, 1);
    if (len == cap) str = realloc(str, (cap *= 2));
    str[len] = '\0';
    tok->tag = CIAPOS_TOKSYM;
    tok->symbol = str;
    return 1;
}
