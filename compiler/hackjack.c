#include <stdio.h>
#include "tokenizer.h"

int main(void)
{
    TokenList tokens;
    tokens_init(&tokens);
    tokens_new(&tokens, KEYWORD, "void");
    tokens_new(&tokens, SYMBOL, "}");
    tokens_new(&tokens, INT_CONST, "123");
    tokens_print(&tokens);
    tokens_free(&tokens);
    return 0;
}