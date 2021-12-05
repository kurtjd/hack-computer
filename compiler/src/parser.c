#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "tokenizer.h"

#define NUM_TYPES 3
#define TYPES "int", "char", "boolean"

static const char ELEM_TYPES[][32] = {
    "token",
    "class",
    "classVarDec",
    "type",
    "subroutineDec",
    "parameterList",
    "subroutineBody",
    "varDec",
    "subroutineName",
    "varName",
    "statements",
    "statement",
    "letStatement",
    "ifStatement",
    "whileStatement",
    "doStatement",
    "returnStatement",
    "expression",
    "term",
    "subroutineCall",
    "expressionList",
    "op",
    "unaryOp",
    "keywordConstant",
};

/* START PROTOTYPES */
static void ps_init(Parser *ps);

static void ps_add_elem(Parser *ps, ElementType type, const Token *token);

/* Program Structure */
// Parses a class initializer
static const Node *ps_class(Parser *ps, const Node *node);

// Parses multiple class variable declarations
static const Node *ps_class_var(Parser *ps, const Node *node, bool optional);

// Parses a subroutine declaration
static const Node *ps_subroutine_dec(Parser *ps, const Node *node,
                                     bool optional);

// Parses a parameter list given to a subroutine
static const Node *ps_params(Parser *ps, const Node *node);

// Parses the body of a subroutine
static const Node *ps_subroutine_body(Parser *ps, const Node *node);

// Parses a variable declaration
static const Node *ps_var(Parser *ps, const Node *node, bool optional);

/* Statements */
// Parses multiple statements
static const Node *ps_statements(Parser *ps, const Node *node);

// Parses a single statement
static const Node *ps_statement(Parser *ps, const Node *node, bool optional);

// Parses a let statement
static const Node *ps_let(Parser *ps, const Node *node);

// Parses an if statement
static const Node *ps_if(Parser *ps, const Node *node);

// Parses a while statement
static const Node *ps_while(Parser *ps, const Node *node);

// Parses a do statement
static const Node *ps_do(Parser *ps, const Node *node);

// Parses a return statement
static const Node *ps_return(Parser *ps, const Node *node);

/* Expressions */
// Parses a single expression
static const Node *ps_expression(Parser *ps, const Node *node);

// Parses a subroutine call
static const Node *ps_subroutine_call(Parser *ps, const Node *node);

// Parses a single term
static const Node *ps_term(Parser *ps, const Node *node);

// Parses a comma-separated lsit of expressions
static const Node *ps_expression_list(Parser *ps, const Node *node);

/* Literals */
// Parses a literal value or type if it's found within a given list
static const Node *ps_values(Parser *ps, const Node *node,
                             char values[][TOKEN_MAX_LEN], int num_vals,
                             TokenType types[], int num_types, bool optional);

// Parses a literal token value
static const Node *ps_value(Parser *ps, const Node *node, char *value,
                            bool optional);

// Parses a literal token type
static const Node *ps_type(Parser *ps, const Node *node, TokenType type,
                           bool optional);

// Removes the last element if the node is NULL
static bool ps_remove_null(Parser *ps, const Node *node);

/* END PROTOTYPES */
static void ps_init(Parser *ps)
{
    list_init(&ps->elements, sizeof(Element));
}

static void ps_add_elem(Parser *ps, ElementType type, const Token *token)
{
    Element elem = {type, token};
    list_add(&ps->elements, &elem);
}

static bool ps_remove_null(Parser *ps, const Node *node)
{
    if (node == NULL)
    {
        list_reverse_to(&ps->elements, ps->elements.end->prev);
        return true;
    }

    return false;
}

static const Node *ps_class(Parser *ps, const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    ps_add_elem(ps, CLASS, NULL);

    node = ps_value(ps, node, "class", false);
    node = ps_type(ps, node, IDENTIFIER, false);
    node = ps_value(ps, node, "{", false);

    // Parse any class variables
    const Node *tmp = node;
    while (node != NULL)
    {
        tmp = node;
        node = ps_class_var(ps, node, true);
    }
    node = tmp;

    // Parse any subroutines
    while (node != NULL)
    {
        tmp = node;
        node = ps_subroutine_dec(ps, node, true);
    }

    node = ps_value(ps, tmp, "}", false);

    if (!ps_remove_null(ps, node))
    {
        ps_add_elem(ps, CLASS_END, NULL);
    }

    return node;
}

