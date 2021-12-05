#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdbool.h>
#include "linkedlist.h"

#define TOKEN_MAX_LEN 64

typedef enum
{
    NONE,
    KEYWORD,
    SYMBOL,
    INT_CONST,
    STR_CONST,
    IDENTIFIER,
    COMMENT
} TokenType;

typedef struct Token
{
    TokenType type;
    char value[TOKEN_MAX_LEN];
} Token;

typedef struct Tokenizer
{
    char buf[TOKEN_MAX_LEN];
    TokenType possible;
    int in_comment;
    LinkedList tokens;
} Tokenizer;

extern const char TOKEN_TYPES[6][16];

// Generates tokens from an input file
bool tk_tokenize(Tokenizer *tk, const char *filename);

// Frees all the tokens
void tk_free(Tokenizer *tk);

// Generates an XML file containing the token data
bool tk_gen_xml(Tokenizer *tk, const char *filename);

#endif
