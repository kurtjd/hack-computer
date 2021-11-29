#include <stdio.h>
#include <string.h>
#include "compiler.h"
#include "tokenizer.h"

#define NUM_TYPES 3
#define TYPES "int", "char", "boolean"

/* START PROTOTYPES */

/* Program Structure */
// Compiles a class initializer
static const Node *cp_class(const Node *node);

// Compiles multiple class variable declarations
static const Node *cp_class_var(const Node *node, bool optional);

// Compiles a subroutine declaration
static const Node *cp_subroutine_dec(const Node *node, bool optional);

// Compiles a parameter list given to a subroutine
static const Node *cp_params(const Node *node);

// Compiles the body of a subroutine
static const Node *cp_subroutine_body(const Node *node);

// Compiles a variable declaration
static const Node *cp_var(const Node *node, bool optional);

/* Statements */
// Compiles multiple statements
static const Node *cp_statements(const Node *node);

// Compiles a single statement
static const Node *cp_statement(const Node *node, bool optional);

// Compiles a let statement
static const Node *cp_let(const Node *node);

// Compiles an if statement
static const Node *cp_if(const Node *node);

// Compiles a while statement
static const Node *cp_while(const Node *node);

// Compiles a do statement
static const Node *cp_do(const Node *node);

// Compiles a return statement
static const Node *cp_return(const Node *node);

/* Expressions */
// Compiles a single expression
static const Node *cp_expression(const Node *node);

// Compiles a single term
static const Node *cp_term(const Node *node);

// Compiles a comma-separated lsit of expressions
static const Node *cp_expression_list(const Node *node);

/* Literals */
// Compiles a literal value or type if it's found within a given list
static const Node *cp_values(const Node *node,
                             char values[][TOKEN_MAX_LEN], int num_vals,
                             TokenType types[], int num_types, bool optional);

// Compiles a literal token value
static const Node *cp_value(const Node *node, char *value, bool optional);

// Compiles a literal token type
static const Node *cp_type(const Node *node, TokenType type, bool optional);

/* END PROTOTYPES */

static const Node *cp_class(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    node = cp_value(node, "class", false);
    node = cp_type(node, IDENTIFIER, false);
    node = cp_value(node, "{", false);

    // Compile any class variables
    const Node *tmp = node;
    while (node != NULL)
    {
        tmp = node;
        node = cp_class_var(node, true);
    }
    node = tmp;

    // Compile any subroutines
    while (node != NULL)
    {
        tmp = node;
        node = cp_subroutine_dec(node, true);
    }

    node = cp_value(tmp, "}", false);

    return node;
}

static const Node *cp_class_var(const Node *node, bool optional)
{
    if (node == NULL)
    {
        return NULL;
    }

    // Ensure variable begins with static/field, and has a type, and identifier
    node = cp_values(node, (char[][TOKEN_MAX_LEN]){"static", "field"}, 2,
                     NULL, 0, optional);
    node = cp_values(node, (char[][TOKEN_MAX_LEN]){TYPES}, NUM_TYPES,
                     (TokenType[]){IDENTIFIER}, 1, false);
    node = cp_type(node, IDENTIFIER, false);

    const Node *tmp = node;
    const Node *tmp2 = NULL;

    // Search for additional identifiers
    while ((tmp = cp_value(tmp, ",", true)) != NULL)
    {
        tmp = cp_type(tmp, IDENTIFIER, false);
        tmp2 = tmp;
    }
    if (tmp2 != NULL)
    {
        node = tmp2;
    }

    node = cp_value(node, ";", false);

    return node;
}

static const Node *cp_subroutine_dec(const Node *node, bool optional)
{
    if (node == NULL)
    {
        return NULL;
    }

    /* Ensure declaration begins with a subroutine type followed by an
     * identifier and opening (
     */
    char subrtn_types[][TOKEN_MAX_LEN] = {"constructor", "function", "method"};
    node = cp_values(node, subrtn_types, 3, NULL, 0, optional);
    node = cp_values(node, (char[][TOKEN_MAX_LEN]){TYPES, "void"},
                     NUM_TYPES + 1, (TokenType[]){IDENTIFIER}, 1, false);
    node = cp_type(node, IDENTIFIER, false);
    node = cp_value(node, "(", false);

    // Compile any parameters
    const Node *tmp = cp_params(node);
    if (tmp != NULL)
    {
        node = tmp;
    }

    // Finally compile the body
    node = cp_value(node, ")", false);
    node = cp_subroutine_body(node);

    return node;
}

static const Node *cp_params(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    // Compile the first parameter
    node = cp_values(node, (char[][TOKEN_MAX_LEN]){TYPES}, NUM_TYPES,
                     (TokenType[]){IDENTIFIER}, 1, true);
    node = cp_type(node, IDENTIFIER, false);

    const Node *tmp = node;
    const Node *tmp2 = NULL;

    // The compile any additional parameters
    while ((tmp = cp_value(tmp, ",", true)) != NULL)
    {
        tmp = cp_values(tmp, (char[][TOKEN_MAX_LEN]){TYPES}, NUM_TYPES,
                        (TokenType[]){IDENTIFIER}, 1, false);
        tmp = cp_type(tmp, IDENTIFIER, false);
        tmp2 = tmp;
    }
    if (tmp2 != NULL)
    {
        node = tmp2;
    }

    return node;
}