static const Node *ps_class_var(Parser *ps, const Node *node, bool optional)
{
    if (node == NULL)
    {
        return NULL;
    }

    ps_add_elem(ps, CLASS_VAR_DEC, NULL);

    // Ensure variable begins with static/field, and has a type, and identifier
    node = ps_values(ps, node, (char[][TOKEN_MAX_LEN]){"static", "field"}, 2,
                     NULL, 0, optional);
    node = ps_values(ps, node, (char[][TOKEN_MAX_LEN]){TYPES}, NUM_TYPES,
                     (TokenType[]){IDENTIFIER}, 1, false);
    node = ps_type(ps, node, IDENTIFIER, false);

    const Node *tmp = node;
    const Node *tmp2 = NULL;

    // Search for additional identifiers
    while ((tmp = ps_value(ps, tmp, ",", true)) != NULL)
    {
        tmp = ps_type(ps, tmp, IDENTIFIER, false);
        tmp2 = tmp;
    }
    if (tmp2 != NULL)
    {
        node = tmp2;
    }

    node = ps_value(ps, node, ";", false);

    if (!ps_remove_null(ps, node))
    {
        ps_add_elem(ps, CLASS_VAR_DEC_END, NULL);
    }

    return node;
}

static const Node *ps_subroutine_dec(Parser *ps, const Node *node,
                                     bool optional)
{
    if (node == NULL)
    {
        return NULL;
    }

    ps_add_elem(ps, SUBROUTINE_DEC, NULL);

    /* Ensure declaration begins with a subroutine type followed by an
     * identifier and opening (
     */
    char subrtn_types[][TOKEN_MAX_LEN] = {"constructor", "function", "method"};
    node = ps_values(ps, node, subrtn_types, 3, NULL, 0, optional);
    node = ps_values(ps, node, (char[][TOKEN_MAX_LEN]){TYPES, "void"},
                     NUM_TYPES + 1, (TokenType[]){IDENTIFIER}, 1, false);
    node = ps_type(ps, node, IDENTIFIER, false);
    node = ps_value(ps, node, "(", false);

    // Parse any parameters
    const Node *tmp = ps_params(ps, node);
    if (tmp != NULL)
    {
        node = tmp;
    }

    // Finally parse the body
    node = ps_value(ps, node, ")", false);
    node = ps_subroutine_body(ps, node);

    if (!ps_remove_null(ps, node))
    {
        ps_add_elem(ps, SUBROUTINE_DEC_END, NULL);
    }

    return node;
}

static const Node *ps_params(Parser *ps, const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    ps_add_elem(ps, PARAM_LIST, NULL);

    // Parse the first parameter
    node = ps_values(ps, node, (char[][TOKEN_MAX_LEN]){TYPES}, NUM_TYPES,
                     (TokenType[]){IDENTIFIER}, 1, true);

    node = ps_type(ps, node, IDENTIFIER, false);

    const Node *tmp = node;
    const Node *tmp2 = NULL;

    // Then parse any additional parameters
    while ((tmp = ps_value(ps, tmp, ",", true)) != NULL)
    {
        tmp = ps_values(ps, tmp, (char[][TOKEN_MAX_LEN]){TYPES}, NUM_TYPES,
                        (TokenType[]){IDENTIFIER}, 1, false);
        tmp = ps_type(ps, tmp, IDENTIFIER, false);
        tmp2 = tmp;
    }

    if (tmp2 != NULL)
    {
        node = tmp2;
    }

    ps_add_elem(ps, PARAM_LIST_END, NULL);

    return node;
}

