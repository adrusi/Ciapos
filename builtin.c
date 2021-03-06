#include "builtin.h"
#include "sexp.h"
#include <assert.h>
#include <string.h>

#define METADATA(a) \
static ciapos_function a##_f = { \
    .header = { .tag = CIAPOS_TAGFN, .mark = 0, .next = NULL }, \
    .evaluator = a##_evaluator, \
    .fbody = { .tag = CIAPOS_TAGNIL, .debug_info = 0 }, \
    .env = { .tag = CIAPOS_TAGNIL, .debug_info = 0 } \
}; \
ciapos_sexp ciapos_builtin_##a = { .tag = CIAPOS_TAGFN, .debug_info = 0, .function = &a##_f };

static ciapos_sexp add_evaluator(ciapos_vm *_vm, ciapos_sexp _fbody, ciapos_sexp _env, ciapos_sexp args) {
    ciapos_integer acc = 0;
    while (args.tag != CIAPOS_TAGNIL) {
        assert(args.tag == CIAPOS_TAGTUP);
        assert(args.tuple->length == 2);
        ciapos_sexp car = ciapos_tuple_get(args, 0);
        assert(car.tag == CIAPOS_TAGINT); // TODO real support
        acc += car.integer;
        args = args.tuple->buffer[1];
    }
    return ciapos_mkinteger(acc);
}

METADATA(add);

static ciapos_sexp subtract_evaluator(ciapos_vm *_vm, ciapos_sexp _fbody, ciapos_sexp _env, ciapos_sexp args) {
    assert(args.tag == CIAPOS_TAGTUP);
    assert(args.tuple->length == 2);
    ciapos_sexp car = ciapos_tuple_get(args, 0);
    assert(car.tag == CIAPOS_TAGINT);
    ciapos_integer acc = car.integer;
    args = ciapos_tuple_get(args, 1);
    while (args.tag != CIAPOS_TAGNIL) {
        assert(args.tag == CIAPOS_TAGTUP);
        assert(args.tuple->length == 2);
        car = ciapos_tuple_get(args, 0);
        assert(car.tag == CIAPOS_TAGINT);
        acc -= car.integer;
        args = ciapos_tuple_get(args, 1);
    }
    return ciapos_mkinteger(acc);
}

METADATA(subtract);

static ciapos_sexp multiply_evaluator(ciapos_vm *_vm, ciapos_sexp _fbody, ciapos_sexp _env, ciapos_sexp args) {
    ciapos_integer acc = 1;
    while (args.tag != CIAPOS_TAGNIL) {
        assert(args.tag == CIAPOS_TAGTUP);
        assert(args.tuple->length == 2);
        ciapos_sexp car = ciapos_tuple_get(args, 0);
        assert(car.tag == CIAPOS_TAGINT); // TODO real support
        acc *= car.integer;
        args = args.tuple->buffer[1];
    }
    return ciapos_mkinteger(acc);
}

METADATA(multiply);

static ciapos_sexp divide_evaluator(ciapos_vm *_vm, ciapos_sexp _fbody, ciapos_sexp _env, ciapos_sexp args) {
    assert(args.tag == CIAPOS_TAGTUP);
    assert(args.tuple->length == 2);
    ciapos_sexp car = ciapos_tuple_get(args, 0);
    assert(car.tag == CIAPOS_TAGINT);
    ciapos_integer acc = car.integer;
    args = ciapos_tuple_get(args, 1);
    while (args.tag != CIAPOS_TAGNIL) {
        assert(args.tag == CIAPOS_TAGTUP);
        assert(args.tuple->length == 2);
        car = ciapos_tuple_get(args, 0);
        assert(car.tag == CIAPOS_TAGINT);
        acc /= car.integer;
        args = ciapos_tuple_get(args, 1);
    }
    return ciapos_mkinteger(acc);
}

METADATA(divide);

static ciapos_sexp mod_evaluator(ciapos_vm *_vm, ciapos_sexp _fbody, ciapos_sexp _env, ciapos_sexp args) {
    assert(args.tag == CIAPOS_TAGTUP);
    assert(args.tuple->length == 2);
    ciapos_sexp car = ciapos_tuple_get(args, 0);
    assert(car.tag == CIAPOS_TAGINT);
    ciapos_integer acc = car.integer;
    args = ciapos_tuple_get(args, 1);
    while (args.tag != CIAPOS_TAGNIL) {
        assert(args.tag == CIAPOS_TAGTUP);
        assert(args.tuple->length == 2);
        car = ciapos_tuple_get(args, 0);
        assert(car.tag == CIAPOS_TAGINT);
        acc %= car.integer;
    }
    return ciapos_mkinteger(acc);
}

