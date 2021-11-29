#include <stdio.h>
#include <string.h>
#include "compiler.h"
#include "tokenizer.h"

#define NUM_STATEMENTS 5
#define NUM_TYPES 3
#define TYPES "int", "char", "boolean"

/* START PROTOTYPES */
static const Node *cp_values(const Node *node,
                             char values[][TOKEN_MAX_LEN], int num_vals,
                             TokenType types[], int num_types, bool optional);
static const Node *cp_value(const Node *node, char *value, bool optional);
static const Node *cp_type(const Node *node, TokenType type, bool optional);
static const Node *cp_statement(const Node *node, bool optional);
static const Node *cp_statements(const Node *node);
static const Node *cp_expression(const Node *node);
static const Node *cp_expression_list(const Node *node);
static const Node *cp_while(const Node *node);
static const Node *cp_if(const Node *node);
static const Node *cp_let(const Node *node);
static const Node *cp_return(const Node *node);
static const Node *cp_subroutine_call(const Node *node);
static const Node *cp_do(const Node *node);
static const Node *cp_class(const Node *node);
static const Node *cp_class_var(const Node *node, bool optional);
static const Node *cp_subroutine_dec(const Node *node);
/* END PROTOTYPES */

static const Node *cp_values(const Node *node,
                             char values[][TOKEN_MAX_LEN], int num_vals,
                             TokenType types[], int num_types, bool optional)
{
    if (node == NULL)
    {
        return NULL;
    }

    if (num_vals > 0)
    {
        for (int i = 0; i < num_vals; i++)
        {
            if (strcmp(values[i], node->token.value) == 0)
            {
                return node->next;
            }
        }
    }

    if (num_types > 0)
    {
        for (int i = 0; i < num_types; i++)
        {
            if (types[i] == node->token.type)
            {
                return node->next;
            }
        }
    }

    if (!optional)
    {
        fprintf(stderr, "Expected '%s' but received '%s'.\n",
                values[0] ? values[0] : "NULL", node->token.value);
    }

    return NULL;
}

static const Node *cp_value(const Node *node, char *value, bool optional)
{
    char val[1][TOKEN_MAX_LEN];
    strcpy(val[0], value);
    return cp_values(node, val, 1, NULL, 0, optional);
}

static const Node *cp_type(const Node *node, TokenType type, bool optional)
{
    TokenType types[] = {type};
    return cp_values(node, NULL, 0, types, 1, optional);
}

static const Node *cp_statements(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    int count = -1;
    const Node *tmp = node;
    do
    {
        tmp = node;
        node = cp_statement(node, true);
        count++;
    } while (node != NULL);

    if (count < 1)
    {
        fprintf(stderr, "Expected at least one statement.\n");
        return NULL;
    }

    return tmp;
}

static const Node *cp_expression(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    return node->next;
}

static const Node *cp_expression_list(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    return cp_expression(node);
}

static const Node *cp_while(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    node = cp_value(node, "while", false);
    node = cp_value(node, "(", false);
    node = cp_expression(node);
    node = cp_value(node, ")", false);
    node = cp_value(node, "{", false);
    node = cp_statements(node);
    node = cp_value(node, "}", false);

    return node;
}

static const Node *cp_if(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    node = cp_value(node, "if", false);
    node = cp_value(node, "(", false);
    node = cp_expression(node);
    node = cp_value(node, ")", false);
    node = cp_value(node, "{", false);
    node = cp_statements(node);
    node = cp_value(node, "}", false);

    const Node *tmp = cp_value(node, "else", true);
    if (tmp != NULL)
    {
        node = cp_value(tmp, "{", false);
        node = cp_statements(node);
        node = cp_value(node, "}", false);
    }

    return node;
}

static const Node *cp_let(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    node = cp_value(node, "let", false);
    node = cp_type(node, IDENTIFIER, false);

    const Node *tmp = cp_value(node, "[", true);
    if (tmp != NULL)
    {
        node = cp_expression(tmp);
        node = cp_value(node, "]", false);
    }

    node = cp_value(node, "=", false);
    node = cp_expression(node);
    node = cp_value(node, ";", false);

    return node;
}

static const Node *cp_return(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    node = cp_value(node, "return", false);

    const Node *tmp = cp_expression(node);
    if (tmp != NULL)
    {
        //node = tmp; Remove comment once cp_expression implemented
    }

    node = cp_value(node, ";", false);

    return node;
}

static const Node *cp_subroutine_call(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    node = cp_type(node, IDENTIFIER, false);

    const Node *tmp = cp_value(node, ".", true);
    if (tmp != NULL)
    {
        node = cp_type(tmp, IDENTIFIER, false);
    }

    node = cp_value(node, "(", false);
    node = cp_expression_list(node);
    node = cp_value(node, ")", false);

    return node;
}

static const Node *cp_do(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    node = cp_value(node, "do", false);
    node = cp_subroutine_call(node);
    node = cp_value(node, ";", false);

    return node;
}

static const Node *cp_statement(const Node *node, bool optional)
{
    if (node == NULL)
    {
        return NULL;
    }

    const char *val = node->token.value;

    if (strcmp(val, "let") == 0)
    {
        node = cp_let(node);
    }
    else if (strcmp(val, "if") == 0)
    {
        node = cp_if(node);
    }
    else if (strcmp(val, "while") == 0)
    {
        node = cp_while(node);
    }
    else if (strcmp(val, "do") == 0)
    {
        node = cp_do(node);
    }
    else if (strcmp(val, "return") == 0)
    {
        node = cp_return(node);
    }
    else
    {
        if (!optional)
        {
            fprintf(stderr, "Expected statement but received '%s'.\n", val);
        }

        node = NULL;
    }

    return node;
}

static const Node *cp_class(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    node = cp_value(node, "class", false);
    node = cp_type(node, IDENTIFIER, false);
    node = cp_value(node, "{", false);

    const Node *tmp = node;
    while (node != NULL)
    {
        tmp = node;
        node = cp_class_var(node, true);
    }
    node = tmp;

    node = cp_subroutine_dec(node);

    node = cp_value(node, "}", false);

    return node;
}

static const Node *cp_class_var(const Node *node, bool optional)
{
    if (node == NULL)
    {
        return NULL;
    }

    node = cp_values(node, (char[][TOKEN_MAX_LEN]){"static", "field"}, 2,
                     NULL, 0, optional);
    node = cp_values(node, (char[][TOKEN_MAX_LEN]){TYPES}, NUM_TYPES,
                     (TokenType[]){IDENTIFIER}, 1, false);
    node = cp_type(node, IDENTIFIER, false);

    const Node *tmp = node;
    const Node *tmp2 = NULL;

    while ((tmp = cp_value(tmp, ",", true)) != NULL)
    {
        tmp = cp_type(node, IDENTIFIER, false);
        tmp2 = tmp;
    }
    if (tmp2 != NULL)
    {
        node = tmp2;
    }

    node = cp_value(node, ";", true);

    return node;
}

static const Node *cp_subroutine_dec(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    return node->next->next;
}

void cp_compile(const Tokenizer *tk)
{
    cp_class(tk->tokens.start);
}
