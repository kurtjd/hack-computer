#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "tokenizer.h"

#define NUM_KEYWORDS 21

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
    if (strchr(SYMBOLS, c))
    {
        return true;
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

    tk->possible = NONE;
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

/* Returns the token type of the token buffer
 * A return of NONE means the buffer is not yet a valid token
 */
static TokenType tk_get_buf_type(Tokenizer *tk, char c)
{
    size_t buf_len = strlen(tk->buf);

    /* With the previous token saved, we begin looking for a new one.
     * Only a single character symbol (with the exception of a /,
     * since it could be a comment) can be a token now, anything else is just
     * a possible token.
     */
    if (buf_len == 0)
    {
        if (is_symbol(c) && c != '/')
        {
            return SYMBOL;
        }
        else if (isdigit(c))
        {
            tk->possible = INT_CONST;
        }
        else if (c == '"')
        {
            tk->possible = STR_CONST;
        }
        else if (c == '/')
        {
            tk->possible = COMMENT;
        }

        return NONE;
    }

    /* Depending on which token we've possibly found,
     * check for conditions that signify it is actually a token.
     */
    switch (tk->possible)
    {
    case NONE:
        if (!isalnum(c))
        {
            if (is_keyword(tk->buf))
            {
                return KEYWORD;
            }
            else
            {
                return IDENTIFIER;
            }
        }
        break;

    case INT_CONST:
        if (!isdigit(c))
        {
            return INT_CONST;
        }
        break;

    case STR_CONST:
        if (c == '"')
        {
            return STR_CONST;
        }
        break;

    case COMMENT:
        if (c == '/')
        {
            tk->in_comment = 1;
            tk_clear_buf(tk);
        }
        else if (c == '*')
        {
            tk->in_comment = 2;
            tk_clear_buf(tk);
        }
        else
        {
            return SYMBOL;
        }

        break;

    default:
        break;
    }

    return NONE;
}

// Convert the token buffer into a token and add it to list
static bool tk_add_buf(Tokenizer *tk, TokenType type)
{
    bool success = tk_add(tk, type, tk->buf);
    tk_clear_buf(tk);
    return success;
}

void tk_init(Tokenizer *tk)
{
    tk->in_comment = 0;
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

bool tk_feed_buf(Tokenizer *tk, char c)
{
    /* If we are in a comment, check to see if next character takes us out
     * but don't do anything else.
     */
    if (tk->in_comment)
    {
        if ((tk->in_comment == 1) && (c == '\n'))
        {
            tk->in_comment = 0;
        }
        else if (tk->in_comment == 2 && c == '*')
        {
            tk->in_comment++;
        }
        else if (tk->in_comment == 3 && c == '/')
        {
            tk->in_comment = 0;
        }
        else if (tk->in_comment == 3 && c != '/')
        {
            tk->in_comment = 2;
        }

        return true;
    }

    /* If we found a token, add it to list before updating buf
     * with next character.
     */
    TokenType type = tk_get_buf_type(tk, c);
    if (type != NONE && type != SYMBOL)
    {
        if (!tk_add_buf(tk, type))
        {
            return false;
        }
    }

    size_t buf_len = strlen(tk->buf);
    if (buf_len < (TOKEN_MAX_LEN - 1))
    {
        /* If we are not in a comment, and the next character is not a space
         * (and we're not in a string) or a quote, add it to buf
         */
        if ((tk->possible == STR_CONST || !isspace(c)) && c != '"' &&
            !tk->in_comment)
        {
            tk->buf[buf_len] = c;
        }

        // If current or next characters are symbols (and not a /), add them now
        if (type == SYMBOL)
        {
            if (!tk_add_buf(tk, type))
            {
                return false;
            }
        }
        else if (is_symbol(c) && c != '/' && !tk->in_comment)
        {
            if (!tk_add_buf(tk, SYMBOL))
            {
                return false;
            }
        }

        return true;
    }
    else
    {
        fprintf(stderr, "Token cannot be greater than %d characters.\n",
                TOKEN_MAX_LEN);
        return false;
    }
}

void tk_flush_buf(Tokenizer *tk)
{
    if (tk->possible != NONE)
    {
        tk_add_buf(tk, tk->possible);
    }
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