METADATA(mod);

static ciapos_sexp typeof_evaluator(ciapos_vm *_vm, ciapos_sexp _fbody, ciapos_sexp _env, ciapos_sexp args) {
    assert(args.tag == CIAPOS_TAGTUP);
    assert(args.tuple->length == 2);
    assert(ciapos_tuple_get(args, 1).tag == CIAPOS_TAGNIL);
    return (ciapos_sexp) { .tag = CIAPOS_TAGSYM, .debug_info = 0, .symbol = ciapos_tuple_get(args, 0).tag };
}

METADATA(typeof);

static ciapos_sexp withtype_evaluator(ciapos_vm *_vm, ciapos_sexp _fbody, ciapos_sexp _env, ciapos_sexp args) {
    assert(args.tag == CIAPOS_TAGTUP);
    assert(args.tuple->length == 2);
    ciapos_sexp car = ciapos_tuple_get(args, 0);
    assert(car.tag == CIAPOS_TAGSYM);
    assert(car.symbol >= CIAPOS_TAGTUP);
    ciapos_sexp cdr = ciapos_tuple_get(args, 1);
    assert(cdr.tag == CIAPOS_TAGTUP);
    assert(cdr.tuple->length == 2);
    assert(ciapos_tuple_get(cdr, 1).tag == CIAPOS_TAGNIL);
    ciapos_sexp cdar = ciapos_tuple_get(cdr, 0);
    assert(cdar.tag >= CIAPOS_TAGTUP);
    cdar.tag = car.symbol;
    return cdar;
}

METADATA(withtype);

static ciapos_sexp tuple_evaluator(ciapos_vm *vm, ciapos_sexp _fbody, ciapos_sexp _env, ciapos_sexp args) {
    assert(args.tag == CIAPOS_TAGTUP);
    assert(args.tuple->length == 2);
    assert(ciapos_tuple_get(args, 1).tag == CIAPOS_TAGNIL);
    ciapos_sexp car = ciapos_tuple_get(args, 0);
    assert(car.tag == CIAPOS_TAGINT);
    return ciapos_mktuple(&vm->top_of_heap, car.integer); // TODO we need to somehow access the heap
}

METADATA(tuple);

static ciapos_sexp set_evaluator(ciapos_vm *_vm, ciapos_sexp _fbody, ciapos_sexp _env, ciapos_sexp args) {
    assert(args.tag == CIAPOS_TAGTUP);
    assert(args.tuple->length == 2);
    ciapos_sexp car = ciapos_tuple_get(args, 0);
    assert(car.tag >= CIAPOS_TAGTUP);
    ciapos_sexp cdr = ciapos_tuple_get(args, 1);
    assert(cdr.tag == CIAPOS_TAGTUP);
    assert(cdr.tuple->length == 2);
    ciapos_sexp cdar = ciapos_tuple_get(cdr, 0);
    assert(cdar.tag == CIAPOS_TAGINT);
    ciapos_sexp cddr = ciapos_tuple_get(cdr, 1);
    assert(cddr.tag == CIAPOS_TAGTUP);
    assert(cddr.tuple->length == 2);
    ciapos_sexp cddar = ciapos_tuple_get(cddr, 0);
    assert(ciapos_tuple_get(cddr, 1).tag == CIAPOS_TAGNIL);
    ciapos_tuple_put(car, cdar.integer, cddar);
    return (ciapos_sexp) { .tag = CIAPOS_TAGNIL, .debug_info = 0 };
}

METADATA(set);

static ciapos_sexp get_evaluator(ciapos_vm *_vm, ciapos_sexp _fbody, ciapos_sexp _env, ciapos_sexp args) {
    assert(args.tag == CIAPOS_TAGTUP);
    assert(args.tuple->length == 2);
    ciapos_sexp car = ciapos_tuple_get(args, 0);
    assert(car.tag >= CIAPOS_TAGTUP);
    ciapos_sexp cdr = ciapos_tuple_get(args, 1);
    assert(cdr.tag == CIAPOS_TAGTUP);
    assert(cdr.tuple->length == 2);
    ciapos_sexp cdar = ciapos_tuple_get(cdr, 0);
    assert(cdar.tag == CIAPOS_TAGINT);
    assert(ciapos_tuple_get(cdr, 1).tag == CIAPOS_TAGNIL);
    return ciapos_tuple_get(car, cdar.integer);
}

