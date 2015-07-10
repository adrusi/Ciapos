#include "unicode.h"

size_t ciapos_grapheme_len(ciapos_codepoint *grapheme) {
   size_t len = 0;
   while (*(grapheme++)) len++;
   return len;
}

int ciapos_is_whitespace(ciapos_codepoint *grapheme) {
    // TODO this is only implemented over ASCII graphemes
    switch (*grapheme) {
    case ' ': case '\t': case '\n': case '\r':
        return 1;
    }
    return 0;
}

int ciapos_is_newline(ciapos_codepoint *grapheme) {
    // TODO this is only implemented over ASCII graphemes
    size_t len = ciapos_grapheme_len(grapheme);
    if (len == 2 && grapheme[0] == '\r' && grapheme[1] == '\n') return 1;
    if (len == 1 && (*grapheme == '\r' || *grapheme == '\n')) return 1;
    return 0;
}
    

int ciapos_is_opening_bracket(ciapos_codepoint *grapheme) {
    // TODO this is only implemented over ASCII graphemes
    switch (*grapheme) {
    case '(': case '[': case '{':
        return 1;
    }
    return 0;
}

int ciapos_is_closing_bracket(ciapos_codepoint *grapheme) {
    // TODO this is only implemented over ASCII graphemes
    switch (*grapheme) {
    case ')': case ']': case '}':
        return 1;
    }
    return 0;
}

int ciapos_are_matching_brackets(ciapos_codepoint *open, ciapos_codepoint *close) {
    // TODO this is only implemented of ASCII graphemes
    if (*open == '(' && *close == ')') return 1;
    if (*open == '[' && *close == ']') return 1;
    if (*open == '{' && *close == '}') return 1;
    return 0;
}

int ciapos_is_opening_quote(ciapos_codepoint *grapheme) { return *grapheme == '"'; }
int ciapos_is_closing_quote(ciapos_codepoint *grapheme) { return *grapheme == '"'; }
int ciapos_are_matching_quotes(ciapos_codepoint *open, ciapos_codepoint *close) {
    return (*open == *close) && *open == '"';
}
