#include <stdio.h>
#include <string.h>
#include "tokenizer.h"

int main(void)
{
    Tokenizer tk;
    tk_init(&tk);

    const char input[] = "{()}\n";
    for (size_t i = 0; i < strlen(input); i++)
    {
        tk_feed_buf(&tk, input[i]);
        TokenType type = tk_get_buf_type(&tk);
        if (type != NONE)
        {
            tk_add_buf(&tk, type);
        }
    }

    tk_print(&tk);
    tk_gen_xml(&tk, "Test.xml");
    tk_free(&tk);
    return 0;
}