static const Node *ps_subroutine_body(Parser *ps, const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    ps_add_elem(ps, SUBROUTINE_BODY, NULL);

    node = ps_value(ps, node, "{", false);

    // Parse any variable declarations there may be
    const Node *tmp = node;
    while (node != NULL)
    {
        tmp = node;
        node = ps_var(ps, node, true);
    }

    // Then parse any statements
    node = ps_statements(ps, tmp);
    node = ps_value(ps, node, "}", false);

    ps_add_elem(ps, SUBROUTINE_BODY_END, NULL);

    return node;
}

static const Node *ps_var(Parser *ps, const Node *node, bool optional)
{
    if (node == NULL)
    {
        return NULL;
    }

    ps_add_elem(ps, VAR_DEC, NULL);

    // Check for the keyword 'var' followed by a type and then an identifier
    node = ps_value(ps, node, "var", optional);
    node = ps_values(ps, node, (char[][TOKEN_MAX_LEN]){TYPES}, NUM_TYPES,
                     (TokenType[]){IDENTIFIER}, 1, false);
    node = ps_type(ps, node, IDENTIFIER, false);

    const Node *tmp = node;
    const Node *tmp2 = NULL;

    // Search for additional comma-separated identifiers
    while ((tmp = ps_value(ps, tmp, ",", true)) != NULL)
    {
        tmp = ps_type(ps, tmp, IDENTIFIER, false);
        tmp2 = tmp;
    }
    if (tmp2 != NULL)
    {
        node = tmp2;
    }

    node = ps_value(ps, node, ";", false);

    if (!ps_remove_null(ps, node))
    {
        ps_add_elem(ps, VAR_DEC_END, NULL);
    }

    return node;
}

static const Node *ps_statements(Parser *ps, const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    ps_add_elem(ps, STATEMENTS, NULL);

    // Keep compiling statements until no more are found
    const Node *tmp = node;
    do
    {
        tmp = node;
        node = ps_statement(ps, node, true);
    } while (node != NULL);

    ps_add_elem(ps, STATEMENTS_END, NULL);

    return tmp;
}

