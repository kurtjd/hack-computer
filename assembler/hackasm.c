// This is an assembler for the Hack computer.

#define MAX_LINE_LEN 128
#define LINE_CHUNK 128
#define PROGRAM_BUF_CHUNK (MAX_LINE_LEN * LINE_CHUNK)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

// Contains information about the loaded program such as contents and size.
typedef struct Program
{
    char *binary;
    size_t bin_size;
    char *assembly;
    size_t size;
    int lines;
} Program;
void program_init(Program *program)
{
    program->binary = NULL;
    program->bin_size = 0;
    program->assembly = calloc(PROGRAM_BUF_CHUNK, sizeof(char));
    program->size = PROGRAM_BUF_CHUNK * sizeof(char);
    program->lines = 0;
}

// Remove all whitespace from the line.
void trim_ws(char *str)
{
    char buf[MAX_LINE_LEN] = "";
    for (size_t i = 0; i < strlen(str); i++)
    {
        if (!isspace(str[i]))
        {
            buf[strlen(buf)] = str[i];
        }
    }
    strncpy(str, buf, strlen(buf) + 1);
}

// Remove all comments from the line.
void trim_comments(char *str)
{
    for (size_t i = 0; i < strlen(str); i++)
    {
        if (str[i] == '/' && str[i + 1] == '/')
        {
            str[i] = '\0';
            break;
        }
    }
}

/* Performs the first pass which includes removing whitespace and comments,
and building the symbol table. */
bool first_pass(char *filename, Program *program)
{
    // Open and read .asm file line-by-line
    char line[MAX_LINE_LEN];
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Unable to open assembly file.\n");
        return false;
    }

    while (fgets(line, MAX_LINE_LEN, fp) != NULL)
    {
        // Strip whitespace and comments from line
        trim_ws(line);
        trim_comments(line);

        // Disregard blank lines
        size_t line_len = strlen(line);
        if (line_len > 0)
        {
            // Check for symbol in line, add to symbol table
            // If label symbol, value is next line number.
            // If variable symbol, value is 16 + variables found so far

            // Add line to buffer.
            strncpy(program->assembly + (program->lines * MAX_LINE_LEN), line, line_len + 1);

            // Expand the program buffer every LINE_CHUNK number of lines.
            program->lines++;
            if (program->lines % LINE_CHUNK == 0)
            {
                program->size *= 2;
                program->assembly = realloc(program->assembly, program->size);
            }
        }
    }

    fclose(fp);
    return true;
}

/* Performs the second pass which includes converting symbols into numbers and
instructions into binary. */
void second_pass(Program *program)
{
    // Do second pass replacing each symbol in line with value in table
    // After replace symbol, break assembly into parts
    // Convert parts into binary
    // Concat binary to out string + \n
    for (int i = 0; i < program->lines; i++)
    {
        printf("%s\n", program->assembly + (i * MAX_LINE_LEN));
    }
}

/* Generates a .hack file containing the raw binary of converted assembly
program. */
bool gen_hack(char *filename, Program *program)
{
    FILE *fp = fopen(filename, "w");
    if (fp != NULL)
    {
        fwrite(program->binary, program->bin_size, 1, fp);
    }
    else
    {
        fprintf(stderr, "Unable to generate .hack file.\n");
        return false;
    }

    fclose(fp);
    return true;
}

// Frees up memory before exiting.
void clean_exit(Program *program, int status)
{
    if (program->assembly != NULL)
    {
        free(program->assembly);
    }

    exit(status);
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: ./hackasm <path-to-file>\n");
        return 1;
    }

    // Initialize container which holds the contents of the assembly file.
    Program program;
    program_init(&program);

    // Create symbol table
    // Add predefined constants to table

    // Perform first pass
    if (!first_pass(argv[1], &program))
    {
        clean_exit(&program, 1);
    }

    // Perform second pass
    second_pass(&program);

    // Write binary to .hack file
    /*if (!gen_hack("out.hack", &program))
    {
        clean_exit(&program, 1);
    }*/

    clean_exit(&program, 0);
}