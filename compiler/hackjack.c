#include <stdio.h>
#include <string.h>
#include "tokenizer.h"

bool tokenize(const char *filename, Tokenizer *tk)
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
        if (!tk_feed_buf(tk, c))
        {
            return false;
        }
    }

    fclose(fp);
    tk_flush_buf(tk);

    return true;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: ./hackjack <path-to-file>\n");
        return 1;
    }

    Tokenizer tk;
    if (tokenize(argv[1], &tk))
    {
        tk_gen_xml(&tk, "Test.xml");
        tk_free(&tk);
    }

    return 0;
}
