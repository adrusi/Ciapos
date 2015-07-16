#include "vm.h"
#include "symbol.h"
#include "builtin.h"
#include <assert.h>
#include <stdio.h>

static ciapos_sexp install_builtins(ciapos_vm *self, ciapos_sexp thread) {
    ciapos_sexp env = ciapos_mkenvironment(&self->top_of_heap, 128);
    ciapos_environment_put(env, ciapos_symbolof(&self->registry, "+"), ciapos_builtin_add);
    ciapos_environment_put(env, ciapos_symbolof(&self->registry, "-"), ciapos_builtin_subtract);
    ciapos_environment_put(env, ciapos_symbolof(&self->registry, "*"), ciapos_builtin_multiply);
    ciapos_environment_put(env, ciapos_symbolof(&self->registry, "/"), ciapos_builtin_divide);
    ciapos_environment_put(env, ciapos_symbolof(&self->registry, "%"), ciapos_builtin_mod);
    ciapos_environment_put(env, ciapos_symbolof(&self->registry, "alias"), ciapos_builtin_alias);
    ciapos_environment_put(env, ciapos_symbolof(&self->registry, "in-pkg"), ciapos_builtin_inpkg);
    ciapos_environment_put(env, ciapos_symbolof(&self->registry, "typeof"), ciapos_builtin_typeof);
    ciapos_environment_put(env, ciapos_symbolof(&self->registry, "with-type"), ciapos_builtin_withtype);
    ciapos_environment_put(env, ciapos_symbolof(&self->registry, "tuple"), ciapos_builtin_tuple);
    ciapos_environment_put(env, ciapos_symbolof(&self->registry, "tuple-set!"), ciapos_builtin_set);
    ciapos_environment_put(env, ciapos_symbolof(&self->registry, "tuple-get"), ciapos_builtin_get);
    ciapos_sexp newthread = ciapos_mktuple(&self->top_of_heap, 2);
    ciapos_tuple_put(newthread, 0, env);
    ciapos_tuple_put(newthread, 1, thread);
    return newthread;
}

void ciapos_vm_init(ciapos_vm *self) {
    ciapos_symreg_init(&self->registry);
    self->top_of_heap = NULL;
    self->stack = (ciapos_sexp) { .tag = CIAPOS_TAGNIL, .debug_info = 0 };
    self->stack = install_builtins(self, self->stack);
}

void ciapos_vm_deinit(ciapos_vm *self) {
    ciapos_symreg_deinit(&self->registry);
    while (self->top_of_heap) {
        ciapos_gc_header *tmp = self->top_of_heap;
        self->top_of_heap = self->top_of_heap->next;
        ciapos_gc_deinit(tmp);
    }
}

static ciapos_sexp lookup(ciapos_sexp stack, ciapos_symbol id) {
    while (stack.tag != CIAPOS_TAGNIL) {
        assert(stack.tag == CIAPOS_TAGTUP);
        assert(stack.tuple->length == 2);
        ciapos_sexp env = ciapos_tuple_get(stack, 0);
        if (ciapos_environment_has(env, id)) {
            return ciapos_environment_get(env, id);
        }
        stack = ciapos_tuple_get(stack, 1);
    }
    assert(!"undefined variable");
}

static ciapos_sexp ciapos_vm_eval_withstack(ciapos_vm *self, ciapos_sexp stack, ciapos_sexp expr);

static ciapos_sexp eval_args(ciapos_vm *self, ciapos_sexp stack, ciapos_sexp args) {
    if (args.tag == CIAPOS_TAGNIL) return args;
    assert(args.tag == CIAPOS_TAGTUP);
    assert(args.tuple->length == 2);
    ciapos_sexp result = ciapos_mktuple(&self->top_of_heap, 2);
    ciapos_tuple_put(result, 0, ciapos_vm_eval_withstack(self, stack, ciapos_tuple_get(args, 0)));
    ciapos_tuple_put(result, 1, eval_args(self, stack, ciapos_tuple_get(args, 1)));
    return result;
}

