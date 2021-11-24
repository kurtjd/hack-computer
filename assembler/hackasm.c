/* Hack Computer Assembler/Disassembler
 *
 * Assembler reads a .asm file and generates a .hack file in ASCII with one
 * instruction per line.
 * Disassembler reads an ASCII .hack file and generates a .asm file with one
 * instruction per line.
 * 
 * Written by Kurtis Dinelle for the nand2tetris course.
 */

#define MAX_LINE_LEN 128
#define LINE_CHUNK 128
#define PROGRAM_BUF_CHUNK (MAX_LINE_LEN * LINE_CHUNK)
#define INSTR_BITS 16
#define SYMBOL_MAX_LEN 64
#define VAR_START_ADDR 16
#define MAX_SYMBOLS 64000

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>

// Represents a symbol as a name:value pair.
typedef struct Symbol
{
    char name[SYMBOL_MAX_LEN];
    char value[SYMBOL_MAX_LEN];
} Symbol;

// Sets the values of a symbol.
void symbol_set_value(Symbol *symbol, char *name, char *value)
{
    strncpy(symbol->name, name, SYMBOL_MAX_LEN);
    strncpy(symbol->value, value, SYMBOL_MAX_LEN);
}

/* Represents the list of symbols found in the program.
Although a hash table might make more sense, a simple array is much quicker
to implement and may even be faster since the size of the table should be
relatively small. */
typedef struct SymbolTable
{
    Symbol symbols[MAX_SYMBOLS];
    int count;
    int variables;
} SymbolTable;

// Adds a symbol to the table.
bool symboltable_add(SymbolTable *table, Symbol symbol, bool is_var)
{
    table->symbols[table->count] = symbol;
    table->count++;

    if (table->count >= MAX_SYMBOLS)
    {
        fprintf(stderr, "Programs can only contain a max of %d symbols.\n", MAX_SYMBOLS);
        return false;
    }

    if (is_var)
    {
        table->variables++;
    }

    return true;
}

// Retrieves the value of a symbol if it exists.
void symboltable_get(SymbolTable *table, char *name, char *dest)
{
    for (int i = 0; i < table->count; i++)
    {
        if (strcmp(table->symbols[i].name, name) == 0)
        {
            strncpy(dest, table->symbols[i].value, SYMBOL_MAX_LEN);
            break;
        }
    }
}

// Initializes the symbol table.
void symboltable_init(SymbolTable *table)
{
    table->count = 0;
    table->variables = 0;
    Symbol symbol;

    // Add predefined symbols
    symbol_set_value(&symbol, "SP", "0");
    symboltable_add(table, symbol, false);

    symbol_set_value(&symbol, "LCL", "1");
    symboltable_add(table, symbol, false);

    symbol_set_value(&symbol, "ARG", "2");
    symboltable_add(table, symbol, false);

    symbol_set_value(&symbol, "THIS", "3");
    symboltable_add(table, symbol, false);

    symbol_set_value(&symbol, "THAT", "4");
    symboltable_add(table, symbol, false);

    // Add symbols R0-R15
    for (int i = 0; i < 16; i++)
    {
        char label[4] = "R";
        char num[3];
        sprintf(num, "%d", i);
        strncat(label, num, 3);

        symbol_set_value(&symbol, label, num);
        symboltable_add(table, symbol, false);
    }

    symbol_set_value(&symbol, "SCREEN", "16384");
    symboltable_add(table, symbol, false);

    symbol_set_value(&symbol, "KBD", "24576");
    symboltable_add(table, symbol, false);
}

// Print contents of symbol table.
void symboltable_print(SymbolTable *table)
{
    printf("Name : Value\n");
    printf("------------\n");
    for (int i = 0; i < table->count; i++)
    {
        printf("%s : %s\n", table->symbols[i].name, table->symbols[i].value);
    }
}

// Contains information about the loaded program such as contents and size.
typedef struct Program
{
    char *binary;
    char *assembly;
    size_t size;
    int lines;
} Program;

// Initialize the program.
void program_init(Program *program)
{
    program->assembly = calloc(PROGRAM_BUF_CHUNK, sizeof(char));
    if (program->assembly == NULL)
    {
        fprintf(stderr, "Could not create program buffer.\n");
        exit(1);
    }

    program->size = PROGRAM_BUF_CHUNK * sizeof(char);
    program->lines = 0;
}