static const Node *cp_subroutine_body(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    node = cp_value(node, "{", false);

    // Compile any variable declarations there may be
    const Node *tmp = node;
    while (node != NULL)
    {
        tmp = node;
        node = cp_var(node, true);
    }

    // Then compile any statements
    node = cp_statements(tmp);
    node = cp_value(node, "}", false);

    return node;
}

static const Node *cp_var(const Node *node, bool optional)
{
    if (node == NULL)
    {
        return NULL;
    }

    // Check for the keyword 'var' followed by a type and then an identifier
    node = cp_value(node, "var", optional);
    node = cp_values(node, (char[][TOKEN_MAX_LEN]){TYPES}, NUM_TYPES,
                     (TokenType[]){IDENTIFIER}, 1, false);
    node = cp_type(node, IDENTIFIER, false);

    const Node *tmp = node;
    const Node *tmp2 = NULL;

    // Search for additional comma-separated identifiers
    while ((tmp = cp_value(tmp, ",", true)) != NULL)
    {
        tmp = cp_type(tmp, IDENTIFIER, false);
        tmp2 = tmp;
    }
    if (tmp2 != NULL)
    {
        node = tmp2;
    }

    node = cp_value(node, ";", false);

    return node;
}

static const Node *cp_statements(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    // Keep compiling statements until no more are found
    const Node *tmp = node;
    do
    {
        tmp = node;
        node = cp_statement(node, true);
    } while (node != NULL);

    return tmp;
}

static const Node *cp_statement(const Node *node, bool optional)
{
    if (node == NULL)
    {
        return NULL;
    }

    const char *val = node->token.value;

    // Dispatch the correct rule depending on which statement we are handling
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

static const Node *cp_let(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    node = cp_value(node, "let", false);
    node = cp_type(node, IDENTIFIER, false);

    // Check to see if we are dealing with an array
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

    // Search for an optional else statement
    const Node *tmp = cp_value(node, "else", true);
    if (tmp != NULL)
    {
        node = cp_value(tmp, "{", false);
        node = cp_statements(node);
        node = cp_value(node, "}", false);
    }

    return node;
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

static const Node *cp_do(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    node = cp_value(node, "do", false);
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

    // Optionally grab an expression after return
    const Node *tmp = cp_expression(node);
    if (tmp != NULL)
    {
        node = tmp;
    }

    node = cp_value(node, ";", false);

    return node;
}

static const Node *cp_expression(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    // Get the first term of expression
    node = cp_term(node);

    // Optionally, get additional terms separated by an operator
    char ops[][TOKEN_MAX_LEN] = {"+", "-", "*", "/", "&", "|", "<", ">", "="};
    const Node *tmp = node;
    const Node *tmp2 = NULL;
    while ((tmp = cp_values(tmp, ops, 9, NULL, 0, true)) != NULL)
    {
        tmp = cp_term(tmp);
        tmp2 = tmp;
    }
    if (tmp2 != NULL)
    {
        node = tmp2;
    }

    return node;
}

static const Node *cp_expression_list(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    // Get the next expression
    const Node *tmp = cp_expression(node);
    if (tmp == NULL)
    {
        return node;
    }

    // Now search for additional comma-separated expressions
    node = tmp;
    const Node *tmp2 = NULL;
    while ((tmp = cp_value(tmp, ",", true)) != NULL)
    {
        tmp = cp_expression(tmp);
        tmp2 = tmp;
    }
    if (tmp2 != NULL)
    {
        node = tmp2;
    }

    return node;
}

static const Node *cp_term(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    TokenType types[] = {INT_CONST, STR_CONST, IDENTIFIER};
    char keywd_consts[][TOKEN_MAX_LEN] = {"true", "false", "null", "this"};

    /* Search for a constant or identifier.
     * If found, check if it's an array or function call.
     */
    const Node *tmp = cp_values(node, keywd_consts, 4, types, 4, true);
    if (tmp != NULL)
    {
        node = tmp;

        tmp = cp_value(node, "[", true);
        if (tmp != NULL)
        {
            node = cp_expression(tmp);
            node = cp_value(node, "]", false);
            return node;
        }

        tmp = cp_value(node, ".", true);
        if (tmp != NULL)
        {
            node = cp_type(tmp, IDENTIFIER, false);
        }

        tmp = cp_value(node, "(", true);
        if (tmp != NULL)
        {
            node = cp_expression_list(tmp);
            node = cp_value(node, ")", false);
        }

        return node;
    }

    // If no identifier/constant found, search for a parenthesized expression
    tmp = cp_value(node, "(", true);
    if (tmp != NULL)
    {
        node = cp_expression(tmp);
        node = cp_value(node, ")", false);
        return node;
    }

    // If none of the above is true, search for a unary operation
    node = cp_values(node, (char[][TOKEN_MAX_LEN]){"-", "~"}, 2,
                     NULL, 0, true);
    node = cp_term(node);

    return node;
}

static const Node *cp_values(const Node *node,
                             char values[][TOKEN_MAX_LEN], int num_vals,
                             TokenType types[], int num_types, bool optional)
{
    if (node == NULL)
    {
        return NULL;
    }

    // If values were provided, look for a match
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
    // If types were provided, look for a match
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

void cp_compile(const Tokenizer *tk)
{
    // All Jack programs must begin with a class so start there.
    cp_class(tk->tokens.start);
}