static void extract_args(ciapos_sexp frame, ciapos_sexp arglist, ciapos_sexp args) {
    if (arglist.tag == CIAPOS_TAGNIL || args.tag == CIAPOS_TAGNIL) {
        assert(args.tag == arglist.tag);
        return;
    }
    if (arglist.tag == CIAPOS_TAGSYM) ciapos_environment_put(frame, arglist.symbol, args);
    assert(args.tag == CIAPOS_TAGTUP);
    assert(arglist.tag == CIAPOS_TAGTUP);
    ciapos_sexp argname = ciapos_tuple_get(arglist, 0);
    ciapos_sexp argval  = ciapos_tuple_get(args, 0);
    assert(argname.tag == CIAPOS_TAGSYM);
    ciapos_environment_put(frame, argname.symbol, argval);
    extract_args(frame, ciapos_tuple_get(arglist, 1), ciapos_tuple_get(args, 1));
}

static ciapos_sexp ciapos_vm_eval_withstack(ciapos_vm *self, ciapos_sexp stack, ciapos_sexp expr);

ciapos_sexp ciapos_vm_eval(ciapos_vm *self, ciapos_sexp expr) {
    return ciapos_vm_eval_withstack(self, self->stack, expr);
}

static ciapos_sexp usercode_eval(ciapos_vm *vm, ciapos_sexp fbody, ciapos_sexp env, ciapos_sexp args) {
    ciapos_sexp arglist = ciapos_tuple_get(fbody, 0);
    ciapos_sexp frame = ciapos_mkenvironment(&vm->top_of_heap, 16);
    extract_args(frame, arglist, args);

    ciapos_sexp newenv = ciapos_mktuple(&vm->top_of_heap, 2);
    ciapos_tuple_put(newenv, 0, frame);
    ciapos_tuple_put(newenv, 1, env);
    env = newenv;

    ciapos_sexp result;
    fbody = ciapos_tuple_get(fbody, 1);
    assert(fbody.tag == CIAPOS_TAGTUP);
    do {
        result = ciapos_vm_eval_withstack(vm, env, ciapos_tuple_get(fbody, 0));
        fbody = ciapos_tuple_get(fbody, 1);
    } while (fbody.tag == CIAPOS_TAGTUP);
    assert(fbody.tag == CIAPOS_TAGNIL);

    return result;
}

static ciapos_sexp ciapos_vm_eval_withstack(ciapos_vm *self, ciapos_sexp stack, ciapos_sexp expr) {
    switch (expr.tag) {
    case CIAPOS_TAGNIL: case CIAPOS_TAGINT: case CIAPOS_TAGREAL: case CIAPOS_TAGSTR: case CIAPOS_TAGOPAQUE:
    case CIAPOS_TAGFN: return expr;
    case CIAPOS_TAGSYM: return lookup(stack, expr.symbol);
    case CIAPOS_TAGTUP: default:
        assert(expr.tuple->length == 2);
        ciapos_sexp fexpr = ciapos_tuple_get(expr, 0);
        if (fexpr.tag == CIAPOS_TAGSYM) {
            if (fexpr.symbol == ciapos_symbolof(&self->registry, "std:quote")) {
                ciapos_sexp args = ciapos_tuple_get(expr, 1);
                assert(args.tag == CIAPOS_TAGTUP);
                assert(args.tuple->length == 2);
                assert(ciapos_tuple_get(args, 1).tag == CIAPOS_TAGNIL);
                return ciapos_tuple_get(args, 0);
            }
            if (fexpr.symbol == ciapos_symbolof(&self->registry, "std:lambda")) {
                ciapos_sexp args = ciapos_tuple_get(expr, 1);
                assert(args.tag == CIAPOS_TAGTUP);
                assert(args.tuple->length == 2);
                ciapos_sexp paramlist = ciapos_tuple_get(args, 0);
                for (ciapos_sexp a = paramlist;
                     a.tag != CIAPOS_TAGNIL;
                     a = ciapos_tuple_get(a, 1))
                {
                    assert(a.tag == CIAPOS_TAGTUP);
                    assert(a.tuple->length == 2);
                    assert(ciapos_tuple_get(a, 0).tag == CIAPOS_TAGSYM);
                }
                assert(ciapos_tuple_get(args, 1).tag != CIAPOS_TAGNIL);
                return ciapos_mkfunction(&self->top_of_heap, usercode_eval, args, stack);
            }
        }
        ciapos_sexp function = ciapos_vm_eval_withstack(self, stack, fexpr);
        assert(function.tag == CIAPOS_TAGFN);
        return ciapos_function_eval(function, self, eval_args(self, stack, ciapos_tuple_get(expr, 1)));
    }
}
