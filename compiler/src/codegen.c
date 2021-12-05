#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "codegen.h"

#define ElemNodeData ((Element *)(node->data))
#define SymNodeData ((Symbol *)(node->data))

#define VM_MAX_LINE (FILENAME_MAX + 128)
#define VM_LINE_BUF 128
#define VM_CHUNK_SIZE (VM_LINE_BUF * VM_MAX_LINE)

#define MAX_FILES 32

// Holds the translated VM code
typedef struct VMProg
{
    char *data;
    size_t size;
    int line_count;
} VMProg;

static VMProg vmprog;

// Initialize the vm data
static bool vm_init(VMProg *prog)
{
    prog->size = VM_CHUNK_SIZE;
    prog->data = malloc(prog->size);
    if (prog->data == NULL)
    {
        fprintf(stderr, "Unable to allocate memory for VM.\n");
        return false;
    }

    *prog->data = '\0';
    prog->line_count = 0;

    return true;
}

// Add a line of VM
static void vm_add_line(VMProg *prog, const char *format, ...)
{
    char line[VM_MAX_LINE];
    sprintf(line, "%s\n", format);

    // Allow for variable arguments making it easier to compose vm instructions
    va_list args;
    va_start(args, format);
    vsprintf(prog->data + strlen(prog->data), line, args);
    va_end(args);

    prog->line_count++;

    if ((prog->line_count % VM_LINE_BUF) == 0)
    {
        prog->size += VM_CHUNK_SIZE;
        prog->data = realloc(prog->data, prog->size);
        if (prog->data == NULL)
        {
            fprintf(stderr, "Unable to reallocate memory for VM.\n");
            exit(1);
        }
    }
}

// Free the VM data
static void vm_free(VMProg *prog)
{
    if (prog->data != NULL)
    {
        free(prog->data);
    }

    prog->data = NULL;
}

static const char KINDS[][9] = {
    "this",
    "static",
    "argument",
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
static const Node *cg_expression_list(CodeGen *cg, const Node *node,
                                      int *num_expr);

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

    if (node == NULL)
    {
        node = cg->cls_symbols.start;
        local = false;
    }

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
        /*// Get the 'type' of variable
        if (ElemNodeData->token->type == KEYWORD)
        {
            strcpy(type, ElemNodeData->token->value);
        }
        else if (ElemNodeData->token->type == IDENTIFIER)
        {
            cg_add_symbol(&cg->subr_symbols, ElemNodeData->token->value, type,
                          ARG, cg_get_next_index(&cg->subr_symbols, ARG));
        }

        node = node->next;*/

        strcpy(type, ElemNodeData->token->value);
        node = node->next;
        cg_add_symbol(&cg->subr_symbols, ElemNodeData->token->value, type,
                      ARG, cg_get_next_index(&cg->subr_symbols, ARG));
        node = node->next;
        if (ElemNodeData->type == TOKEN)
        {
            node = node->next;
        }
    }

    return node;
}

