#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "tokenizer.h"

#define NUM_KEYWORDS 21
#define NUM_SYMBOLS 19

static const char SYMBOLS[] = "{}()[].,;+-*/&|<>=~";
static const char KEYWORDS[][12] = {
    "class",
    "constructor",
    "function",
    "method",
    "field",
    "static",
    "var",
    "int",
    "char",
    "boolean",
    "void",
    "true",
    "false",
    "null",
    "this",
    "let",
    "do",
    "if",
    "else",
    "while",
    "return",
};
static const char TOKEN_TYPES[][16] = {
    "none",
    "keyword",
    "symbol",
    "integerConstant",
    "stringConstant",
    "identifier",
};

// Checks if a given string is a keyword
static bool is_keyword(const char *str)
{
    for (int i = 0; i < NUM_KEYWORDS; i++)
    {
        if (strcmp(KEYWORDS[i], str) == 0)
        {
            return true;
        }
    }

    return false;
}

// Checks if a given char is a symbol
static bool is_symbol(char c)
{
    for (int i = 0; i < NUM_SYMBOLS; i++)
    {
        if (SYMBOLS[i] == c)
        {
            return true;
        }
    }

    return false;
}

// Initializes a single token with given values
static void token_init(Token *token, TokenType type, const char *value)
{
    token->type = type;
    strcpy(token->value, value);
}

// Clears the token buffer
static void tk_clear_buf(Tokenizer *tk)
{
    for (int i = 0; i < TOKEN_MAX_LEN; i++)
    {
        tk->buf[i] = '\0';
    }
}

// Adds a new token
static bool tk_add(Tokenizer *tk, TokenType type, const char *value)
{
    Node *new_node = malloc(sizeof(*new_node));
    if (new_node == NULL)
    {
        fprintf(stderr, "Unable to allocate memory for new token.\n");
        return false;
    }

    // Initialize new node and token
    new_node->next = NULL;
    token_init(&new_node->token, type, value);

    // Insert the new node just after the previous end node
    Node *end_node = tk->tokens.end;
    if (end_node != NULL)
    {
        new_node->prev = end_node;
        end_node->next = new_node;
    }
    else
    {
        new_node->prev = NULL;
        tk->tokens.start = new_node;
    }

    tk->tokens.end = new_node;

    return true;
}

void tk_init(Tokenizer *tk)
{
    tk->tokens.start = NULL;
    tk->tokens.end = NULL;
    tk_clear_buf(tk);
}

void tk_free(Tokenizer *tk)
{
    Node *node = tk->tokens.start;
    if (node == NULL)
    {
        return;
    }

    Node *next;
    do
    {
        next = node->next;
        free(node);
        node = next;
    } while (next != NULL);
}

void tk_print(Tokenizer *tk)
{
    Node *node = tk->tokens.start;
    if (node == NULL)
    {
        return;
    }

    do
    {
        printf("Type: %d\nValue: %s\n\n", node->token.type, node->token.value);
    } while ((node = node->next) != NULL);
}

bool tk_feed_buf(Tokenizer *tk, char c)
{
    size_t buf_len = strlen(tk->buf);

    if (buf_len < (TOKEN_MAX_LEN - 1))
    {
        tk->buf[buf_len] = c;
        return true;
    }
    else
    {
        fprintf(stderr, "Token cannot be greater than %d characters.\n",
                TOKEN_MAX_LEN);
        return false;
    }
}

TokenType tk_get_buf_type(Tokenizer *tk)
{
    if (strlen(tk->buf) == 1 && is_symbol(tk->buf[0]))
    {
        return SYMBOL;
    }

    return NONE;
}

void tk_add_buf(Tokenizer *tk, TokenType type)
{
    tk_add(tk, type, tk->buf);
    tk_clear_buf(tk);
}

bool tk_gen_xml(Tokenizer *tk, const char *filename)
{
    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Unable to generate XML file.\n");
        return false;
    }

    fputs("<tokens>\n", fp);

    Node *node = tk->tokens.start;
    if (node != NULL)
    {
        do
        {
            const char *type = TOKEN_TYPES[node->token.type];
            fprintf(fp, "<%s> %s </%s>\n", type, node->token.value, type);
        } while ((node = node->next) != NULL);
    }

    fputs("</tokens>", fp);

    fclose(fp);
    return true;
}
