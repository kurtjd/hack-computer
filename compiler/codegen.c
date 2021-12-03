#include <stdio.h>
#include <string.h>
#include "codegen.h"

#define ElemNodeData ((Element *)(node->data))
#define SymNodeData ((Symbol *)(node->data))

// Adds a symbol to the specified symbol table
static void cg_add_symbol(LinkedList *symtbl, const char *name,
                          const char *type, Kind kind, int position)
{
    Symbol symbl;
    strcpy(symbl.name, name);
    strcpy(symbl.type, type);
    symbl.kind = kind;
    symbl.position = position;

    list_add(symtbl, &symbl);
}

// Returns the next position for a specified symbol kind in the symbol table
static int cg_get_next_pos(const LinkedList *symtbl, Kind kind)
{
    int pos = 0;
    const Node *node = symtbl->start;
    while (node != NULL)
    {
        if (SymNodeData->kind == kind)
        {
            pos = SymNodeData->position + 1;
        }

        node = node->next;
    }

    return pos;
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
                          cg_get_next_pos(symtbl, kind));
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
                          ARG, cg_get_next_pos(&cg->subr_symbols, ARG));
        }

        node = node->next;
    }

    return node;
}

// Handles a subroutine declaration
static void cg_subroutine_dec(CodeGen *cg)
{
    list_free(&cg->subr_symbols);
    list_init(&cg->subr_symbols, sizeof(Symbol));
}

// Handles a class declaration
static void cg_class_dec(CodeGen *cg)
{
    list_free(&cg->cls_symbols);
    list_init(&cg->cls_symbols, sizeof(Symbol));
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
            cg_class_dec(cg);
            break;
        case SUBROUTINE_DEC:
            cg_subroutine_dec(cg);
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
               symbl->type, symbl->kind, symbl->position);
        node = node->next;
    }
}
