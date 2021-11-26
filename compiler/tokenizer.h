#include <stdbool.h>

#define TOKEN_MAX_LEN 64

typedef enum
{
    KEYWORD,
    SYMBOL,
    IDENTIFIER,
    INT_CONST,
    STR_CONST
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

// Initializes a single token with given values
void token_init(Token *token, TokenType type, const char *value);

// Initializes the token linked list
void tokens_init(TokenList *tokens);

// Frees all the tokens/nodes in the linked list
void tokens_free(TokenList *tokens);

// Adds a new token/node to the linked list
bool tokens_new(TokenList *tokens, TokenType type, const char *value);

// Prints all the tokens in the linked list
void tokens_print(TokenList *tokens);
