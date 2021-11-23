// TODO: Replace Static with filename
// TODO: Allow translate multiple VM files
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>

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

// Add a line of assembly
void asm_add_line(AsmProg *prog, const char *format, ...)
{
    char line[ASM_MAX_LINE];
    sprintf(line, "%s\n", format);

    // Allow for variable arguments making it easier to compose asm instructions
    va_list args;
    va_start(args, format);
    vsprintf(prog->data + strlen(prog->data), line, args);
    va_end(args);

    prog->line_count++;

    if ((prog->line_count % ASM_LINE_BUF) == 0)
    {
        prog->data = realloc(prog->data, ASM_SIZE + ASM_CHUNK_SIZE);
        if (prog->data == NULL)
        {
            fprintf(stderr, "Unable to reallocate memory for assembly.\n");
            exit(1);
        }
    }
}

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

    // Add code to initialize stack
    asm_add_line(prog, "// Initialize stack");
    asm_add_line(prog, "@%d", STACK_START_ADDR);
    asm_add_line(prog, "D=A");
    asm_add_line(prog, "@SP");
    asm_add_line(prog, "M=D");
    asm_add_line(prog, "");

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

// Adds instructions to push to stack
void asm_push_stack(AsmProg *prog)
{
    asm_add_line(prog, "@SP");
    asm_add_line(prog, "M=M+1");
    asm_add_line(prog, "A=M-1");
    asm_add_line(prog, "M=D");
}

// Adds instructions to pop from stack
void asm_pop_stack(AsmProg *prog)
{
    asm_add_line(prog, "@SP");
    asm_add_line(prog, "M=M-1");
    asm_add_line(prog, "A=M");
    asm_add_line(prog, "D=M");
}

// Adds instructions to pop two variables from stack
void asm_pop_two(AsmProg *prog)
{
    asm_pop_stack(prog);
    asm_add_line(prog, "@R13");
    asm_add_line(prog, "M=D");
    asm_pop_stack(prog);
    asm_add_line(prog, "@R13");
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

// Remove newline characters from the line.
void trim_nl(char *str)
{
    char buf[VM_MAX_LINE] = "";
    for (size_t i = 0; i < strlen(str); i++)
    {
        if (str[i] != '\n' && str[i] != '\r')
        {
            buf[strlen(buf)] = str[i];
        }
    }
    strcpy(str, buf);
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
bool parse_push(AsmProg *prog, const char *arg1, const char *arg2)
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
        asm_add_line(prog, "@%d", TEMP_START_ADDR + atoi(arg2));
    }
    else if (strcmp(arg1, "constant") == 0)
    {
        asm_add_line(prog, "@%d", atoi(arg2));
        asm_add_line(prog, "D=A");
        is_const = true;
    }
    else if (strcmp(arg1, "static") == 0)
    {
        asm_add_line(prog, "@Static.%d", atoi(arg2));
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
            asm_add_line(prog, "@%d", atoi(arg2));
            asm_add_line(prog, "A=D+A");
            asm_add_line(prog, "D=M");
        }
    }

    // Push data onto stack
    asm_push_stack(prog);

    return true;
}

