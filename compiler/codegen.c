#include <stdio.h>
#include <string.h>
#include "codegen.h"

#define ElemNodeData ((Element *)(node->data))
#define SymNodeData ((Symbol *)(node->data))

// Adds a symbol to the specified symbol table
static void cg_add_symbol(LinkedList *symtbl, const char *name,
                          const char *type, Kind kind, int index)
{
    Symbol symbl;
    strcpy(symbl.name, name);
    strcpy(symbl.type, type);
    symbl.kind = kind;
    symbl.index = index;

    list_add(symtbl, &symbl);
}

// Looks for a symbol first in the subroutine table and then in the class table
static const Symbol *cg_get_symbol(CodeGen *cg, const char *name)
{
    bool local = true;
    Node *node = cg->subr_symbols.start;

    // Find a symbol with a matching name
    while (node != NULL)
    {
        if (strcmp(SymNodeData->name, name) == 0)
        {
            return SymNodeData;
        }

        // If no match in subroutine table, switch to class table
        node = node->next;
        if (node == NULL && local)
        {
            node = cg->cls_symbols.start;
            local = false;
        }
    }

    return NULL;
}

// Returns the next index for a specified symbol kind in the symbol table
static int cg_get_next_index(const LinkedList *symtbl, Kind kind)
{
    int index = 0;
    const Node *node = symtbl->start;
    while (node != NULL)
    {
        if (SymNodeData->kind == kind)
        {
            index = SymNodeData->index + 1;
        }

        node = node->next;
    }

    return index;
}

// Handles a class or subroutine variable declaration
static const Node *cg_var_dec(LinkedList *symtbl, const Node *node)
{
    char type[TOKEN_MAX_LEN];
    Kind kind;

    // Determine the 'kind' of variable declaration
    const char *str_kind = ElemNodeData->token->value;
    if (strcmp(str_kind, "static") == 0)
    {
        kind = STATIC;
    }
    else if (strcmp(str_kind, "field") == 0)
    {
        kind = FIELD;
    }
    else
    {
        kind = VAR;
    }
    node = node->next;

    // Get the 'type' of variable
    strcpy(type, ElemNodeData->token->value);
    node = node->next;

    /* Now add any variable name (which can be in a comma-separated list) to
     * the symbol table until end of declaration.
     */
    while (ElemNodeData->type != CLASS_VAR_DEC_END &&
           ElemNodeData->type != VAR_DEC_END)
    {
        if (ElemNodeData->token->type == IDENTIFIER)
        {
            cg_add_symbol(symtbl, ElemNodeData->token->value, type, kind,
                          cg_get_next_index(symtbl, kind));
        }

        node = node->next;
    }

    return node;
}

// Handles argument variables declared in a parameter list
static const Node *cg_params(CodeGen *cg, const Node *node)
{
    char type[TOKEN_MAX_LEN];

    /* Add any parameter (which can be in a comma-separated list) to
     * the symbol table until end of parameter list.
     */
    while (ElemNodeData->type != PARAM_LIST_END)
    {
        // Get the 'type' of variable
        if (ElemNodeData->token->type == KEYWORD)
        {
            strcpy(type, ElemNodeData->token->value);
        }
        else if (ElemNodeData->token->type == IDENTIFIER)
        {
            cg_add_symbol(&cg->subr_symbols, ElemNodeData->token->value, type,
                          ARG, cg_get_next_index(&cg->subr_symbols, ARG));
        }

        node = node->next;
    }

    return node;
}

// Handles a subroutine declaration
static void cg_subroutine_dec(CodeGen *cg, const Node *node)
{
    list_free(&cg->subr_symbols);
    list_init(&cg->subr_symbols, sizeof(Symbol));

    // Automatically add 'this' variable to symbol table if method
    if (strcmp(ElemNodeData->token->value, "method") == 0)
    {
        cg_add_symbol(&cg->subr_symbols, "this", cg->cur_cls, ARG, 0);
    }
}

// Handles a class declaration
static void cg_class_dec(CodeGen *cg, const Node *node)
{
    list_free(&cg->cls_symbols);
    list_init(&cg->cls_symbols, sizeof(Symbol));

    strcpy(cg->cur_cls, ElemNodeData->token->value);
}

// Handles an expression
static const Node *cg_expression(CodeGen *cg, const Node *node)
{
    node = cg_term(cg, node);
    return node;
}

void cg_generate(CodeGen *cg, const Parser *ps)
{
    list_init(&cg->cls_symbols, sizeof(Symbol));
    list_init(&cg->subr_symbols, sizeof(Symbol));

    const Node *node = ps->elements.start;
    while (node != NULL)
    {
        Element *elem = node->data;

        switch (elem->type)
        {
        case CLASS:
            cg_class_dec(cg, node->next);
            break;
        case SUBROUTINE_DEC:
            cg_subroutine_dec(cg, node->next->next);
            break;
        case CLASS_VAR_DEC:
            node = cg_var_dec(&cg->cls_symbols, node->next);
            break;
        case VAR_DEC:
            node = cg_var_dec(&cg->subr_symbols, node->next);
            break;
        case PARAM_LIST:
            node = cg_params(cg, node->next);
            break;
        default:
            break;
        }

        node = node->next;
    }
}

void cg_free(CodeGen *cg)
{
    list_free(&cg->cls_symbols);
    list_free(&cg->subr_symbols);
}

void cg_print_symtbl(LinkedList *symtbl)
{
    const Node *node = symtbl->start;

    while (node != NULL)
    {
        Symbol *symbl = node->data;
        printf("Name: %s\nType: %s\nKind: %d\nPos: %d\n", symbl->name,
               symbl->type, symbl->kind, symbl->index);
        node = node->next;
    }
}
