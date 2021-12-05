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
    int index;
} Symbol;

typedef struct FuncData
{
    char name[TOKEN_MAX_LEN];
    char kind[16];
    char type[16];
} FuncData;

typedef struct CodeGen
{
    char cur_cls[TOKEN_MAX_LEN];
    FuncData cur_func;
    LinkedList cls_symbols;
    LinkedList subr_symbols;
} CodeGen;

// Generates Hack VM code from a given list of Jack program elements
void cg_generate(CodeGen *cg, const Parser *ps);

// Frees all memory used by the code generator
void cg_free(CodeGen *cg);

// Prints the contents of a symbol table
void cg_print_symtbl(LinkedList *symtbl);

// Generates a VM file
bool cg_gen_vm_file(const char *name);

#endif
