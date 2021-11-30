#ifndef PARSER_H
#define PARSER_H

#include "tokenizer.h"

#define ELEM_MAX_LEN 128

typedef enum
{
    TOKEN,

    CLASS,
    CLASS_VAR_DEC,
    TYPE,
    SUBROUTINE_DEC,
    PARAM_LIST,
    SUBROUTINE_BODY,
    VAR_DEC,
    SUBROUTINE_NAME,
    VAR_NAME,
    STATEMENTS,
    STATEMENT,
    LET_STATEMENT,
    IF_STATEMENT,
    WHILE_STATEMENT,
    DO_STATEMENT,
    RETURN_STATEMENT,
    EXPRESSION,
    TERM,
    SUBORUTINE_CALL,
    EXPRESSION_LIST,
    OP,
    UNARY_OP,
    KEYWORD_CONST,

    CLASS_END,
    CLASS_VAR_DEC_END,
    TYPE_END,
    SUBROUTINE_DEC_END,
    PARAM_LIST_END,
    SUBROUTINE_BODY_END,
    VAR_DEC_END,
    SUBROUTINE_NAME_END,
    VAR_NAME_END,
    STATEMENTS_END,
    STATEMENT_END,
    LET_STATEMENT_END,
    IF_STATEMENT_END,
    WHILE_STATEMENT_END,
    DO_STATEMENT_END,
    RETURN_STATEMENT_END,
    EXPRESSION_END,
    TERM_END,
    SUBORUTINE_CALL_END,
    EXPRESSION_LIST_END,
    OP_END,
    UNARY_OP_END,
    KEYWORD_CONST_END,
} ElementType;

typedef struct Element
{
    ElementType type;
    Token token;
} Element;

// Recursively parses tokens by following language rules
void ps_parse(const Tokenizer *tk);

#endif
