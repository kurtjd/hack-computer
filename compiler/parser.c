#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "tokenizer.h"

#define NUM_TYPES 3
#define TYPES "int", "char", "boolean"

/* START PROTOTYPES */

/* Program Structure */
// Parses a class initializer
static const Node *ps_class(const Node *node);

// Parses multiple class variable declarations
static const Node *ps_class_var(const Node *node, bool optional);

// Parses a subroutine declaration
static const Node *ps_subroutine_dec(const Node *node, bool optional);

// Parses a parameter list given to a subroutine
static const Node *ps_params(const Node *node);

// Parses the body of a subroutine
static const Node *ps_subroutine_body(const Node *node);

// Parses a variable declaration
static const Node *ps_var(const Node *node, bool optional);

/* Statements */
// Parses multiple statements
static const Node *ps_statements(const Node *node);

// Parses a single statement
static const Node *ps_statement(const Node *node, bool optional);

// Parses a let statement
static const Node *ps_let(const Node *node);

// Parses an if statement
static const Node *ps_if(const Node *node);

// Parses a while statement
static const Node *ps_while(const Node *node);

// Parses a do statement
static const Node *ps_do(const Node *node);

// Parses a return statement
static const Node *ps_return(const Node *node);

/* Expressions */
// Parses a single expression
static const Node *ps_expression(const Node *node);

// Parses a single term
static const Node *ps_term(const Node *node);

// Parses a comma-separated lsit of expressions
static const Node *ps_expression_list(const Node *node);

/* Literals */
// Parses a literal value or type if it's found within a given list
static const Node *ps_values(const Node *node,
                             char values[][TOKEN_MAX_LEN], int num_vals,
                             TokenType types[], int num_types, bool optional);

// Parses a literal token value
static const Node *ps_value(const Node *node, char *value, bool optional);

// Parses a literal token type
static const Node *ps_type(const Node *node, TokenType type, bool optional);

/* END PROTOTYPES */

static const Node *ps_class(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    node = ps_value(node, "class", false);
    node = ps_type(node, IDENTIFIER, false);
    node = ps_value(node, "{", false);

    // Parse any class variables
    const Node *tmp = node;
    while (node != NULL)
    {
        tmp = node;
        node = ps_class_var(node, true);
    }
    node = tmp;

    // Parse any subroutines
    while (node != NULL)
    {
        tmp = node;
        node = ps_subroutine_dec(node, true);
    }

    node = ps_value(tmp, "}", false);

    return node;
}

static const Node *ps_class_var(const Node *node, bool optional)
{
    if (node == NULL)
    {
        return NULL;
    }

    // Ensure variable begins with static/field, and has a type, and identifier
    node = ps_values(node, (char[][TOKEN_MAX_LEN]){"static", "field"}, 2,
                     NULL, 0, optional);
    node = ps_values(node, (char[][TOKEN_MAX_LEN]){TYPES}, NUM_TYPES,
                     (TokenType[]){IDENTIFIER}, 1, false);
    node = ps_type(node, IDENTIFIER, false);

    const Node *tmp = node;
    const Node *tmp2 = NULL;

    // Search for additional identifiers
    while ((tmp = ps_value(tmp, ",", true)) != NULL)
    {
        tmp = ps_type(tmp, IDENTIFIER, false);
        tmp2 = tmp;
    }
    if (tmp2 != NULL)
    {
        node = tmp2;
    }

    node = ps_value(node, ";", false);

    return node;
}

static const Node *ps_subroutine_dec(const Node *node, bool optional)
{
    if (node == NULL)
    {
        return NULL;
    }

    /* Ensure declaration begins with a subroutine type followed by an
     * identifier and opening (
     */
    char subrtn_types[][TOKEN_MAX_LEN] = {"constructor", "function", "method"};
    node = ps_values(node, subrtn_types, 3, NULL, 0, optional);
    node = ps_values(node, (char[][TOKEN_MAX_LEN]){TYPES, "void"},
                     NUM_TYPES + 1, (TokenType[]){IDENTIFIER}, 1, false);
    node = ps_type(node, IDENTIFIER, false);
    node = ps_value(node, "(", false);

    // Parse any parameters
    const Node *tmp = ps_params(node);
    if (tmp != NULL)
    {
        node = tmp;
    }

    // Finally parse the body
    node = ps_value(node, ")", false);
    node = ps_subroutine_body(node);

    return node;
}

