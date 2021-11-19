#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "emulib.h"

// Clear the display memory
static void hack_clear_rom(Hack *machine)
{
    for (int i = 0; i < ROM_SIZE; i++)
    {
        strcpy(machine->rom[i], "0000000000000000");
    }
}

// Clear the ROM
static void hack_clear_display(Hack *machine)
{
    for (int i = SCREEN_ADDR; i < KEYBD_ADDR; i++)
    {
        machine->ram[i] = 0;
    }
}

// Calculate comp value
static int16_t hack_calc_comp(const Hack *machine, const char *comp_bits)
{
    const int16_t A = machine->a_reg;
    const int16_t D = machine->d_reg;
    const int16_t M = machine->ram[machine->a_reg];

    if (strcmp(comp_bits, "0101010") == 0)
    {
        return 0;
    }
    else if (strcmp(comp_bits, "0111111") == 0)
    {
        return 1;
    }
    else if (strcmp(comp_bits, "0111010") == 0)
    {
        return -1;
    }
    else if (strcmp(comp_bits, "0001100") == 0)
    {
        return D;
    }
    else if (strcmp(comp_bits, "0110000") == 0)
    {
        return A;
    }
    else if (strcmp(comp_bits, "0001101") == 0)
    {
        return ~D;
    }
    else if (strcmp(comp_bits, "0110001") == 0)
    {
        return ~A;
    }
    else if (strcmp(comp_bits, "0001111") == 0)
    {
        return -D;
    }
    else if (strcmp(comp_bits, "0110011") == 0)
    {
        return -A;
    }
    else if (strcmp(comp_bits, "0011111") == 0)
    {
        return D + 1;
    }
    else if (strcmp(comp_bits, "0110111") == 0)
    {
        return A + 1;
    }
    else if (strcmp(comp_bits, "0001110") == 0)
    {
        return D - 1;
    }
    else if (strcmp(comp_bits, "0110010") == 0)
    {
        return A - 1;
    }
    else if (strcmp(comp_bits, "0000010") == 0)
    {
        return D + A;
    }
    else if (strcmp(comp_bits, "0010011") == 0)
    {
        return D - A;
    }
    else if (strcmp(comp_bits, "0000111") == 0)
    {
        return A - D;
    }
    else if (strcmp(comp_bits, "0000000") == 0)
    {
        return D & A;
    }
    else if (strcmp(comp_bits, "0010101") == 0)
    {
        return D | A;
    }
    else if (strcmp(comp_bits, "1110000") == 0)
    {
        return M;
    }
    else if (strcmp(comp_bits, "1110001") == 0)
    {
        return ~M;
    }
    else if (strcmp(comp_bits, "1110011") == 0)
    {
        return -M;
    }
    else if (strcmp(comp_bits, "1110111") == 0)
    {
        return M + 1;
    }
    else if (strcmp(comp_bits, "1110010") == 0)
    {
        return M - 1;
    }
    else if (strcmp(comp_bits, "1000010") == 0)
    {
        return D + M;
    }
    else if (strcmp(comp_bits, "1010011") == 0)
    {
        return D - M;
    }
    else if (strcmp(comp_bits, "1000111") == 0)
    {
        return M - D;
    }
    else if (strcmp(comp_bits, "1000000") == 0)
    {
        return D & M;
    }
    else if (strcmp(comp_bits, "1010101") == 0)
    {
        return D | M;
    }
    else
    {
        return 0;
    }
}

void hack_init(Hack *machine)
{
    machine->program_size = 0;
    machine->pc = 0;
    machine->ram[KEYBD_ADDR] = 0;

    hack_clear_rom(machine);
    hack_clear_display(machine);
}

void hack_execute(Hack *machine)
{
    // Fetch instruction from ROM and increment program counter
    char instruction[WORD_SIZE + 1];
    strcpy(instruction, machine->rom[machine->pc++]);

    // Handle an A instruction (which stores a number in the A register)
    if (instruction[0] == '0')
    {
        // Convert the last 15 bits to a number
        machine->a_reg = strtol(instruction + 1, NULL, 2);
    }

    // Handle a C instruction
    else
    {
        char comp_bits[8] = {'\0'};
        strncpy(comp_bits, instruction + 3, 7);
        int16_t comp = hack_calc_comp(machine, comp_bits);
    }
}

bool hack_load_rom(Hack *machine, const char *filepath)
{
    if (strlen(filepath) > FILENAME_MAX)
    {
        fprintf(stderr, "Unable to open ROM because filename"
                        " exceeds %d characters.\n",
                FILENAME_MAX);
        return false;
    }

    FILE *fp = fopen(filepath, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Unable to open %s\n", filepath);
        return false;
    }

    /* A hack ROM has one instruction per line, and since one instruction is 
     * 16 "bits" (word size), a line has a total of 17 bytes. So we read in each
     * line but replace the newline character with a string terminator and
     * increase the program size.
     */
    while (fgets(machine->rom[machine->program_size],
                 WORD_SIZE + 2, fp) != NULL)
    {
        machine->rom[machine->program_size++][WORD_SIZE] = '\0';
    }

    fclose(fp);
    return true;
}

void hack_print_rom(const Hack *machine)
{
    for (int i = 0; i < machine->program_size; i++)
    {
        printf("%s\n", machine->rom[i]);
    }
}
