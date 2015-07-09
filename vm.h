#ifndef CIAPOS_VM_H
#define CIAPOS_VM_H

#include "sexp.h"
#include "symbol.h"

typedef struct ciapos_vm {
    ciapos_symreg symbol_registry;
    ciapos_gc_header *heap;
    ciapos_sexp stack;
} ciapos_vm;

#endif
