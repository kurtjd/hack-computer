#include <stdio.h>
#include <string.h>
#include "codegen.h"

#define ElemNodeData ((Element *)(node->data))

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

static const Node *cg_class_var_dec(CodeGen *cg, const Node *node)
{
    char name[TOKEN_MAX_LEN];
    char type[TOKEN_MAX_LEN];
    Kind kind;
    int position = 0;

    node = node->next;
    const char *str_kind = ElemNodeData->token->value;
    if (strcmp(str_kind, "static") == 0)
    {
        kind = STATIC;
    }
    else
    {
        kind = FIELD;
    }
    node = node->next;

    strcpy(type, ElemNodeData->token->value);
    node = node->next;
    strcpy(name, ElemNodeData->token->value);
    node = node->next;
    cg_add_symbol(&cg->cls_symbols, name, type, kind, position);

    while (strcmp(ElemNodeData->token->value, ";") != 0)
    {
        if (ElemNodeData->token->type == IDENTIFIER)
        {
            strcpy(name, ElemNodeData->token->value);
            cg_add_symbol(&cg->cls_symbols, name, type, kind, position);
        }
        node = node->next;
    }

    return node->next;
}

void cg_generate(CodeGen *cg, const Parser *ps)
{
    list_init(&cg->cls_symbols, sizeof(Symbol));
    const Node *node = ps->elements.start;

    while (node != NULL)
    {
        Element *elem = node->data;

        switch (elem->type)
        {
        case CLASS_VAR_DEC:
            node = cg_class_var_dec(cg, node);
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
