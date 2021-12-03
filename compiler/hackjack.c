#include <stdio.h>
#include "tokenizer.h"
#include "parser.h"
#include "codegen.h"

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

    CodeGen cg;
    cg_generate(&cg, &ps);
    cg_print_symtbl(&cg.cls_symbols);

    tk_free(&tk);
    ps_free(&ps);
    cg_free(&cg);

    return 0;
}