static void cg_subroutine_dec(CodeGen *cg, const Node *node)
{
    list_free(&cg->subr_symbols);
    list_init(&cg->subr_symbols, sizeof(Symbol));

    strcpy(cg->cur_func.kind, ElemNodeData->token->value);
    strcpy(cg->cur_func.type, (((Element *)(node->next->data))->token->value));
    strcpy(cg->cur_func.name,
           (((Element *)(node->next->next->data))->token->value));

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
            vm_add_line(&vmprog, "add");
            break;
        case '-':
            vm_add_line(&vmprog, "sub");
            break;
        case '*':
            vm_add_line(&vmprog, "call Math.multiply 2");
            break;
        case '/':
            vm_add_line(&vmprog, "call Math.divide 2");
            break;
        case '&':
            vm_add_line(&vmprog, "and");
            break;
        case '|':
            vm_add_line(&vmprog, "or");
            break;
        case '<':
            vm_add_line(&vmprog, "lt");
            break;
        case '>':
            vm_add_line(&vmprog, "gt");
            break;
        case '=':
            vm_add_line(&vmprog, "eq");
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
        vm_add_line(&vmprog, "push constant %s", ElemNodeData->token->value);
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

static const Node *cg_expression_list(CodeGen *cg, const Node *node,
                                      int *num_expr)
{
    while (ElemNodeData->type != EXPRESSION_LIST_END)
    {
        if (ElemNodeData->type == EXPRESSION)
        {
            (*num_expr)++;
            node = cg_expression(cg, node->next);
        }
        node = node->next;
    }

    return node;
}

static const Node *cg_string(const Node *node)
{
    vm_add_line(&vmprog, "push constant %ld", strlen(ElemNodeData->token->value));
    vm_add_line(&vmprog, "call String.new 1");

    // Have to call this OS function for every character
    for (size_t i = 0; i < strlen(ElemNodeData->token->value); i++)
    {
        vm_add_line(&vmprog, "push constant %d", ElemNodeData->token->value[i]);
        vm_add_line(&vmprog, "call String.appendChar 2");
    }

    return node->next;
}

static const Node *cg_term_identifier(CodeGen *cg, const Node *node)
{
    // In case this identifier happens to be a function, save its name
    char func_name[TOKEN_MAX_LEN * 2];
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
            node = cg_expression(cg, node->next->next);
            node = node->next->next;

            vm_add_line(&vmprog, "push %s %d", KINDS[symbl->kind],
                        symbl->index);
            vm_add_line(&vmprog, "add");
            vm_add_line(&vmprog, "pop pointer 1");
            vm_add_line(&vmprog, "push that 0");
        }
        else
        {
            int num_args = 0;

            const Symbol *symbol = cg_get_symbol(cg, func_name);
            if (symbol != NULL)
            {
                strcpy(func_name, symbol->type);
                vm_add_line(&vmprog, "push %s %d", KINDS[symbol->kind],
                            symbol->index);
                num_args++;
            }

            /* If a dot is found, append the next identifier to function name.
             * Otherwise make the function name Class.func_name
             * and call as method
             */
            if (op == '.')
            {
                strcat(func_name, ".");
                node = node->next;
                strcat(func_name, ElemNodeData->token->value);
                node = node->next;
            }
            else
            {
                char tmp_func[TOKEN_MAX_LEN];
                strcpy(tmp_func, func_name);
                sprintf(func_name, "%s.%s", cg->cur_cls, tmp_func);
                num_args++;

                vm_add_line(&vmprog, "push pointer 0");
            }

            node = cg_expression_list(cg, node->next->next, &num_args);
            node = node->next->next;

            vm_add_line(&vmprog, "call %s %d", func_name, num_args);
        }
    }
    else
    {
        vm_add_line(&vmprog, "push %s %d", KINDS[symbl->kind], symbl->index);
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
            vm_add_line(&vmprog, "neg");
        }
        else if (op == '~')
        {
            vm_add_line(&vmprog, "not");
        }
    }

    return node->next;
}

static const Node *cg_term_keyword(const Node *node)
{
    if (strcmp(ElemNodeData->token->value, "true") == 0)
    {
        vm_add_line(&vmprog, "push constant 0");
        vm_add_line(&vmprog, "not");
    }
    else if (strcmp(ElemNodeData->token->value, "false") == 0)
    {
        vm_add_line(&vmprog, "push constant 0");
    }
    else if (strcmp(ElemNodeData->token->value, "null") == 0)
    {
        vm_add_line(&vmprog, "push constant 0");
    }
    else if (strcmp(ElemNodeData->token->value, "this") == 0)
    {
        vm_add_line(&vmprog, "push pointer 0");
    }

    return node->next;
}

static const Node *cg_return(CodeGen *cg, const Node *node)
{
    if (((Element *)(node->next->data))->type == EXPRESSION)
    {
        node = cg_expression(cg, node->next->next);
    }
    else if (strcmp(((Element *)(node->next->data))->token->value, "this") == 0)
    {
        vm_add_line(&vmprog, "push pointer 0");
    }
    else
    {
        vm_add_line(&vmprog, "push constant 0");
    }

    vm_add_line(&vmprog, "return");
    return node->next->next;
}

static const Node *cg_let(CodeGen *cg, const Node *node)
{
    bool is_array = false;
    const Symbol *symbol = cg_get_symbol(cg, ElemNodeData->token->value);
    node = node->next;

    if (ElemNodeData->token->value[0] == '[')
    {
        node = cg_expression(cg, node->next->next);
        node = node->next->next;
        is_array = true;

        vm_add_line(&vmprog, "push %s %d", KINDS[symbol->kind], symbol->index);
        vm_add_line(&vmprog, "add");
    }

    node = cg_expression(cg, node->next->next);

    if (is_array)
    {
        vm_add_line(&vmprog, "pop temp 0");
        vm_add_line(&vmprog, "pop pointer 1");
        vm_add_line(&vmprog, "push temp 0");
        vm_add_line(&vmprog, "pop that 0");
    }
    else
    {
        vm_add_line(&vmprog, "pop %s %d", KINDS[symbol->kind], symbol->index);
    }

    return node->next->next;
}

static const Node *cg_do(CodeGen *cg, const Node *node)
{
    int num_args = 0;
    char func_name[TOKEN_MAX_LEN * 2];
    strcpy(func_name, ElemNodeData->token->value);

    node = node->next;

    // If a dot is found, append the next identifier to function name
    if (ElemNodeData->token->value[0] == '.')
    {
        const Symbol *symbol = cg_get_symbol(cg, func_name);
        if (symbol != NULL)
        {
            strcpy(func_name, symbol->type);
            vm_add_line(&vmprog, "push %s %d", KINDS[symbol->kind],
                        symbol->index);
            num_args++;
        }

        strcat(func_name, ".");
        node = node->next;
        strcat(func_name, ElemNodeData->token->value);
        node = node->next;
    }
    else
    {
        // Otherwise make the function name Class.func_name and call as method
        char tmp_func[TOKEN_MAX_LEN];
        strcpy(tmp_func, func_name);
        sprintf(func_name, "%s.%s", cg->cur_cls, tmp_func);
        num_args++;

        vm_add_line(&vmprog, "push pointer 0");
    }

    node = cg_expression_list(cg, node->next->next, &num_args);
    node = node->next->next;

    vm_add_line(&vmprog, "call %s %d", func_name, num_args);
    vm_add_line(&vmprog, "pop temp 0"); // Discard return value

    return node;
}

