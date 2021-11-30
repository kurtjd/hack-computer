#include <stdio.h>
#include "tokenizer.h"
#include "parser.h"

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
        tk_gen_xml(&tk, "Tokens.xml");
    }

    Parser ps;
    ps_parse(&ps, &tk);
    ps_gen_xml(&ps, "Elements.xml");

    tk_free(&tk);
    ps_free(&ps);

    return 0;
}
