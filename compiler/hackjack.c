#include <stdio.h>
#include "tokenizer.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: ./hackjack <path-to-file>\n");
        return 1;
    }

    Tokenizer tk;
    if (tk_tokenize(&tk, argv[1]))
    {
        tk_gen_xml(&tk, "Test.xml");
    }

    tk_free(&tk);
    return 0;
}
