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

// Initializes the tokenizer
void tk_init(Tokenizer *tk);

// Frees all the tokens
void tk_free(Tokenizer *tk);

// Adds a character to the token buffer
bool tk_feed_buf(Tokenizer *tk, char c);

// Creates a token from the whatever is in the buffer and its possible type
void tk_flush_buf(Tokenizer *tk);

// Generates an XML file containing the token data
bool tk_gen_xml(Tokenizer *tk, const char *filename);
