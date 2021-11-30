#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "tokenizer.h"

#define NUM_KEYWORDS 21

/*************************************************************
 *   PRIVATE                                                 *
 *************************************************************/
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

// Checks if a given string is a keyword
static bool tk_is_keyword(const char *str)
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
static bool tk_is_symbol(char c)
{
    if (strchr(SYMBOLS, c))
    {
        return true;
    }

    return false;
}

// Initializes a single token with given values
static void tk_token_init(Token *token, TokenType type, const char *value)
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
    Token tok;
    tk_token_init(&tok, type, value);

    return list_add(&tk->tokens, &tok) != NULL ? true : false;
}

/* Returns the token type of the token buffer
 * A return of NONE means the buffer is not yet a valid token
 * Also determines if the tokenizer should enter the comment state
 */
static TokenType tk_get_buf_type(Tokenizer *tk, char c)
{
    /* With the previous token saved, we begin looking for a new one.
     * Only a single character symbol (with the exception of a /,
     * since it could be a comment) can be a token now, anything else is just
     * a possible token.
     */
    if (strlen(tk->buf) == 0)
    {
        if (tk_is_symbol(c) && c != '/')
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
     * check for conditions that signify the end of the token.
     */
    switch (tk->possible)
    {
    case NONE:
        if (!isalnum(c))
        {
            if (tk_is_keyword(tk->buf))
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
        }
        else if (c == '*')
        {
            tk->in_comment = 2;
        }
        else
        {
            return SYMBOL;
        }

        break;

    default:
        break;
    }

    // If just entered a comment, clear the / that was previously in the buffer
    if (tk->in_comment > 0)
    {
        tk_clear_buf(tk);
    }

    return NONE;
}

// Convert the token buffer into a token and add it to list
static bool tk_add_from_buf(Tokenizer *tk, TokenType type)
{
    bool success = tk_add(tk, type, tk->buf);
    tk_clear_buf(tk);
    return success;
}

// Updates tokenizer comment state and returns true if still in comment
static bool tk_handle_comment(Tokenizer *tk, char c)
{
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

    return false;
}

// Feeds a character to the tokenizer for it to handle
static bool tk_feed(Tokenizer *tk, char c)
{
    // Don't feed the character if still in a comment
    if (tk_handle_comment(tk, c))
    {
        return true;
    }

    TokenType type = tk_get_buf_type(tk, c);
    bool success = true;

    /* If the current buffer is a token, add it to token list before adding
     * the next character to the buffer.
     */
    if (type != NONE && type != SYMBOL)
    {
        success = tk_add_from_buf(tk, type);
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
    }
    else
    {
        fprintf(stderr, "Token cannot be greater than %d characters.\n",
                TOKEN_MAX_LEN);
        return false;
    }

    // If current or next characters are symbols (and not a /), add them now
    if (type == SYMBOL)
    {
        success = tk_add_from_buf(tk, type);
    }
    else if (tk_is_symbol(c) && c != '/' && !tk->in_comment)
    {
        success = tk_add_from_buf(tk, SYMBOL);
    }

    return success;
}

// Creates a token from the whatever is in the buffer and its possible type
static bool tk_flush_buf(Tokenizer *tk)
{
    TokenType type;
    bool found_tok = false;

    if (tk->possible != NONE)
    {
        type = tk->possible;
        found_tok = true;
    }
    else if (tk_is_keyword(tk->buf))
    {
        type = KEYWORD;
        found_tok = true;
    }
    else if (strlen(tk->buf) > 0)
    {
        type = IDENTIFIER;
        found_tok = true;
    }

    return found_tok ? tk_add_from_buf(tk, type) : true;
}

// Initializes the tokenizer
static void tk_init(Tokenizer *tk)
{
    list_init(&tk->tokens, sizeof(Token));
    tk->in_comment = 0;
    tk_clear_buf(tk);
}

/*************************************************************
 *   PUBLIC                                                  *
 *************************************************************/
const char TOKEN_TYPES[6][16] = {
    "none",
    "keyword",
    "symbol",
    "integerConstant",
    "stringConstant",
    "identifier",
};

bool tk_tokenize(Tokenizer *tk, const char *filename)
{
    tk_init(tk);

    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Unable to open %s\n", filename);
        return false;
    }

    char c;
    while ((c = fgetc(fp)) != EOF)
    {
        if (!tk_feed(tk, c))
        {
            return false;
        }
    }

    fclose(fp);

    return tk_flush_buf(tk);
}

void tk_free(Tokenizer *tk)
{
    list_free(&tk->tokens);
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
            Token *tok = node->data;
            const char *type = TOKEN_TYPES[tok->type];
            fprintf(fp, "<%s> %s </%s>\n", type, tok->value, type);
        } while ((node = node->next) != NULL);
    }

    fputs("</tokens>", fp);

    fclose(fp);
    return true;
}