// Add a line to the program.
void program_add_line(Program *program, char *line, size_t line_len)
{
    strncpy(program->assembly + (program->lines * MAX_LINE_LEN), line, line_len + 1);

    // Expand the program buffer every LINE_CHUNK number of lines.
    program->lines++;
    if (program->lines % LINE_CHUNK == 0)
    {
        program->size += PROGRAM_BUF_CHUNK;
        program->assembly = realloc(program->assembly, program->size);
        if (program->assembly == NULL)
        {
            fprintf(stderr, "Unable to resize program buffer.\n");
            exit(1);
        }
    }
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

// Handles a symbol found on the first pass.
void handle_symbol_fp(Program *program, SymbolTable *symtbl, char *line)
{
    Symbol symbol = {"", ""};
    strncpy(symbol.name, line + 1, strlen(line) - 2);
    symboltable_get(symtbl, symbol.name, symbol.value);

    if (symbol.value[0] == '\0')
    {
        sprintf(symbol.value, "%d", program->lines);
        if (!symboltable_add(symtbl, symbol, false))
        {
            clean_exit(program, 1);
        }
    }
}

/* Performs the first pass which includes removing whitespace and comments,
and adding labels to the symbol table. */
bool first_pass(char *filename, Program *program, SymbolTable *symtbl)
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
            /* If a label is encountered, add it to the symbol table if new and
            strip it from the program. */
            if (line[0] == '(')
            {
                handle_symbol_fp(program, symtbl, line);
                continue;
            }

            // Add line to program buffer.
            program_add_line(program, line, line_len);
        }
    }

    fclose(fp);

    // Create space to store the binary conversion of program
    program->binary = calloc(program->size, sizeof(char));
    if (program->binary == NULL)
    {
        fprintf(stderr, "Unable to create binary program buffer.\n");
        clean_exit(program, 1);
    }

    return true;
}

// Splits asm instruction into its parts.
void split_asm(char *line, char *dest, char *comp, char *jump)
{
    char *dest_pntr = strchr(line, '=');
    char *jump_pntr = strchr(line, ';');

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
}

// Converts the comp as asm to binary.
bool comp_asm_to_bin(char *comp, char *comp_start, int comp_bits)
{
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
        return false;
    }

    return true;
}

// Converts the dest as asm to binary.
bool dest_asm_to_bin(char *dest, char *dest_start, int dest_bits)
{
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
        return false;
    }

    return true;
}

// Converts the jump as asm to binary.
bool jump_asm_to_bin(char *jump, char *jump_start, int jump_bits)
{
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
        return false;
    }

    return true;
}

// Handles a symbol found in second pass.
void handle_symbol_sp(Program *program, SymbolTable *symtbl, char *line)
{
    Symbol symbol = {"", ""};
    strncpy(symbol.name, line + 1, strlen(line));
    symboltable_get(symtbl, symbol.name, symbol.value);

    /* If the symbol does not exist yet, we know it must be a
                variable since labels were added to the table in the first pass.
                Thus, we add it to the table with an appropriate address. */
    if (symbol.value[0] == '\0')
    {
        sprintf(symbol.value, "%d", VAR_START_ADDR + symtbl->variables);
        if (!symboltable_add(symtbl, symbol, true))
        {
            clean_exit(program, 1);
        }
    }

    // Replace the line with the value of the symbol.
    strncpy(line + 1, symbol.value, strlen(symbol.value) + 1);
}

// Converts the decimal address in asm instruction to binary string.
void asm_dec_to_bin(char *line, char *instruction)
{
    /* Get the least significant bit of address, convert it to
        ASCII, and store it in respective instruction bit. */
    uint16_t addr = atoi(line + 1);
    for (int b = 0; b < (INSTR_BITS - 1); b++)
    {
        instruction[(INSTR_BITS - 1) - b] = ((addr >> b) & 0x0001) + '0';
    }
}

