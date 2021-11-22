#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "stack.h"

#define ASM_MAX_LINE 32
#define ASM_LINE_BUF 128
#define ASM_CHUNK_SIZE (ASM_LINE_BUF * ASM_MAX_LINE)
#define ASM_SIZE (sizeof *(prog->data))

#define VM_MAX_LINE 128
#define VM_MAX_ARGS 3
#define VM_MAX_ARG_LEN 16
#define VM_ARG_DELIM " "
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
    TEMP_START_ADDR,
    R13_ADDR = 13,
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
    sprintf(prog->data + strlen(prog->data), "%s\n", line);
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

// Parse a 'push' instruction
bool parse_push(AsmProg *prog, char *asm_line, const char *arg1,
                const char *arg2)
{
    bool is_virtual = false;
    bool is_const = false;

    // Figure out which address to get data
    if (strcmp(arg1, "local") == 0)
    {
        asm_add_line(prog, "@LCL");
        is_virtual = true;
    }
    else if (strcmp(arg1, "argument") == 0)
    {
        asm_add_line(prog, "@ARG");
        is_virtual = true;
    }
    else if (strcmp(arg1, "this") == 0)
    {
        asm_add_line(prog, "@THIS");
        is_virtual = true;
    }
    else if (strcmp(arg1, "that") == 0)
    {
        asm_add_line(prog, "@THAT");
        is_virtual = true;
    }
    else if (strcmp(arg1, "pointer") == 0)
    {
        int pntr_val = atoi(arg2);
        if (pntr_val)
        {
            asm_add_line(prog, "@THAT");
        }
        else
        {
            asm_add_line(prog, "@THIS");
        }
    }
    else if (strcmp(arg1, "temp") == 0)
    {
        sprintf(asm_line, "@%d", TEMP_START_ADDR + atoi(arg2));
        asm_add_line(prog, asm_line);
    }
    else if (strcmp(arg1, "constant") == 0)
    {
        sprintf(asm_line, "@%d", atoi(arg2));
        asm_add_line(prog, asm_line);
        asm_add_line(prog, "D=A");
        is_const = true;
    }
    else if (strcmp(arg1, "static") == 0)
    {
        sprintf(asm_line, "@Static.%d", atoi(arg2));
        asm_add_line(prog, asm_line);
    }
    else
    {
        fprintf(stderr, "Unknown argument: %s\n", arg1);
        return false;
    }

    // Get the data stored at address
    if (!is_const)
    {
        asm_add_line(prog, "D=M");
        if (is_virtual)
        {
            sprintf(asm_line, "@%d", atoi(arg2));
            asm_add_line(prog, asm_line);
            asm_add_line(prog, "A=D+A");
            asm_add_line(prog, "D=M");
        }
    }

    // Push data onto stack
    asm_add_line(prog, "@SP");
    asm_add_line(prog, "M=M+1");
    asm_add_line(prog, "A=M-1");
    asm_add_line(prog, "M=D");

    return true;
}

// Parse a 'pop' instruction
bool parse_pop(AsmProg *prog, char *asm_line, const char *arg1,
               const char *arg2)
{
    bool is_virtual = false;

    // Figure out which address to store data
    if (strcmp(arg1, "local") == 0)
    {
        sprintf(asm_line, "@LCL");
        is_virtual = true;
    }
    else if (strcmp(arg1, "argument") == 0)
    {
        sprintf(asm_line, "@ARG");
        is_virtual = true;
    }
    else if (strcmp(arg1, "this") == 0)
    {
        sprintf(asm_line, "@THIS");
        is_virtual = true;
    }
    else if (strcmp(arg1, "that") == 0)
    {
        sprintf(asm_line, "@ADD");
        is_virtual = true;
    }
    else if (strcmp(arg1, "pointer") == 0)
    {
        int pntr_val = atoi(arg2);
        if (pntr_val)
        {
            sprintf(asm_line, "@THAT");
        }
        else
        {
            sprintf(asm_line, "@THIS");
        }
    }
    else if (strcmp(arg1, "temp") == 0)
    {
        sprintf(asm_line, "@%d", TEMP_START_ADDR + atoi(arg2));
    }
    else if (strcmp(arg1, "static") == 0)
    {
        sprintf(asm_line, "@Static.%d", atoi(arg2));
    }
    else
    {
        fprintf(stderr, "Unknown argument: %s\n", arg1);
        return false;
    }

    // Pop stack value
    asm_add_line(prog, "@SP");
    asm_add_line(prog, "M=M-1");
    asm_add_line(prog, "A=M");
    asm_add_line(prog, "D=M");

    // Get the address
    if (is_virtual)
    {
        asm_add_line(prog, "@R13");
        asm_add_line(prog, "M=D");
        asm_add_line(prog, asm_line);

        asm_add_line(prog, "D=M");
        sprintf(asm_line, "@%d", atoi(arg2));
        asm_add_line(prog, asm_line);
        asm_add_line(prog, "D=D+A");

        asm_add_line(prog, "@R14");
        asm_add_line(prog, "M=D");

        asm_add_line(prog, "@R13");
        asm_add_line(prog, "D=M");

        asm_add_line(prog, "@R14");
        asm_add_line(prog, "A=M");
    }
    else
    {
        asm_add_line(prog, asm_line);
    }

    // Store the data
    asm_add_line(prog, "M=D");

    return true;
}

// Parses a line of VM code into Assembly code
bool parse(char *line, AsmProg *prog)
{
    /* The 'arguments' of a line (the instruction itself plus additional
     * arguments)
     */
    char args[VM_MAX_ARGS][VM_MAX_ARG_LEN] = {'\0'};
    char asm_line[ASM_MAX_LINE];
    char temp_line[VM_MAX_LINE];

    // Split the line into arguments based on a delimeter
    strcpy(temp_line, line);
    char *token = strtok(temp_line, VM_ARG_DELIM);
    int i = 0;
    while (token != NULL && i < VM_MAX_ARGS)
    {
        strcpy(args[i++], token);
        token = strtok(NULL, VM_ARG_DELIM);
    }

    // Add a comment to the asm notating the VM instruction
    sprintf(asm_line, "// %s", line);
    asm_add_line(prog, asm_line);

    // Handle push
    if (strcmp(args[0], "push") == 0)
    {
        parse_push(prog, asm_line, args[1], args[2]);
    }

    // Handle pop
    else if (strcmp(args[0], "pop") == 0)
    {
        parse_pop(prog, asm_line, args[1], args[2]);
    }

    return true;
}

// Opens a VM file and translates it into Hack assembly code
bool translate(const char *filename, AsmProg *prog)
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
        // Strip comments and \n from line
        trim_comments(line);
        line[strlen(line) - 1] = '\0';

        // Disregard blank lines
        if (!line_is_empty(line))
        {
            parse(line, prog);
        }
    }

    fclose(fp);
    return true;
}

// Frees memory and exits
void clean_exit(AsmProg *prog)
{
    asm_free(prog);
    exit(0);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: ./hackvm <path-to-file>\n");
        return 1;
    }

    if (strlen(argv[1]) > FILENAME_MAX)
    {
        fprintf(stderr, "Filename too large.\n");
        return 1;
    }

    AsmProg prog;
    asm_init(&prog);

    if (!translate(argv[1], &prog))
    {
        clean_exit(&prog);
        return 1;
    }

    printf("%s\n", prog.data);

    clean_exit(&prog);
    return 0;
}
