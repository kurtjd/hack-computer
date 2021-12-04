#include <stdio.h>
#include <string.h>
#include "codegen.h"

#define ElemNodeData ((Element *)(node->data))
#define SymNodeData ((Symbol *)(node->data))

static const char KINDS[][8] = {
    "field",
    "static",
    "arg",
    "local",
};

// Adds a symbol to the specified symbol table
static void cg_add_symbol(LinkedList *symtbl, const char *name,
                          const char *type, Kind kind, int index);

// Looks for a symbol first in the subroutine table and then in the class table
static const Symbol *cg_get_symbol(CodeGen *cg, const char *name);

// Returns the next index for a specified symbol kind in the symbol table
static int cg_get_next_index(const LinkedList *symtbl, Kind kind);

// Handles a class or subroutine variable declaration
static const Node *cg_var_dec(LinkedList *symtbl, const Node *node);

// Handles argument variables declared in a parameter list
static const Node *cg_params(CodeGen *cg, const Node *node);

// Handles a subroutine declaration
static void cg_subroutine_dec(CodeGen *cg, const Node *node);

// Handles a class declaration
static void cg_class_dec(CodeGen *cg, const Node *node);

// Handles an expression
static const Node *cg_expression(CodeGen *cg, const Node *node);

// Handles a term
static const Node *cg_term(CodeGen *cg, const Node *node);

// Handles an expression list
static const Node *cg_expression_list(CodeGen *cg, const Node *node);

// Handles a string
static const Node *cg_string(const Node *node);

// Handles an identifier found in a term
static const Node *cg_term_identifier(CodeGen *cg, const Node *node);

// Handles a symbol found in a term
static const Node *cg_term_symbol(CodeGen *cg, const Node *node);

// Handles a keyword found in a term
static const Node *cg_term_keyword(const Node *node);

// Handles a return statement
static const Node *cg_return(CodeGen *cg, const Node *node);

// Handles a let statement
static const Node *cg_let(CodeGen *cg, const Node *node);

// Handles a do statement
static const Node *cg_do(CodeGen *cg, const Node *node);

// Handles an if statement
static const Node *cg_if(CodeGen *cg, const Node *node);

// Handles a multiple statements
static const Node *cg_statements(CodeGen *cg, const Node *node);

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
        node = node->next;

        // If no match in subroutine table, switch to class table
        if (node == NULL && local)
        {
            node = cg->cls_symbols.start;
            local = false;
        }
    }

    return NULL;
}

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

static void cg_class_dec(CodeGen *cg, const Node *node)
{
    list_free(&cg->cls_symbols);
    list_init(&cg->cls_symbols, sizeof(Symbol));
    strcpy(cg->cur_cls, ElemNodeData->token->value);
}

static const Node *cg_expression(CodeGen *cg, const Node *node)
{
    char op;
    node = cg_term(cg, node->next);
    node = node->next;

    while (ElemNodeData->type == TOKEN)
    {
        op = ElemNodeData->token->value[0];
        node = cg_term(cg, node->next->next);

        switch (op)
        {
        case '+':
            printf("add\n");
            break;
        case '-':
            printf("sub\n");
            break;
        case '*':
            printf("call Math.multiply\n");
            break;
        case '/':
            printf("Math.divide\n");
            break;
        case '&':
            printf("and\n");
            break;
        case '|':
            printf("or\n");
            break;
        case '<':
            printf("lt\n");
            break;
        case '>':
            printf("gt\n");
            break;
        case '=':
            printf("eq\n");
            break;
        default:
            break;
        }

        node = node->next;
    }

    return node;
}

static const Node *cg_term(CodeGen *cg, const Node *node)
{
    switch (ElemNodeData->token->type)
    {
    case INT_CONST:
        printf("push constant %s\n", ElemNodeData->token->value);
        node = node->next;
        break;

    case STR_CONST:
        node = cg_string(node);
        break;

    case IDENTIFIER:
        node = cg_term_identifier(cg, node);
        break;

    case SYMBOL:
        node = cg_term_symbol(cg, node);
        break;

    case KEYWORD:
        node = cg_term_keyword(node);
        break;

    default:
        break;
    }

    return node;
}

static const Node *cg_expression_list(CodeGen *cg, const Node *node)
{
    while (ElemNodeData->type != EXPRESSION_LIST_END)
    {
        if (ElemNodeData->type == EXPRESSION)
        {
            node = cg_expression(cg, node->next);
        }
        node = node->next;
    }

    return node;
}

static const Node *cg_string(const Node *node)
{
    printf("push constant %ld\n", strlen(ElemNodeData->token->value));
    printf("call String.new\n");

    // Have to call this OS function for every character
    for (size_t i = 0; i < strlen(ElemNodeData->token->value); i++)
    {
        printf("push constant %d\n", ElemNodeData->token->value[i]);
        printf("call String.appendChar\n");
    }

    return node->next;
}

static const Node *cg_term_identifier(CodeGen *cg, const Node *node)
{
    // In case this identifier happens to be a function, save its name
    char func_name[TOKEN_MAX_LEN];
    strcpy(func_name, ElemNodeData->token->value);

    // Save the symbol found here for future use
    const Symbol *symbl = cg_get_symbol(cg, ElemNodeData->token->value);

    /* If a token is found, we are dealing with a function or array.
     * Otherwise, it's just a variable name.
     */
    node = node->next;
    if (ElemNodeData->type == TOKEN)
    {
        char op = ElemNodeData->token->value[0];
        if (op == '[')
        {
            printf("push %s %d\n", KINDS[symbl->kind], symbl->index);

            node = cg_expression(cg, node->next->next);
            node = node->next->next;

            printf("add\n");
        }
        else
        {
            // If a dot is found, append the next identifier to function name
            if (op == '.')
            {
                strcat(func_name, ".");
                node = node->next;
                strcat(func_name, ElemNodeData->token->value);
                node = node->next;
            }

            node = cg_expression_list(cg, node->next->next);
            node = node->next->next;

            printf("call %s\n", func_name);
        }
    }
    else
    {
        printf("push %s %d\n", KINDS[symbl->kind], symbl->index);
    }

    return node;
}

