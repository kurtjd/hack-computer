#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include "tokenizer.h"
#include "parser.h"
#include "codegen.h"

#define MAX_FILES 32

char FILES[MAX_FILES][FILENAME_MAX] = {'\0'};
int NUM_FILES = 0;

// Gets a list of Jack files from a filepath
void get_files(const char *filepath)
{
    DIR *d;
    struct dirent *dir;
    d = opendir(filepath);

    /* If path is directory read in all the Jack files, otherwise treat path as
     * the file itself.
     */
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            char *fname = dir->d_name;
            int flen = strlen(fname);

            if (strcmp(fname + (flen - 5), ".jack") == 0)
            {
                sprintf(FILES[NUM_FILES++], "%s/%s", filepath, fname);
            }
        }

        closedir(d);
    }
    else
    {
        strcpy(FILES[0], filepath);
        NUM_FILES = 1;
    }
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: ./hackjack <path-to-file/folder>\n");
        return 1;
    }

    if (strlen(argv[1]) > FILENAME_MAX)
    {
        fprintf(stderr, "Filepath too long.\n");
        return 1;
    }

    get_files(argv[1]);

    // Compile each .jack file in directory
    for (int i = 0; i < NUM_FILES; i++)
    {
        FILE *fp = fopen(FILES[i], "r");
        if (fp == NULL)
        {
            fprintf(stderr, "Unable to open %s\n", FILES[i]);
            return false;
        }

        Tokenizer tk;
        if (tk_tokenize(&tk, FILES[i]))
        {
            Parser ps;
            if (ps_parse(&ps, &tk))
            {
                CodeGen cg;
                cg_generate(&cg, &ps);

                cg_gen_vm_file(FILES[i]);

                tk_free(&tk);
                ps_free(&ps);
                cg_free(&cg);

                fclose(fp);
            }
        }
    }

    return 0;
}
