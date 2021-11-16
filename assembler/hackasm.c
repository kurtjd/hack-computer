// This is an assembler for the Hack computer.

#define MAX_LINE_LEN 128
#define LINE_CHUNK 128
#define PROGRAM_BUF_CHUNK (MAX_LINE_LEN * LINE_CHUNK)
#define INSTR_BITS 16

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>

// Contains information about the loaded program such as contents and size.
typedef struct Program
{
    char *binary;
    char *assembly;
    size_t size;
    int lines;
} Program;
void program_init(Program *program)
{
    program->assembly = calloc(PROGRAM_BUF_CHUNK, sizeof(char));
    program->size = PROGRAM_BUF_CHUNK * sizeof(char);
    program->lines = 0;
}

// Frees up memory before exiting.
void clean_exit(Program *program, int status)
{
    if (program->assembly != NULL)
    {
        free(program->assembly);
    }
    if (program->binary != NULL)
    {
        free(program->binary);
    }

    exit(status);
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
                program->size += (LINE_CHUNK * MAX_LINE_LEN);
                program->assembly = realloc(program->assembly, program->size);
            }
        }
    }

    fclose(fp);

    // Create space to store the binary conversion of program
    program->binary = calloc(program->size, sizeof(char));

    return true;
}

/* Performs the second pass which includes converting symbols into numbers and
instructions into binary. */
void second_pass(Program *program)
{
    for (int i = 0; i < program->lines; i++)
    {
        // Replace each symbol in line with value in table
        // TODO

        // Get the line of assembly code
        char line[MAX_LINE_LEN];
        strncpy(line, program->assembly + (i * MAX_LINE_LEN), MAX_LINE_LEN);

        char instruction[INSTR_BITS + 2] = "0000000000000000\n";

        // Handle A instruction (which all begin with '@')
        if (line[0] == '@')
        {
            uint16_t addr = atoi(line + 1);
            for (int b = 0; b < (INSTR_BITS - 1); b++)
            {
                /* Get the least significant bit of address, convert it to
                ASCII, and store it in respective instruction bit. */
                instruction[(INSTR_BITS - 1) - b] = ((addr >> b) & 0x0001) + '0';
            }
        }

        // Handle C instruction
        else
        {
            // Set first 3 bits to 111
            memcpy(instruction, "111", 3);

            // Store the three parts of a Hack assembly instruction
            char dest[4] = "";
            char comp[4] = "";
            char jump[4] = "";

            char *dest_pntr = strchr(line, '=');
            char *jump_pntr = strchr(line, ';');

            int comp_bits = 7;
            int dest_bits = 3;
            int jump_bits = 3;

            char *comp_start = instruction + 3;
            char *dest_start = instruction + 3 + comp_bits;
            char *jump_start = instruction + 3 + comp_bits + dest_bits;

            // Get the destination
            if (dest_pntr != NULL)
            {
                strncpy(dest, line, dest_pntr - line);
            }

            // Get the jump
            if (jump_pntr != NULL)
            {
                strncpy(jump, jump_pntr + 1, 3);
            }

            // Get the comp
            if (dest_pntr == NULL && jump_pntr == NULL)
            {
                strncpy(comp, line, strlen(line));
            }
            else if (dest_pntr == NULL)
            {
                strncpy(comp, line, jump_pntr - line);
            }
            else
            {
                strcpy(comp, dest_pntr + 1);
            }

            // Convert comp into binary
            if (strcmp(comp, "0") == 0)
            {
                memcpy(comp_start, "0101010", comp_bits);
            }
            else if (strcmp(comp, "1") == 0)
            {
                memcpy(comp_start, "0111111", comp_bits);
            }
            else if (strcmp(comp, "-1") == 0)
            {
                memcpy(comp_start, "0111010", comp_bits);
            }
            else if (strcmp(comp, "D") == 0)
            {
                memcpy(comp_start, "0001100", comp_bits);
            }
            else if (strcmp(comp, "A") == 0)
            {
                memcpy(comp_start, "0110000", comp_bits);
            }
            else if (strcmp(comp, "!D") == 0)
            {
                memcpy(comp_start, "0001101", comp_bits);
            }
            else if (strcmp(comp, "!A") == 0)
            {
                memcpy(comp_start, "0110011", comp_bits);
            }
            else if (strcmp(comp, "-D") == 0)
            {
                memcpy(comp_start, "0001111", comp_bits);
            }
            else if (strcmp(comp, "-A") == 0)
            {
                memcpy(comp_start, "0110011", comp_bits);
            }
            else if (strcmp(comp, "D+1") == 0)
            {
                memcpy(comp_start, "0011111", comp_bits);
            }
            else if (strcmp(comp, "A+1") == 0)
            {
                memcpy(comp_start, "0110111", comp_bits);
            }
            else if (strcmp(comp, "D-1") == 0)
            {
                memcpy(comp_start, "0001110", comp_bits);
            }
            else if (strcmp(comp, "A-1") == 0)
            {
                memcpy(comp_start, "0110010", comp_bits);
            }
            else if (strcmp(comp, "D+A") == 0)
            {
                memcpy(comp_start, "0000010", comp_bits);
            }
            else if (strcmp(comp, "D-A") == 0)
            {
                memcpy(comp_start, "0010011", comp_bits);
            }
            else if (strcmp(comp, "A-D") == 0)
            {
                memcpy(comp_start, "0000111", comp_bits);
            }
            else if (strcmp(comp, "D&A") == 0)
            {
                memcpy(comp_start, "0000000", comp_bits);
            }
            else if (strcmp(comp, "D|A") == 0)
            {
                memcpy(comp_start, "0010101", comp_bits);
            }
            else if (strcmp(comp, "M") == 0)
            {
                memcpy(comp_start, "1110000", comp_bits);
            }
            else if (strcmp(comp, "!M") == 0)
            {
                memcpy(comp_start, "1110001", comp_bits);
            }
            else if (strcmp(comp, "-M") == 0)
            {
                memcpy(comp_start, "1110011", comp_bits);
            }
            else if (strcmp(comp, "M+1") == 0)
            {
                memcpy(comp_start, "1110111", comp_bits);
            }
            else if (strcmp(comp, "M-1") == 0)
            {
                memcpy(comp_start, "1110010", comp_bits);
            }
            else if (strcmp(comp, "D+M") == 0)
            {
                memcpy(comp_start, "1000010", comp_bits);
            }
            else if (strcmp(comp, "D-M") == 0)
            {
                memcpy(comp_start, "1010011", comp_bits);
            }
            else if (strcmp(comp, "M-D") == 0)
            {
                memcpy(comp_start, "1000111", comp_bits);
            }
            else if (strcmp(comp, "D&M") == 0)
            {
                memcpy(comp_start, "1000000", comp_bits);
            }
            else if (strcmp(comp, "D|M") == 0)
            {
                memcpy(comp_start, "1010101", comp_bits);
            }
            else
            {
                fprintf(stderr, "Invalid compute on line %d.\n", i);
                clean_exit(program, 1);
            }

            // Convert dest into binary
            if (strcmp(dest, "M") == 0)
            {
                memcpy(dest_start, "001", dest_bits);
            }
            else if (strcmp(dest, "D") == 0)
            {
                memcpy(dest_start, "010", dest_bits);
            }
            else if (strcmp(dest, "MD") == 0)
            {
                memcpy(dest_start, "011", dest_bits);
            }
            else if (strcmp(dest, "A") == 0)
            {
                memcpy(dest_start, "100", dest_bits);
            }
            else if (strcmp(dest, "AM") == 0)
            {
                memcpy(dest_start, "101", dest_bits);
            }
            else if (strcmp(dest, "AD") == 0)
            {
                memcpy(dest_start, "110", dest_bits);
            }
            else if (strcmp(dest, "AMD") == 0)
            {
                memcpy(dest_start, "111", dest_bits);
            }
            else if (strcmp(dest, "") == 0)
            {
                memcpy(dest_start, "000", dest_bits);
            }
            else
            {
                fprintf(stderr, "Invalid destination on line %d.\n", i);
                clean_exit(program, 1);
            }

            // Convert jump into binary
            if (strcmp(jump, "JGT") == 0)
            {
                memcpy(jump_start, "001", jump_bits);
            }
            else if (strcmp(jump, "JEQ") == 0)
            {
                memcpy(jump_start, "010", jump_bits);
            }
            else if (strcmp(jump, "JGE") == 0)
            {
                memcpy(jump_start, "011", jump_bits);
            }
            else if (strcmp(jump, "JLT") == 0)
            {
                memcpy(jump_start, "100", jump_bits);
            }
            else if (strcmp(jump, "JNE") == 0)
            {
                memcpy(jump_start, "101", jump_bits);
            }
            else if (strcmp(jump, "JLE") == 0)
            {
                memcpy(jump_start, "110", jump_bits);
            }
            else if (strcmp(jump, "JMP") == 0)
            {
                memcpy(jump_start, "111", jump_bits);
            }
            else if (strcmp(jump, "") == 0)
            {
                memcpy(jump_start, "000", jump_bits);
            }
            else
            {
                fprintf(stderr, "Invalid jump on line %d.\n", i);
                clean_exit(program, 1);
            }
        }

        // Add the binary instruction to the overall binary program.
        strncat(program->binary, instruction, strlen(instruction) + 1);
    }
}

/* Generates a .hack file containing the raw binary of converted assembly
program. */
bool gen_hack(char *filename, Program *program)
{
    FILE *fp = fopen(filename, "w");
    if (fp != NULL)
    {
        fwrite(program->binary, strlen(program->binary), 1, fp);
    }
    else
    {
        fprintf(stderr, "Unable to generate %s\n", filename);
        return false;
    }

    fclose(fp);
    return true;
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
    // Add predefined symbols to table

    // Perform first pass
    if (!first_pass(argv[1], &program))
    {
        clean_exit(&program, 1);
    }

    // Perform second pass
    second_pass(&program);

    // Write binary to .hack file
    if (!gen_hack("out.hack", &program))
    {
        clean_exit(&program, 1);
    }

    clean_exit(&program, 0);
}