static const Node *ps_statement(Parser *ps, const Node *node, bool optional)
{
    if (node == NULL)
    {
        return NULL;
    }

    const char *val = ((Token *)node->data)->value;

    // Dispatch the correct rule depending on which statement we are handling
    if (strcmp(val, "let") == 0)
    {
        node = ps_let(ps, node);
    }
    else if (strcmp(val, "if") == 0)
    {
        node = ps_if(ps, node);
    }
    else if (strcmp(val, "while") == 0)
    {
        node = ps_while(ps, node);
    }
    else if (strcmp(val, "do") == 0)
    {
        node = ps_do(ps, node);
    }
    else if (strcmp(val, "return") == 0)
    {
        node = ps_return(ps, node);
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

static const Node *ps_let(Parser *ps, const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    ps_add_elem(ps, LET_STATEMENT, NULL);

    node = ps_value(ps, node, "let", false);
    node = ps_type(ps, node, IDENTIFIER, false);

    // Check to see if we are dealing with an array
    const Node *tmp = ps_value(ps, node, "[", true);
    if (tmp != NULL)
    {
        node = ps_expression(ps, tmp);
        node = ps_value(ps, node, "]", false);
    }

    node = ps_value(ps, node, "=", false);
    node = ps_expression(ps, node);
    node = ps_value(ps, node, ";", false);

    ps_add_elem(ps, LET_STATEMENT_END, NULL);

    return node;
}

static const Node *ps_if(Parser *ps, const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    ps_add_elem(ps, IF_STATEMENT, NULL);

    node = ps_value(ps, node, "if", false);
    node = ps_value(ps, node, "(", false);
    node = ps_expression(ps, node);
    node = ps_value(ps, node, ")", false);
    node = ps_value(ps, node, "{", false);
    node = ps_statements(ps, node);
    node = ps_value(ps, node, "}", false);

    // Search for an optional else statement
    const Node *tmp = ps_value(ps, node, "else", true);
    if (tmp != NULL)
    {
        node = ps_value(ps, tmp, "{", false);
        node = ps_statements(ps, node);
        node = ps_value(ps, node, "}", false);
    }

    ps_add_elem(ps, IF_STATEMENT_END, NULL);

    return node;
}

static const Node *ps_while(Parser *ps, const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    ps_add_elem(ps, WHILE_STATEMENT, NULL);

    node = ps_value(ps, node, "while", false);
    node = ps_value(ps, node, "(", false);
    node = ps_expression(ps, node);
    node = ps_value(ps, node, ")", false);
    node = ps_value(ps, node, "{", false);
    node = ps_statements(ps, node);
    node = ps_value(ps, node, "}", false);

    ps_add_elem(ps, WHILE_STATEMENT_END, NULL);

    return node;
}

static const Node *ps_do(Parser *ps, const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    ps_add_elem(ps, DO_STATEMENT, NULL);

    node = ps_value(ps, node, "do", false);
    node = ps_subroutine_call(ps, node);
    node = ps_value(ps, node, ";", false);

    ps_add_elem(ps, DO_STATEMENT_END, NULL);

    return node;
}

static const Node *ps_return(Parser *ps, const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    ps_add_elem(ps, RETURN_STATEMENT, NULL);

    node = ps_value(ps, node, "return", false);

    // Optionally grab an expression after return
    const Node *tmp = ps_expression(ps, node);
    if (tmp != NULL)
    {
        node = tmp;
    }

    node = ps_value(ps, node, ";", false);

    if (!ps_remove_null(ps, node))
    {
        ps_add_elem(ps, RETURN_STATEMENT_END, NULL);
    }

    return node;
}

static const Node *ps_expression(Parser *ps, const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    ps_add_elem(ps, EXPRESSION, NULL);

    // Get the first term of expression
    node = ps_term(ps, node);

    // Optionally, get additional terms separated by an operator
    char ops[][TOKEN_MAX_LEN] = {"+", "-", "*", "/", "&", "|", "<", ">", "="};
    const Node *tmp = node;
    const Node *tmp2 = NULL;
    while ((tmp = ps_values(ps, tmp, ops, 9, NULL, 0, true)) != NULL)
    {
        tmp = ps_term(ps, tmp);
        tmp2 = tmp;
    }
    if (tmp2 != NULL)
    {
        node = tmp2;
    }

    if (!ps_remove_null(ps, node))
    {
        ps_add_elem(ps, EXPRESSION_END, NULL);
    }

    return node;
}

static const Node *ps_expression_list(Parser *ps, const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    ps_add_elem(ps, EXPRESSION_LIST, NULL);

    // Get the next expression
    const Node *tmp = ps_expression(ps, node);
    if (tmp == NULL)
    {
        ps_add_elem(ps, EXPRESSION_LIST_END, NULL);
        return node;
    }

    // Now search for additional comma-separated expressions
    node = tmp;
    const Node *tmp2 = NULL;
    while ((tmp = ps_value(ps, tmp, ",", true)) != NULL)
    {
        tmp = ps_expression(ps, tmp);
        tmp2 = tmp;
    }
    if (tmp2 != NULL)
    {
        node = tmp2;
    }

    ps_add_elem(ps, EXPRESSION_LIST_END, NULL);

    return node;
}

static const Node *ps_subroutine_call(Parser *ps, const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    node = ps_type(ps, node, IDENTIFIER, false);

    const Node *tmp = ps_value(ps, node, ".", true);
    if (tmp != NULL)
    {
        node = ps_type(ps, tmp, IDENTIFIER, false);
    }

    node = ps_value(ps, node, "(", true);
    node = ps_expression_list(ps, node);
    node = ps_value(ps, node, ")", false);

    return node;
}

static const Node *ps_term(Parser *ps, const Node *node)
{
    if (node == NULL)
    {
        return NULL;
    }

    ps_add_elem(ps, TERM, NULL);

    TokenType types[] = {INT_CONST, STR_CONST, IDENTIFIER};
    char keywd_consts[][TOKEN_MAX_LEN] = {"true", "false", "null", "this"};

    /* Search for a constant or identifier.
     * If found, check if it's an array or function call.
     */
    const Node *tmp = ps_values(ps, node, keywd_consts, 4, types, 4, true);
    if (tmp != NULL)
    {
        node = tmp;

        // Array
        tmp = ps_value(ps, node, "[", true);
        if (tmp != NULL)
        {
            node = ps_expression(ps, tmp);
            node = ps_value(ps, node, "]", false);

            ps_add_elem(ps, TERM_END, NULL);
            return node;
        }

        // Function
        tmp = ps_value(ps, node, ".", true);
        if (tmp != NULL)
        {
            node = ps_type(ps, tmp, IDENTIFIER, false);
        }

        tmp = ps_value(ps, node, "(", true);
        if (tmp != NULL)
        {
            node = ps_expression_list(ps, tmp);
            node = ps_value(ps, node, ")", false);
        }

        ps_add_elem(ps, TERM_END, NULL);
        return node;
    }

    // If no identifier/constant found, search for a parenthesized expression
    tmp = ps_value(ps, node, "(", true);
    if (tmp != NULL)
    {
        node = ps_expression(ps, tmp);
        node = ps_value(ps, node, ")", false);
        ps_add_elem(ps, TERM_END, NULL);
        return node;
    }

    // If none of the above is true, search for a unary operation
    node = ps_values(ps, node, (char[][TOKEN_MAX_LEN]){"-", "~"}, 2,
                     NULL, 0, true);
    node = ps_term(ps, node);

    if (!ps_remove_null(ps, node))
    {
        ps_add_elem(ps, TERM_END, NULL);
    }

    return node;
}

static const Node *ps_values(Parser *ps, const Node *node,
                             char values[][TOKEN_MAX_LEN], int num_vals,
                             TokenType types[], int num_types, bool optional)
{
    if (node == NULL)
    {
        return NULL;
    }

    ps_add_elem(ps, TOKEN, ((Token *)node->data));

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

    ps_remove_null(ps, NULL);
    return NULL;
}

static const Node *ps_value(Parser *ps, const Node *node, char *value,
                            bool optional)
{
    char val[1][TOKEN_MAX_LEN];
    strcpy(val[0], value);
    return ps_values(ps, node, val, 1, NULL, 0, optional);
}

static const Node *ps_type(Parser *ps, const Node *node, TokenType type,
                           bool optional)
{
    TokenType types[] = {type};
    return ps_values(ps, node, NULL, 0, types, 1, optional);
}

void ps_parse(Parser *ps, const Tokenizer *tk)
{
    ps_init(ps);

    // All Jack programs must begin with a class so start there.
    ps_class(ps, tk->tokens.start);

    /* Bit of a clumsy hack, but this is the simplest way to generate the
    end of class element since the last token has a NULL next and the parser
    interprets that as an error and never adds it as an element.
    */
    ps_add_elem(ps, TOKEN, ((Token *)tk->tokens.end->data));
    ps_add_elem(ps, CLASS_END, NULL);
}

void ps_free(Parser *ps)
{
    list_free(&ps->elements);
}

bool ps_gen_xml(Parser *ps, const char *filename)
{
    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Unable to generate XML file.\n");
        return false;
    }

    Node *node = ps->elements.start;
    if (node != NULL)
    {
        do
        {
            Element *elem = node->data;
            if (elem->type != TOKEN)
            {
                if (elem->type <= KEYWORD_CONST)
                {
                    fprintf(fp, "<%s>\n", ELEM_TYPES[elem->type]);
                }
                else
                {
                    fprintf(fp, "</%s>\n",
                            ELEM_TYPES[elem->type - KEYWORD_CONST]);
                }
            }
            else
            {
                fprintf(fp, "<%s> %s </%s>\n", TOKEN_TYPES[elem->token->type],
                        elem->token->value, TOKEN_TYPES[elem->token->type]);
            }
        } while ((node = node->next) != NULL);
    }

    fclose(fp);
    return true;
}
