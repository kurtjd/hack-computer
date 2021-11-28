#include <stdbool.h>

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

typedef struct Node
{
    Token token;
    struct Node *next;
    struct Node *prev;
} Node;

typedef struct TokenList
{
    Node *start;
    Node *end;
} TokenList;

typedef struct Tokenizer
{
    char buf[TOKEN_MAX_LEN];
    TokenType possible;
    int in_comment;
    TokenList tokens;
} Tokenizer;

// Generates tokens from an input file
bool tk_tokenize(Tokenizer *tk, const char *filename);

// Frees all the tokens
void tk_free(Tokenizer *tk);

// Generates an XML file containing the token data
bool tk_gen_xml(Tokenizer *tk, const char *filename);
