#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "tokenizer.h"

void token_init(Token *token, TokenType type, const char *value)
{
    token->type = type;
    strcpy(token->value, value);
}

void tokens_init(TokenList *tokens)
{
    tokens->start = NULL;
    tokens->end = NULL;
}

void tokens_free(TokenList *tokens)
{
    Node *node = tokens->start;
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

bool tokens_new(TokenList *tokens, TokenType type, const char *value)
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
    Node *end_node = tokens->end;
    if (end_node != NULL)
    {
        new_node->prev = end_node;
        end_node->next = new_node;
    }
    else
    {
        new_node->prev = NULL;
        tokens->start = new_node;
    }

    tokens->end = new_node;

    return true;
}

void tokens_print(TokenList *tokens)
{
    Node *node = tokens->start;
    if (node == NULL)
    {
        return;
    }

    do
    {
        printf("Type: %d\nValue: %s\n\n", node->token.type, node->token.value);
    } while ((node = node->next) != NULL);
}
