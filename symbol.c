#include "symbol.h"
#include <string.h>
#include <assert.h>

static void symname_deinit(char const *name) {
    if (name) free((void *) name);
}

void ciapos_symreg_init(ciapos_symreg *registry) {
    registry->symcnt = 0;
    ciapos_sym2str_init(&registry->sym2str, 256, symname_deinit);
    ciapos_str2sym_init(&registry->str2sym, 256, NULL, NULL);
    strcpy(registry->pkgname, "std");
    // reserve the correct IDs for the builtin symbols
    ciapos_symbolof(registry, "nil");
    ciapos_symbolof(registry, "symbol");
    ciapos_symbolof(registry, "integer");
    ciapos_symbolof(registry, "real");
    ciapos_symbolof(registry, "string");
    ciapos_symbolof(registry, "function");
    ciapos_symbolof(registry, "opaque");
    ciapos_symbolof(registry, "environment");
    ciapos_symbolof(registry, "tuple");
    assert(ciapos_symbolof(registry, "nil") == CIAPOS_TAGNIL);
    assert(ciapos_symbolof(registry, "symbol") == CIAPOS_TAGSYM);
    assert(ciapos_symbolof(registry, "integer") == CIAPOS_TAGINT);
    assert(ciapos_symbolof(registry, "real") == CIAPOS_TAGREAL);
    assert(ciapos_symbolof(registry, "string") == CIAPOS_TAGSTR);
    assert(ciapos_symbolof(registry, "function") == CIAPOS_TAGFN);
    assert(ciapos_symbolof(registry, "opaque") == CIAPOS_TAGOPAQUE);
    assert(ciapos_symbolof(registry, "environment") == CIAPOS_TAGENV);
    assert(ciapos_symbolof(registry, "tuple") == CIAPOS_TAGTUP);
}

void ciapos_symreg_deinit(ciapos_symreg *registry) {
    ciapos_str2sym_deinit(&registry->str2sym);
    ciapos_sym2str_deinit(&registry->sym2str);
}

static int is_qualified(char const *str) {
    while (*str) {
        if (*str == ':') return 1;
        str++;
    }
    return 0;
}

ciapos_symbol ciapos_symbolof(ciapos_symreg *registry, char const *str) {
    char *newstr;
    if (!is_qualified(str)) {
        newstr = malloc(strlen(registry->pkgname + 1 + strlen(str) + 1));
        strcpy(newstr, registry->pkgname);
        strcat(newstr, ":");
        strcat(newstr, str);
        if (ciapos_str2sym_has(&registry->str2sym, newstr)) {
            ciapos_symbol sym = ciapos_str2sym_get(&registry->str2sym, newstr);
            free(newstr);
            return sym;
        }
    } else {
        if (ciapos_str2sym_has(&registry->str2sym, str)) {
            return ciapos_str2sym_get(&registry->str2sym, str);
        }
        newstr = malloc(strlen(str) + 1);
        strcpy(newstr, str);
    }
    ciapos_symbol sym = registry->symcnt++;
    ciapos_sym2str_append(&registry->sym2str, newstr);
    ciapos_str2sym_put(&registry->str2sym, newstr, sym);
    return sym;
}

void ciapos_symreg_setpkg(ciapos_symreg *self, char const *pkgname) {
    assert(strlen(pkgname) <= 255);
    strcpy(self->pkgname, pkgname);
}

void ciapos_symreg_alias(ciapos_symreg *self, ciapos_symbol newname, ciapos_symbol oldname) {
    ciapos_str2sym_put(&self->str2sym, ciapos_sym2str_get(&self->sym2str, newname), oldname);
}
