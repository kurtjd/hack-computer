#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "stack.h"

#define ASM_MAX_LINE 10
#define ASM_LINE_BUF 128
#define ASM_CHUNK_SIZE (ASM_LINE_BUF * ASM_MAX_LINE)
#define ASM_SIZE (sizeof *(prog->data))

#define VM_MAX_LINE 128
#define VM_TRUE -1
#define VM_FALSE 0

// Mapping of VM symbols to RAM addresses on the Hack computer
typedef enum
{
    SP_ADDR,
    LCL_ADDR,
    ARG_ADDR,
    THIS_ADDR,
    THAT_ADDR,
    TEMP1_ADDR,
    TEMP2_ADDR,
    TEMP3_ADDR,
    TEMP4_ADDR,
    TEMP5_ADDR,
    TEMP6_ADDR,
    TEMP7_ADDR,
    TEMP8_ADDR,
    R13_ADDR,
    R14_ADDR,
    R15_ADDR,
    STATIC_START_ADDR,
    STACK_START_ADDR = 256
} VM_ADDR;

// Holds the translated assembly code
typedef struct AsmProg
{
    char *data;
    int line_count;
} AsmProg;

// Initialize the assembly data
bool asm_init(AsmProg *prog)
{
    prog->data = malloc(ASM_CHUNK_SIZE);
    if (prog->data == NULL)
    {
        fprintf(stderr, "Unable to allocate memory for assembly.\n");
        return false;
    }

    *prog->data = '\0';
    prog->line_count = 0;
    return true;
}

// Add a line of assembly
bool asm_add_line(AsmProg *prog, const char *line)
{
    strcpy(prog->data + strlen(prog->data), line);
    prog->line_count++;

    if ((prog->line_count % ASM_LINE_BUF) == 0)
    {
        prog->data = realloc(prog->data, ASM_SIZE + ASM_CHUNK_SIZE);
        if (prog->data == NULL)
        {
            fprintf(stderr, "Unable to reallocate memory for assembly.\n");
            return false;
        }
    }

    return true;
}

// Free the assembly data
void asm_free(AsmProg *prog)
{
    if (prog->data != NULL)
    {
        free(prog->data);
    }

    prog->data = NULL;
}

// Remove all comments from the line
void trim_comments(char *line)
{
    for (size_t i = 0; i < strlen(line); i++)
    {
        if (line[i] == '/' && line[i + 1] == '/')
        {
            line[i] = '\0';
            break;
        }
    }
}

// Check if line is empty
bool line_is_empty(const char *line)
{
    while (*line != '\0')
    {
        if (!isspace(*line))
        {
            return false;
        }

        line++;
    }

    return true;
}

bool translate(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Unable to open %s\n", filename);
        return false;
    }

    char line[VM_MAX_LINE];
    while (fgets(line, VM_MAX_LINE, fp) != NULL)
    {
        // Strip comments from line
        trim_comments(line);

        // Disregard blank lines
        if (!line_is_empty(line))
        {
            printf("%s", line);
        }
    }

    fclose(fp);
    return true;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: ./hackvm <path-to-file>\n");
        return 1;
    }
    else
    {
        if (strlen(argv[1]) > FILENAME_MAX)
        {
            fprintf(stderr, "Filename too large.\n");
            return 1;
        }

        if (!translate(argv[1]))
        {
            return 1;
        }
    }

    return 0;
}