METADATA(get);

static ciapos_sexp alias_evaluator(ciapos_vm *vm, ciapos_sexp _fbody, ciapos_sexp _env, ciapos_sexp args) {
    assert(args.tag == CIAPOS_TAGTUP);
    assert(args.tuple->length == 2);
    ciapos_sexp car = ciapos_tuple_get(args, 0);
    assert(car.tag >= CIAPOS_TAGSYM);
    ciapos_sexp cdr = ciapos_tuple_get(args, 1);
    assert(cdr.tag == CIAPOS_TAGTUP);
    assert(cdr.tuple->length == 2);
    ciapos_sexp cdar = ciapos_tuple_get(cdr, 0);
    assert(cdar.tag == CIAPOS_TAGSYM);
    assert(ciapos_tuple_get(cdr, 1).tag == CIAPOS_TAGNIL);
    ciapos_symreg_alias(&vm->registry, car.symbol, cdar.symbol);
    return (ciapos_sexp) { .tag = CIAPOS_TAGNIL, .debug_info = 0 };
}

METADATA(alias);

static ciapos_sexp inpkg_evaluator(ciapos_vm *vm, ciapos_sexp _fbody, ciapos_sexp _env, ciapos_sexp args) {
    assert(args.tag == CIAPOS_TAGTUP);
    assert(args.tuple->length == 2);
    ciapos_sexp car = ciapos_tuple_get(args, 0);
    assert(car.tag >= CIAPOS_TAGSTR);
    ciapos_sexp cdr = ciapos_tuple_get(args, 1);
    assert(cdr.tag == CIAPOS_TAGNIL);
    char *pkgname = calloc(car.string->length + 1, 1);
    memcpy(pkgname, car.string->buffer, car.string->length);
    ciapos_symreg_setpkg(&vm->registry, pkgname);
    free(pkgname);
    return (ciapos_sexp) { .tag = CIAPOS_TAGNIL, .debug_info = 0 };
}

METADATA(inpkg);

static ciapos_sexp setexpansion_evaluator(ciapos_vm *vm, ciapos_sexp _fbody, ciapos_sexp _env, ciapos_sexp args) {
    assert(args.tag == CIAPOS_TAGTUP);
    assert(args.tuple->length == 2);
    ciapos_sexp car = ciapos_tuple_get(args, 0);
    assert(car.tag >= CIAPOS_TAGSYM);
    ciapos_sexp cdr = ciapos_tuple_get(args, 1);
    assert(cdr.tag == CIAPOS_TAGTUP);
    assert(cdr.tuple->length == 2);
    ciapos_sexp cdar = ciapos_tuple_get(cdr, 0);
    assert(cdar.tag == CIAPOS_TAGFN);
    assert(ciapos_tuple_get(cdr, 1).tag == CIAPOS_TAGNIL);
    ciapos_sym2sexp_put(&vm->macros, car.symbol, cdar);
    return (ciapos_sexp) { .tag = CIAPOS_TAGNIL, .debug_info = 0 };
}

METADATA(setexpansion);

static ciapos_sexp envset_evaluator(ciapos_vm *vm, ciapos_sexp _fbody, ciapos_sexp _env, ciapos_sexp args) {
    assert(args.tag == CIAPOS_TAGTUP);
    assert(args.tuple->length == 2);
    ciapos_sexp car = ciapos_tuple_get(args, 0);
    assert(car.tag == CIAPOS_TAGTUP);
    ciapos_sexp caar = ciapos_tuple_get(car, 0);
    assert(caar.tag == CIAPOS_TAGENV);
    ciapos_sexp cdr = ciapos_tuple_get(args, 1);
    assert(cdr.tag == CIAPOS_TAGTUP);
    assert(cdr.tuple->length == 2);
    ciapos_sexp cdar = ciapos_tuple_get(cdr, 0);
    assert(cdar.tag == CIAPOS_TAGSYM);
    ciapos_sexp cddr = ciapos_tuple_get(cdr, 1);
    assert(cddr.tag == CIAPOS_TAGTUP);
    assert(cddr.tuple->length == 2);
    ciapos_sexp cddar = ciapos_tuple_get(cddr, 0);
    assert(ciapos_tuple_get(cddr, 1).tag == CIAPOS_TAGNIL);
    ciapos_environment_put(caar, cdar.symbol, cddar);
    return (ciapos_sexp) { .tag = CIAPOS_TAGNIL, .debug_info = 0 };
}

METADATA(envset);