static const Node *ps_params(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    // Parse the first parameter
    node = ps_values(node, (char[][TOKEN_MAX_LEN]){TYPES}, NUM_TYPES,
                     (TokenType[]){IDENTIFIER}, 1, true);
    node = ps_type(node, IDENTIFIER, false);

    const Node *tmp = node;
    const Node *tmp2 = NULL;

    // The parse any additional parameters
    while ((tmp = ps_value(tmp, ",", true)) != NULL)
    {
        tmp = ps_values(tmp, (char[][TOKEN_MAX_LEN]){TYPES}, NUM_TYPES,
                        (TokenType[]){IDENTIFIER}, 1, false);
        tmp = ps_type(tmp, IDENTIFIER, false);
        tmp2 = tmp;
    }
    if (tmp2 != NULL)
    {
        node = tmp2;
    }

    return node;
}

static const Node *ps_subroutine_body(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    node = ps_value(node, "{", false);

    // Parse any variable declarations there may be
    const Node *tmp = node;
    while (node != NULL)
    {
        tmp = node;
        node = ps_var(node, true);
    }

    // Then parse any statements
    node = ps_statements(tmp);
    node = ps_value(node, "}", false);

    return node;
}

static const Node *ps_var(const Node *node, bool optional)
{
    if (node == NULL)
    {
        return NULL;
    }

    // Check for the keyword 'var' followed by a type and then an identifier
    node = ps_value(node, "var", optional);
    node = ps_values(node, (char[][TOKEN_MAX_LEN]){TYPES}, NUM_TYPES,
                     (TokenType[]){IDENTIFIER}, 1, false);
    node = ps_type(node, IDENTIFIER, false);

    const Node *tmp = node;
    const Node *tmp2 = NULL;

    // Search for additional comma-separated identifiers
    while ((tmp = ps_value(tmp, ",", true)) != NULL)
    {
        tmp = ps_type(tmp, IDENTIFIER, false);
        tmp2 = tmp;
    }
    if (tmp2 != NULL)
    {
        node = tmp2;
    }

    node = ps_value(node, ";", false);

    return node;
}

static const Node *ps_statements(const Node *node)
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
        node = ps_statement(node, true);
    } while (node != NULL);

    return tmp;
}

