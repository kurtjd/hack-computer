#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "emulib.h"

#define COMP_NUM_BITS 7
#define COMP_START_BIT 3
#define DEST_START_BIT 10
#define JUMP_START_BIT 13

// Clear the ROM
static void hack_clear_rom(Hack *this)
{
    for (int i = 0; i < ROM_SIZE; i++)
    {
        strcpy(this->rom[i], "0000000000000000");
    }
}

// Clear the display memory
/*static void hack_clear_display(Hack *this)
{
    for (int i = SCREEN_ADDR; i < KEYBD_ADDR; i++)
    {
        this->ram[i] = 0;
    }
}*/

// Clear the RAM
static void hack_clear_ram(Hack *this)
{
    for (int i = 0; i < RAM_SIZE; i++)
    {
        this->ram[i] = 0;
    }
}

// Calculate comp value
static int16_t hack_calc_comp(const Hack *this, const char *comp_bits)
{
    const int16_t A = this->a_reg;
    const int16_t D = this->d_reg;
    const int16_t M = this->ram[this->a_reg];

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

void hack_init(Hack *this)
{
    this->program_size = 0;
    this->pc = 0;
    this->ram[KEYBD_ADDR] = 0;

    hack_clear_rom(this);
    hack_clear_ram(this);
}

void hack_execute(Hack *this)
{
    // Fetch instruction from ROM and increment program counter
    char instruction[WORD_SIZE + 1];
    strcpy(instruction, this->rom[this->pc++]);

    // Handle an A instruction
    if (instruction[0] == '0')
    {
        // Convert the last 15 bits to a number and store in A register
        this->a_reg = strtol(instruction + 1, NULL, 2);
    }

    // Handle a C instruction
    else
    {
        // Compute a value
        char comp_bits[8] = {'\0'};
        strncpy(comp_bits, instruction + COMP_START_BIT, COMP_NUM_BITS);
        const int16_t comp = hack_calc_comp(this, comp_bits);

        // Store computed value in appropriate destinations
        if (instruction[DEST_START_BIT + 2] == '1')
        {
            this->ram[this->a_reg] = comp;
        }
        if (instruction[DEST_START_BIT + 1] == '1')
        {
            this->d_reg = comp;
        }
        if (instruction[DEST_START_BIT] == '1')
        {
            this->a_reg = comp;
        }

        // Decide if a jump should be performed
        if ((instruction[JUMP_START_BIT] == '1' && comp < 0) ||
            (instruction[JUMP_START_BIT + 1] == '1' && comp == 0) ||
            (instruction[JUMP_START_BIT + 2] == '1' && comp > 0))
        {
            this->pc = this->a_reg;
        }
    }
}

bool hack_load_rom(Hack *this, const char *filepath)
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
    while (fgets(this->rom[this->program_size],
                 WORD_SIZE + 2, fp) != NULL)
    {
        this->rom[this->program_size++][WORD_SIZE] = '\0';
    }

    fclose(fp);
    return true;
}

void hack_print_rom(const Hack *this)
{
    for (int i = 0; i < this->program_size; i++)
    {
        printf("%s\n", this->rom[i]);
    }
}

void hack_print_ram(const Hack *this)
{
    for (int i = 0; i < RAM_SIZE; i++)
    {
        int16_t mem = this->ram[i];

        if (mem)
        {
            printf("%d: %d\n", i, mem);
        }
    }
}