static const Node *cg_term_symbol(CodeGen *cg, const Node *node)
{
    /* Either we are dealing with a grouped, nested expression or
     * a unary operator.
     */
    if (ElemNodeData->token->value[0] == '(')
    {
        node = cg_expression(cg, node->next->next);
        node = node->next;
    }
    else
    {
        char op = ElemNodeData->token->value[0];
        node = cg_term(cg, node->next->next);

        if (op == '-')
        {
            printf("neg\n");
        }
        else if (op == '~')
        {
            printf("not\n");
        }
    }

    return node->next;
}

static const Node *cg_term_keyword(const Node *node)
{
    if (strcmp(ElemNodeData->token->value, "true") == 0)
    {
        printf("push constant -1\n");
    }
    else if (strcmp(ElemNodeData->token->value, "false") == 0)
    {
        printf("push constant 0\n");
    }
    else if (strcmp(ElemNodeData->token->value, "null") == 0)
    {
        printf("push constant 0\n");
    }
    else if (strcmp(ElemNodeData->token->value, "this") == 0)
    {
        printf("push pointer 0\n");
    }

    return node->next;
}

static const Node *cg_return(CodeGen *cg, const Node *node)
{
    if (((Element *)(node->next->data))->type == EXPRESSION)
    {
        node = cg_expression(cg, node->next->next);
    }
    else
    {
        printf("push constant 0\n");
    }

    printf("return\n");
    return node->next->next;
}

static const Node *cg_let(CodeGen *cg, const Node *node)
{
    // TODO: Handle array
    const Symbol *symbol = cg_get_symbol(cg, ElemNodeData->token->value);
    node = cg_expression(cg, node->next->next->next);

    printf("pop %s %d\n", KINDS[symbol->kind], symbol->index);

    return node->next->next;
}

static const Node *cg_do(CodeGen *cg, const Node *node)
{
    char func_name[TOKEN_MAX_LEN];
    strcpy(func_name, ElemNodeData->token->value);

    node = node->next;

    // If a dot is found, append the next identifier to function name
    if (ElemNodeData->token->value[0] == '.')
    {
        strcat(func_name, ".");
        node = node->next;
        strcat(func_name, ElemNodeData->token->value);
        node = node->next;
    }

    node = cg_expression_list(cg, node->next->next);
    node = node->next->next;

    printf("call %s\n", func_name);
    printf("pop temp 0\n"); // Discard return value

    return node;
}

static const Node *cg_if(CodeGen *cg, const Node *node)
{
    static int if_count = 0;
    int cur_if_count = if_count++;

    node = cg_expression(cg, node);

    printf("not\n");
    printf("if-goto %s_IF_END_%d\n", cg->cur_cls, cur_if_count);

    node = cg_statements(cg, node->next->next->next->next);
    node = node->next->next;

    // Handle else statement if there is one
    if (ElemNodeData->type != IF_STATEMENT_END)
    {
        printf("goto %s_ELSE_END_%d\n", cg->cur_cls, cur_if_count);
        printf("label %s_IF_END_%d\n", cg->cur_cls, cur_if_count);

        node = cg_statements(cg, node->next->next->next);
        node = node->next->next;

        printf("label %s_ELSE_END_%d\n", cg->cur_cls, cur_if_count);
    }
    else
    {
        printf("label %s_IF_END_%d\n", cg->cur_cls, cur_if_count);
    }

    return node;
}

static const Node *cg_while(CodeGen *cg, const Node *node)
{
    static int while_count = 0;
    int cur_while_count = while_count++;

    printf("label %s_WHILE_%d\n", cg->cur_cls, cur_while_count);

    node = cg_expression(cg, node);

    printf("not\n");
    printf("if-goto %s_WHILE_END_%d\n", cg->cur_cls, cur_while_count);

    node = cg_statements(cg, node->next->next->next->next);
    node = node->next->next;

    printf("goto %s_WHILE_%d\n", cg->cur_cls, cur_while_count);
    printf("label %s_WHILE_END_%d\n", cg->cur_cls, cur_while_count);

    return node;
}

static const Node *cg_statements(CodeGen *cg, const Node *node)
{
    while (ElemNodeData->type != STATEMENTS_END)
    {
        switch (ElemNodeData->type)
        {
        case RETURN_STATEMENT:
            node = cg_return(cg, node->next);
            break;
        case LET_STATEMENT:
            node = cg_let(cg, node->next->next);
            break;
        case DO_STATEMENT:
            node = cg_do(cg, node->next->next);
            break;
        case IF_STATEMENT:
            node = cg_if(cg, node->next->next->next->next);
            break;
        case WHILE_STATEMENT:
            node = cg_while(cg, node->next->next->next->next);
            break;
        default:
            break;
        }

        node = node->next;
    }

    return node;
}

void cg_generate(CodeGen *cg, const Parser *ps)
{
    list_init(&cg->cls_symbols, sizeof(Symbol));
    list_init(&cg->subr_symbols, sizeof(Symbol));

    const Node *node = ps->elements.start;
    while (node != NULL)
    {
        switch (ElemNodeData->type)
        {
        case CLASS:
            cg_class_dec(cg, node->next->next);
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
        case EXPRESSION:
            node = cg_expression(cg, node->next);
            break;
        case STATEMENTS:
            node = cg_statements(cg, node->next);
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