static const Node *ps_statement(const Node *node, bool optional)
{
    if (node == NULL)
    {
        return NULL;
    }

    const char *val = ((Token *)node->data)->value;

    // Dispatch the correct rule depending on which statement we are handling
    if (strcmp(val, "let") == 0)
    {
        node = ps_let(node);
    }
    else if (strcmp(val, "if") == 0)
    {
        node = ps_if(node);
    }
    else if (strcmp(val, "while") == 0)
    {
        node = ps_while(node);
    }
    else if (strcmp(val, "do") == 0)
    {
        node = ps_do(node);
    }
    else if (strcmp(val, "return") == 0)
    {
        node = ps_return(node);
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

static const Node *ps_let(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    node = ps_value(node, "let", false);
    node = ps_type(node, IDENTIFIER, false);

    // Check to see if we are dealing with an array
    const Node *tmp = ps_value(node, "[", true);
    if (tmp != NULL)
    {
        node = ps_expression(tmp);
        node = ps_value(node, "]", false);
    }

    node = ps_value(node, "=", false);
    node = ps_expression(node);
    node = ps_value(node, ";", false);

    return node;
}

static const Node *ps_if(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    node = ps_value(node, "if", false);
    node = ps_value(node, "(", false);
    node = ps_expression(node);
    node = ps_value(node, ")", false);
    node = ps_value(node, "{", false);
    node = ps_statements(node);
    node = ps_value(node, "}", false);

    // Search for an optional else statement
    const Node *tmp = ps_value(node, "else", true);
    if (tmp != NULL)
    {
        node = ps_value(tmp, "{", false);
        node = ps_statements(node);
        node = ps_value(node, "}", false);
    }

    return node;
}

static const Node *ps_while(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    node = ps_value(node, "while", false);
    node = ps_value(node, "(", false);
    node = ps_expression(node);
    node = ps_value(node, ")", false);
    node = ps_value(node, "{", false);
    node = ps_statements(node);
    node = ps_value(node, "}", false);

    return node;
}

static const Node *ps_do(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    node = ps_value(node, "do", false);
    node = ps_expression(node);
    node = ps_value(node, ";", false);

    return node;
}

static const Node *ps_return(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    node = ps_value(node, "return", false);

    // Optionally grab an expression after return
    const Node *tmp = ps_expression(node);
    if (tmp != NULL)
    {
        node = tmp;
    }

    node = ps_value(node, ";", false);

    return node;
}

static const Node *ps_expression(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    // Get the first term of expression
    node = ps_term(node);

    // Optionally, get additional terms separated by an operator
    char ops[][TOKEN_MAX_LEN] = {"+", "-", "*", "/", "&", "|", "<", ">", "="};
    const Node *tmp = node;
    const Node *tmp2 = NULL;
    while ((tmp = ps_values(tmp, ops, 9, NULL, 0, true)) != NULL)
    {
        tmp = ps_term(tmp);
        tmp2 = tmp;
    }
    if (tmp2 != NULL)
    {
        node = tmp2;
    }

    return node;
}

static const Node *ps_expression_list(const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    // Get the next expression
    const Node *tmp = ps_expression(node);
    if (tmp == NULL)
    {
        return node;
    }

    // Now search for additional comma-separated expressions
    node = tmp;
    const Node *tmp2 = NULL;
    while ((tmp = ps_value(tmp, ",", true)) != NULL)
    {
        tmp = ps_expression(tmp);
        tmp2 = tmp;
    }
    if (tmp2 != NULL)
    {
        node = tmp2;
    }

    return node;
}

static const Node *ps_term(const Node *node)
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
    const Node *tmp = ps_values(node, keywd_consts, 4, types, 4, true);
    if (tmp != NULL)
    {
        node = tmp;

        tmp = ps_value(node, "[", true);
        if (tmp != NULL)
        {
            node = ps_expression(tmp);
            node = ps_value(node, "]", false);
            return node;
        }

        tmp = ps_value(node, ".", true);
        if (tmp != NULL)
        {
            node = ps_type(tmp, IDENTIFIER, false);
        }

        tmp = ps_value(node, "(", true);
        if (tmp != NULL)
        {
            node = ps_expression_list(tmp);
            node = ps_value(node, ")", false);
        }

        return node;
    }

    // If no identifier/constant found, search for a parenthesized expression
    tmp = ps_value(node, "(", true);
    if (tmp != NULL)
    {
        node = ps_expression(tmp);
        node = ps_value(node, ")", false);
        return node;
    }

    // If none of the above is true, search for a unary operation
    node = ps_values(node, (char[][TOKEN_MAX_LEN]){"-", "~"}, 2,
                     NULL, 0, true);
    node = ps_term(node);

    return node;
}

static const Node *ps_values(const Node *node,
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
            if (strcmp(values[i], ((Token *)node->data)->value) == 0)
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
            if (types[i] == ((Token *)node->data)->type)
            {
                return node->next;
            }
        }
    }

    if (!optional)
    {
        fprintf(stderr, "Expected '%s' but received '%s'.\n",
                values[0] ? values[0] : "NULL", ((Token *)node->data)->value);
    }

    return NULL;
}

static const Node *ps_value(const Node *node, char *value, bool optional)
{
    char val[1][TOKEN_MAX_LEN];
    strcpy(val[0], value);
    return ps_values(node, val, 1, NULL, 0, optional);
}

static const Node *ps_type(const Node *node, TokenType type, bool optional)
{
    TokenType types[] = {type};
    return ps_values(node, NULL, 0, types, 1, optional);
}

void ps_parse(const Tokenizer *tk)
{
    // All Jack programs must begin with a class so start there.
    ps_class(tk->tokens.start);
}