// Parse a 'pop' instruction
bool parse_pop(AsmProg *prog, const char *arg1, const char *arg2)
{
    char asm_line[ASM_MAX_LINE];
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
        sprintf(asm_line, "@THAT");
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
    asm_pop_stack(prog);

    // Get the address
    if (is_virtual)
    {
        asm_add_line(prog, "@R13");
        asm_add_line(prog, "M=D");
        asm_add_line(prog, asm_line);

        asm_add_line(prog, "D=M");
        asm_add_line(prog, "@%d", atoi(arg2));
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

// Parse a comparison instruction
void parse_cmp(AsmProg *prog, const char *cmp, int count)
{
    asm_pop_two(prog);
    asm_add_line(prog, "D=D-M");

    asm_add_line(prog, "@%s_%d", cmp, count);
    asm_add_line(prog, "D;J%s", cmp);

    asm_add_line(prog, "D=0");
    asm_add_line(prog, "@END_%s_%d", cmp, count);
    asm_add_line(prog, "0;JMP");

    asm_add_line(prog, "(%s_%d)", cmp, count);
    asm_add_line(prog, "D=-1");

    asm_add_line(prog, "(END_%s_%d)", cmp, count);
    asm_push_stack(prog);
}

// Parse a unary (one operand) instruction
void parse_unary(AsmProg *prog, const char *instr)
{
    asm_pop_stack(prog);
    asm_add_line(prog, instr);
    asm_push_stack(prog);
}

// Parse a binary (two operand) instruction
void parse_binary(AsmProg *prog, const char *instr)
{
    asm_pop_two(prog);
    asm_add_line(prog, instr);
    asm_push_stack(prog);
}

// Parse a label instruction
void parse_label(AsmProg *prog, const char *label)
{
    asm_add_line(prog, "(%s)", label);
}

// Parse a goto instruction
void parse_goto(AsmProg *prog, const char *label)
{
    asm_add_line(prog, "@%s", label);
    asm_add_line(prog, "0;JMP");
}

// Parse an if-goto instruction
void parse_if_goto(AsmProg *prog, const char *label)
{
    asm_pop_stack(prog);
    asm_add_line(prog, "@%s", label);
    asm_add_line(prog, "D;JNE");
}

// Parse a function creation instruction
void parse_function(AsmProg *prog, const char *func_name, const char *nvars)
{
    asm_add_line(prog, "(%s)", func_name);
}

// Parse a function call instruction
void parse_call(AsmProg *prog, const char *func_name, const char *nargs)
{
    (void)prog;
    (void)func_name;
    (void)nargs;
}

// Parse a function return instruction
void parse_return(AsmProg *prog)
{
    (void)prog;
}

// Parses a line of VM code into Assembly code
bool parse(char *line, AsmProg *prog)
{
    /* The 'arguments' of a line (the instruction itself plus additional
     * arguments)
     */
    char args[VM_MAX_ARGS][VM_MAX_ARG_LEN] = {'\0'};
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
    asm_add_line(prog, "// %s", line);

    // Convert VM instructions into assembly
    static int eq_count = 0;
    static int gt_count = 0;
    static int lt_count = 0;

    // Memory
    if (strcmp(args[0], "push") == 0)
    {
        parse_push(prog, args[1], args[2]);
    }
    else if (strcmp(args[0], "pop") == 0)
    {
        parse_pop(prog, args[1], args[2]);
    }

    // Arithmetic
    else if (strcmp(args[0], "add") == 0)
    {
        parse_binary(prog, "D=D+M");
    }
    else if (strcmp(args[0], "sub") == 0)
    {
        parse_binary(prog, "D=D-M");
    }
    else if (strcmp(args[0], "neg") == 0)
    {
        parse_unary(prog, "D=-D");
    }

    // Comparison
    else if (strcmp(args[0], "eq") == 0)
    {
        parse_cmp(prog, "EQ", eq_count++);
    }
    else if (strcmp(args[0], "gt") == 0)
    {
        parse_cmp(prog, "GT", gt_count++);
    }
    else if (strcmp(args[0], "lt") == 0)
    {
        parse_cmp(prog, "LT", lt_count++);
    }

    // Logical
    else if (strcmp(args[0], "and") == 0)
    {
        parse_binary(prog, "D=D&M");
    }
    else if (strcmp(args[0], "or") == 0)
    {
        parse_binary(prog, "D=D|M");
    }
    else if (strcmp(args[0], "not") == 0)
    {
        parse_unary(prog, "D=!D");
    }

    // Branching
    else if (strcmp(args[0], "label") == 0)
    {
        parse_label(prog, args[1]);
    }
    else if (strcmp(args[0], "goto") == 0)
    {
        parse_goto(prog, args[1]);
    }
    else if (strcmp(args[0], "if-goto") == 0)
    {
        parse_if_goto(prog, args[1]);
    }

    // Functions
    else if (strcmp(args[0], "function") == 0)
    {
        parse_function(prog, args[1], args[2]);
    }
    else if (strcmp(args[0], "call") == 0)
    {
        parse_call(prog, args[1], args[2]);
    }
    else if (strcmp(args[0], "return") == 0)
    {
        parse_return(prog);
    }
    else
    {
        fprintf(stderr, "Unrecognized instruction.\n");
        return false;
    }

    // Add blank line for readability
    asm_add_line(prog, "");
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

    char line[VM_MAX_LINE] = {'\0'};
    while (fgets(line, VM_MAX_LINE, fp) != NULL)
    {
        // Strip comments and newline characters
        trim_comments(line);
        trim_nl(line);

        // Disregard blank lines
        if (!line_is_empty(line))
        {
            if (!parse(line, prog))
            {
                return false;
            }
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

// Writes assembly program to file
bool gen_asm_file(AsmProg *prog)
{
    FILE *fp = fopen("out.asm", "w");
    if (fp == NULL)
    {
        fprintf(stderr, "Unable to generate assembly file.\n");
        return false;
    }

    fwrite(prog->data, strlen(prog->data), 1, fp);

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

    if (!gen_asm_file(&prog))
    {
        clean_exit(&prog);
    }

    clean_exit(&prog);
    return 0;
}
