#ifndef CIAPOS_VM_H
#define CIAPOS_VM_H

#include "sexp.h"
#include "symbol.h"
#include "dict.h"

typedef struct ciapos_vm {
    ciapos_symreg registry;
    ciapos_gc_header *top_of_heap;
    ciapos_sexp stack;
    ciapos_sym2sexp macros;
} ciapos_vm;

void ciapos_vm_init(ciapos_vm *self);
void ciapos_vm_deinit(ciapos_vm *self);

ciapos_sexp ciapos_vm_eval(ciapos_vm *self, ciapos_sexp expression);

#endif
