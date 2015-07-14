#include "vm.h"
#include "symbol.h"
#include "builtin.h"
#include <assert.h>

static ciapos_sexp install_builtins(ciapos_vm *self, ciapos_sexp thread) {
    ciapos_sexp env = ciapos_mkenvironment(&self->top_of_heap, 128);
    ciapos_environment_put(env, ciapos_symbolof(&self->registry, "+"), ciapos_builtin_add);
    ciapos_environment_put(env, ciapos_symbolof(&self->registry, "-"), ciapos_builtin_subtract);
    ciapos_environment_put(env, ciapos_symbolof(&self->registry, "*"), ciapos_builtin_multiply);
    ciapos_environment_put(env, ciapos_symbolof(&self->registry, "/"), ciapos_builtin_divide);
    ciapos_environment_put(env, ciapos_symbolof(&self->registry, "%"), ciapos_builtin_mod);
    ciapos_environment_put(env, ciapos_symbolof(&self->registry, "typeof"), ciapos_builtin_typeof);
    ciapos_environment_put(env, ciapos_symbolof(&self->registry, "with-type"), ciapos_builtin_withtype);
    ciapos_environment_put(env, ciapos_symbolof(&self->registry, "tuple"), ciapos_builtin_tuple);
    ciapos_environment_put(env, ciapos_symbolof(&self->registry, "set"), ciapos_builtin_set);
    ciapos_environment_put(env, ciapos_symbolof(&self->registry, "get"), ciapos_builtin_get);
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
    return (ciapos_sexp) { .tag = CIAPOS_TAGNIL, .debug_info = 0 };
}

static ciapos_sexp eval_args(ciapos_vm *self, ciapos_sexp args) {
    if (args.tag == CIAPOS_TAGNIL) return args;
    assert(args.tag == CIAPOS_TAGTUP);
    assert(args.tuple->length == 2);
    ciapos_sexp result = ciapos_mktuple(&self->top_of_heap, 2);
    ciapos_tuple_put(result, 0, ciapos_vm_eval(self, ciapos_tuple_get(args, 0)));
    ciapos_tuple_put(result, 1, eval_args(self, ciapos_tuple_get(args, 1)));
    return result;
}

ciapos_sexp ciapos_vm_eval(ciapos_vm *self, ciapos_sexp expr) {
    switch (expr.tag) {
    case CIAPOS_TAGNIL: case CIAPOS_TAGINT: case CIAPOS_TAGREAL: case CIAPOS_TAGSTR: case CIAPOS_TAGOPAQUE:
    case CIAPOS_TAGFN: return expr;
    case CIAPOS_TAGSYM: return lookup(self->stack, expr.symbol);
    case CIAPOS_TAGTUP: default:
        assert(expr.tuple->length == 2);
        ciapos_sexp fexpr = ciapos_tuple_get(expr, 0);
        if (fexpr.tag == CIAPOS_TAGSYM) {
            if (fexpr.symbol == ciapos_symbolof(&self->registry, "quote")) {
                ciapos_sexp args = ciapos_tuple_get(expr, 1);
                assert(args.tag == CIAPOS_TAGTUP);
                assert(args.tuple->length == 2);
                assert(ciapos_tuple_get(args, 1).tag == CIAPOS_TAGNIL);
                return ciapos_tuple_get(args, 0);
            }
        }
        ciapos_sexp function = ciapos_vm_eval(self, fexpr);
        assert(function.tag == CIAPOS_TAGFN);
        return ciapos_function_eval(function, eval_args(self, ciapos_tuple_get(expr, 1)));
    }
}
