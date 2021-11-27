#include <stdbool.h>

#define TOKEN_MAX_LEN 64

typedef enum
{
    NONE,
    KEYWORD,
    SYMBOL,
    INT_CONST,
    STR_CONST,
    IDENTIFIER
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
    TokenList tokens;
} Tokenizer;

// Initializes the tokenizer
void tk_init(Tokenizer *tk);

// Frees all the tokens
void tk_free(Tokenizer *tk);

// Prints all the tokens
void tk_print(Tokenizer *tk);

// Adds a character to the token buffer
bool tk_feed_buf(Tokenizer *tk, char c);

/* Returns the token type of the token buffer
 * A return of NONE means the buffer is not yet a valid token
 */
TokenType tk_get_buf_type(Tokenizer *tk);

// Convert the token buffer into a token and add it to list
void tk_add_buf(Tokenizer *tk, TokenType type);

// Generates an XML file containing the token data
bool tk_gen_xml(Tokenizer *tk, const char *filename);
