#include <stdio.h>
#include <string.h>
#include "parser.h"
#include "tokenizer.h"

typedef enum
{
    WHILE,
    SYMBOL_RULE,
    STATEMENT,
    STATEMENTS,
    EXPRESSION
} RuleType;

typedef struct Rule
{
    RuleType type;
    char value[TOKEN_MAX_LEN];
} Rule;

static const Rule rule_while[] = {
    {WHILE, "while"},
    {SYMBOL_RULE, "("},
    {EXPRESSION, ""},
    {SYMBOL_RULE, ")"},
    {SYMBOL_RULE, "{"},
    {STATEMENTS, ""},
    {SYMBOL_RULE, "}"},
};

static Node *is_statement(const Node *node, bool single)
{
    (void)single;
    return node->next;
}

static Node *is_expression(const Node *node)
{
    return node->next;
}

static Node *parse_rule(const Rule *rule, const Rule *end, const Node *node)
{
    if ((node->next == NULL) || (rule == end))
    {
        return node->next;
    }

    Node *next = node->next;
    if (rule[0].type == STATEMENT)
    {
        if ((node = is_statement(node, true)) == NULL)
        {
            fprintf(stderr, "Expected a single statement.\n");
            return NULL;
        }
    }
    else if (rule[0].type == STATEMENTS)
    {
        if ((node = is_statement(node, false)) == NULL)
        {
            fprintf(stderr, "Expected at least one statement.\n");
            return NULL;
        }
    }
    else if (rule[0].type == EXPRESSION)
    {
        if ((node = is_expression(node)) == NULL)
        {
            fprintf(stderr, "Expected an expression.\n");
            return NULL;
        }
    }
    else if (strcmp(rule[0].value, node->token.value) != 0)
    {
        fprintf(stderr, "Expected '%s' but received '%s'.\n", rule[0].value,
                node->token.value);
        return NULL;
    }

    return parse_rule(rule + 1, end, next);
}

void parse(Tokenizer *tk)
{
    Node *node = tk->tokens.start;

    while (node != NULL)
    {
        if (strcmp(node->token.value, "while") == 0)
        {
            node = parse_rule(rule_while, rule_while + 6, node);
        }
        else
        {
            node = NULL;
        }
    }
}