/* Performs the second pass which includes converting symbols into numbers and
instructions into binary. */
void second_pass(Program *program, SymbolTable *symtbl)
{
    for (int i = 0; i < program->lines; i++)
    {
        // Get the line of assembly code
        char line[MAX_LINE_LEN];
        strncpy(line, program->assembly + (i * MAX_LINE_LEN), MAX_LINE_LEN);

        char instruction[INSTR_BITS + 2] = "0000000000000000\n";

        // Handle A instruction (which all begin with '@')
        if (line[0] == '@')
        {
            /* If address doesn't begin with number, we know it's a symbol 
            so look it up in symbol table. */
            if (!isdigit(line[1]))
            {
                handle_symbol_sp(program, symtbl, line);
            }

            asm_dec_to_bin(line, instruction);
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
            split_asm(line, dest, comp, jump);

            int comp_bits = 7;
            int dest_bits = 3;
            int jump_bits = 3;

            char *comp_start = instruction + 3;
            char *dest_start = instruction + 3 + comp_bits;
            char *jump_start = instruction + 3 + comp_bits + dest_bits;

            // Convert comp into binary
            if (!comp_asm_to_bin(comp, comp_start, comp_bits))
            {
                fprintf(stderr, "Invalid compute on line %d.\n", i);
                clean_exit(program, 1);
            }

            // Convert dest into binary
            if (!dest_asm_to_bin(dest, dest_start, dest_bits))
            {
                fprintf(stderr, "Invalid destination on line %d.\n", i);
                clean_exit(program, 1);
            }

            // Convert jump into binary
            if (!jump_asm_to_bin(jump, jump_start, jump_bits))
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

// Converts binary string to decimal integer.
int bin_to_dec(char *bin)
{
    int dec = 0;
    int place = 16384; // 15th bit's place value

    while (*bin++)
    {
        if (bin[0] == '1')
        {
            dec += place;
        }

        place /= 2;
    }

    return dec;
}

// Splits binary instruction into its parts.
void split_bin(char *line, char *comp, char *dest, char *jump)
{
    strncpy(comp, line + 3, 7);
    strncpy(dest, line + 10, 3);
    strncpy(jump, line + 13, 3);
}

// Converts the dest as binary to asm.
void dest_bin_to_asm(char *dest, char *instruction)
{
    if (strcmp(dest, "001") == 0)
    {
        strcat(instruction, "M");
    }
    else if (strcmp(dest, "010") == 0)
    {
        strcat(instruction, "D");
    }
    else if (strcmp(dest, "011") == 0)
    {
        strcat(instruction, "MD");
    }
    else if (strcmp(dest, "100") == 0)
    {
        strcat(instruction, "A");
    }
    else if (strcmp(dest, "101") == 0)
    {
        strcat(instruction, "AM");
    }
    else if (strcmp(dest, "110") == 0)
    {
        strcat(instruction, "AD");
    }
    else if (strcmp(dest, "111") == 0)
    {
        strcat(instruction, "AMD");
    }
    if (!strcmp(dest, "000") == 0)
    {
        strcat(instruction, "=");
    }
}

// Converts the comp as binary to asm.
void comp_bin_to_asm(char *comp, char *instruction)
{
    if (strcmp(comp, "0101010") == 0)
    {
        strcat(instruction, "0");
    }
    else if (strcmp(comp, "0111111") == 0)
    {
        strcat(instruction, "1");
    }
    else if (strcmp(comp, "0111010") == 0)
    {
        strcat(instruction, "-1");
    }
    else if (strcmp(comp, "0001100") == 0)
    {
        strcat(instruction, "D");
    }
    else if (strcmp(comp, "0110000") == 0)
    {
        strcat(instruction, "A");
    }
    else if (strcmp(comp, "0001101") == 0)
    {
        strcat(instruction, "!D");
    }
    else if (strcmp(comp, "0110001") == 0)
    {
        strcat(instruction, "!A");
    }
    else if (strcmp(comp, "0001111") == 0)
    {
        strcat(instruction, "-D");
    }
    else if (strcmp(comp, "0110011") == 0)
    {
        strcat(instruction, "-A");
    }
    else if (strcmp(comp, "0011111") == 0)
    {
        strcat(instruction, "D+1");
    }
    else if (strcmp(comp, "0110111") == 0)
    {
        strcat(instruction, "A+1");
    }
    else if (strcmp(comp, "0001110") == 0)
    {
        strcat(instruction, "D-1");
    }
    else if (strcmp(comp, "0110010") == 0)
    {
        strcat(instruction, "A-1");
    }
    else if (strcmp(comp, "0000010") == 0)
    {
        strcat(instruction, "D+A");
    }
    else if (strcmp(comp, "0010011") == 0)
    {
        strcat(instruction, "D-A");
    }
    else if (strcmp(comp, "000111") == 0)
    {
        strcat(instruction, "A-D");
    }
    else if (strcmp(comp, "0000000") == 0)
    {
        strcat(instruction, "D&A");
    }
    else if (strcmp(comp, "0010101") == 0)
    {
        strcat(instruction, "D|A");
    }
    else if (strcmp(comp, "1110000") == 0)
    {
        strcat(instruction, "M");
    }
    else if (strcmp(comp, "1110001") == 0)
    {
        strcat(instruction, "!M");
    }
    else if (strcmp(comp, "1110011") == 0)
    {
        strcat(instruction, "-M");
    }
    else if (strcmp(comp, "1110111") == 0)
    {
        strcat(instruction, "M+1");
    }
    else if (strcmp(comp, "1110010") == 0)
    {
        strcat(instruction, "M-1");
    }
    else if (strcmp(comp, "1000010") == 0)
    {
        strcat(instruction, "D+M");
    }
    else if (strcmp(comp, "1010011") == 0)
    {
        strcat(instruction, "D-M");
    }
    else if (strcmp(comp, "1000111") == 0)
    {
        strcat(instruction, "M-D");
    }
    else if (strcmp(comp, "1000000") == 0)
    {
        strcat(instruction, "D&M");
    }
    else if (strcmp(comp, "1010101") == 0)
    {
        strcat(instruction, "D|M");
    }
}

// Converts the jump as binary to asm.
void jump_bin_to_asm(char *jump, char *instruction)
{
    if (!strcmp(jump, "000") == 0)
    {
        strcat(instruction, ";");
    }
    if (strcmp(jump, "001") == 0)
    {
        strcat(instruction, "JGT");
    }
    else if (strcmp(jump, "010") == 0)
    {
        strcat(instruction, "JEQ");
    }
    else if (strcmp(jump, "011") == 0)
    {
        strcat(instruction, "JGE");
    }
    else if (strcmp(jump, "100") == 0)
    {
        strcat(instruction, "JLT");
    }
    else if (strcmp(jump, "101") == 0)
    {
        strcat(instruction, "JNE");
    }
    else if (strcmp(jump, "110") == 0)
    {
        strcat(instruction, "JLE");
    }
    else if (strcmp(jump, "111") == 0)
    {
        strcat(instruction, "JMP");
    }
}

// Converts a binary program into Hack assembly.
bool disassemble(char *filename)
{
    // Create out.asm file to start writing instructions to
    remove("out.asm");
    FILE *out = fopen("out.asm", "a");
    if (out == NULL)
    {
        fprintf(stderr, "Unable to open out.asm\n");
        return false;
    }

    // Open and read .hack file line-by-line
    char line[MAX_LINE_LEN];
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Unable to open %s\n", filename);
        return false;
    }

    while (fgets(line, MAX_LINE_LEN, fp) != NULL)
    {
        char instruction[MAX_LINE_LEN] = "";

        if (line[0] == '0')
        {
            // Begin instruction with the @ symbol
            strncpy(instruction, "@", 2);

            // Convert rest of the line which is in binary to decimal
            sprintf(instruction + 1, "%d", bin_to_dec(line));
        }
        else
        {
            // Split binary instruction into parts
            char comp[8] = {0, 0, 0, 0, 0, 0, 0, 0};
            char dest[4] = {0, 0, 0, 0};
            char jump[4] = {0, 0, 0, 0};
            split_bin(line, comp, dest, jump);

            // Convert binary to asm
            dest_bin_to_asm(dest, instruction);
            comp_bin_to_asm(comp, instruction);
            jump_bin_to_asm(jump, instruction);
        }

        // Put each instruction on its own line in asm file
        strcat(instruction, "\n");
        fwrite(instruction, strlen(instruction), 1, out);
    }

    fclose(fp);
    fclose(out);
    return true;
}

int main(int argc, char **argv)
{
    // Perform disassembly if -d flag present
    if (argc == 3 && strcmp(argv[1], "-d") == 0)
    {
        if (disassemble(argv[2]))
        {
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else if (argc != 2)
    {
        fprintf(stderr, "Usage: ./hackasm [-d] <path-to-file>\n");
        return 1;
    }

    // Initialize container which holds the contents of the assembly file.
    Program program;
    program_init(&program);

    // Initialize symbol table
    SymbolTable symtbl;
    symboltable_init(&symtbl);

    // Perform first pass
    if (!first_pass(argv[1], &program, &symtbl))
    {
        clean_exit(&program, 1);
    }

    // Perform second pass
    second_pass(&program, &symtbl);

    // Write binary to .hack file
    if (!gen_hack("out.hack", &program))
    {
        clean_exit(&program, 1);
    }

    clean_exit(&program, 0);
}