static const Node *cg_if(CodeGen *cg, const Node *node)
{
    static int if_count = 0;
    int cur_if_count = if_count++;

    node = cg_expression(cg, node);

    vm_add_line(&vmprog, "not");
    vm_add_line(&vmprog, "if-goto %s_IF_END_%d", cg->cur_cls, cur_if_count);

    node = cg_statements(cg, node->next->next->next->next);
    node = node->next->next;

    // Handle else statement if there is one
    if (ElemNodeData->type != IF_STATEMENT_END)
    {
        vm_add_line(&vmprog, "goto %s_ELSE_END_%d", cg->cur_cls, cur_if_count);
        vm_add_line(&vmprog, "label %s_IF_END_%d", cg->cur_cls, cur_if_count);

        node = cg_statements(cg, node->next->next->next);
        node = node->next->next;

        vm_add_line(&vmprog, "label %s_ELSE_END_%d", cg->cur_cls, cur_if_count);
    }
    else
    {
        vm_add_line(&vmprog, "label %s_IF_END_%d", cg->cur_cls, cur_if_count);
    }

    return node;
}

static const Node *cg_while(CodeGen *cg, const Node *node)
{
    static int while_count = 0;
    int cur_while_count = while_count++;

    vm_add_line(&vmprog, "label %s_WHILE_%d", cg->cur_cls, cur_while_count);

    node = cg_expression(cg, node);

    vm_add_line(&vmprog, "not");
    vm_add_line(&vmprog, "if-goto %s_WHILE_END_%d", cg->cur_cls, cur_while_count);

    node = cg_statements(cg, node->next->next->next->next);
    node = node->next->next;

    vm_add_line(&vmprog, "goto %s_WHILE_%d", cg->cur_cls, cur_while_count);
    vm_add_line(&vmprog, "label %s_WHILE_END_%d", cg->cur_cls, cur_while_count);

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

static void cg_subroutine_body(const CodeGen *cg)
{
    int num_vars = 0;
    Node *node = cg->subr_symbols.start;
    while (node != NULL)
    {
        if (SymNodeData->kind == VAR)
        {
            num_vars++;
        }
        node = node->next;
    }

    vm_add_line(&vmprog, "function %s.%s %d", cg->cur_cls, cg->cur_func.name, num_vars);

    if (strcmp(cg->cur_func.kind, "method") == 0)
    {
        vm_add_line(&vmprog, "push argument 0");
        vm_add_line(&vmprog, "pop pointer 0");
    }
    else if (strcmp(cg->cur_func.kind, "constructor") == 0)
    {
        int num_fields = 0;
        Node *node = cg->cls_symbols.start;
        while (node != NULL)
        {
            if (SymNodeData->kind == FIELD)
            {
                num_fields++;
            }
            node = node->next;
        }

        vm_add_line(&vmprog, "push constant %d", num_fields);
        vm_add_line(&vmprog, "call Memory.alloc 1");
        vm_add_line(&vmprog, "pop pointer 0");
    }
}

void cg_generate(CodeGen *cg, const Parser *ps)
{
    vm_init(&vmprog);
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
            cg_subroutine_dec(cg, node->next);
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
            if ((((Element *)(node->prev->data))->type == VAR_DEC_END) ||
                (((Element *)(node->prev->prev->data))->type == SUBROUTINE_BODY))
            {
                cg_subroutine_body(cg);
            }

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
    vm_free(&vmprog);
}

void cg_print_symtbl(LinkedList *symtbl)
{
    printf("Symbol Table:\n");
    const Node *node = symtbl->start;

    while (node != NULL)
    {
        Symbol *symbl = node->data;
        printf("Name: %s\nType: %s\nKind: %d\nPos: %d\n\n", symbl->name,
               symbl->type, symbl->kind, symbl->index);
        node = node->next;
    }
}

bool cg_gen_vm_file(const char *filepath)
{
    char newfile[(FILENAME_MAX * 2) + 16] = {'\0'};
    strcpy(newfile, filepath);
    newfile[strlen(newfile) - 5] = '\0';
    strcat(newfile, ".vm");

    FILE *fp = fopen(newfile, "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Unable to generate VM file.\n");
        return false;
    }

    fwrite(vmprog.data, strlen(vmprog.data), 1, fp);

    fclose(fp);
    return true;
}
