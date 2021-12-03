#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"
#include "linkedlist.h"

typedef enum
{
    FIELD,
    STATIC,
    ARG,
    VAR
} Kind;

typedef struct Symbol
{
    char name[TOKEN_MAX_LEN];
    char type[TOKEN_MAX_LEN];
    Kind kind;
    int position;
} Symbol;

typedef struct CodeGen
{
    LinkedList cls_symbols;
    LinkedList subr_symbols;
} CodeGen;

void cg_generate(CodeGen *cg, const Parser *ps);
void cg_free(CodeGen *cg);
void cg_print_symtbl(LinkedList *symtbl);

#endif
