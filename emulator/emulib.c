#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "emulib.h"

#define COMP_NUM_BITS 7
#define COMP_START_BIT 3
#define DEST_START_BIT 10
#define JUMP_START_BIT 13

// Retrives the nth most significant bit of a number
static inline bool get_bit(uint16_t num, int i)
{
    return ((num << i) & 0x8000);
}

// Clear the ROM
static void hack_clear_rom(Hack *this)
{
    for (int i = 0; i < MEM_SIZE; i++)
    {
        this->rom[i] = 0;
    }
}

// Clear the RAM
static void hack_clear_ram(Hack *this)
{
    for (int i = 0; i < MEM_SIZE; i++)
    {
        this->ram[i] = 0;
    }
}

// Calculate comp value
static int hack_calc_comp(const Hack *this, uint16_t instruction)
{
    const int16_t A = this->a_reg;
    const int16_t D = this->d_reg;
    const int16_t M = this->ram[A];

    /* The comp bit ends at the 7th LSB so we shift the other LSBs off and since
     * the comp bit starts at 10th MSB (after shift) we mask all the preceding
     * MSBs.
     */
    const uint16_t comp_bits = (instruction >> 6) & 0x007F;
    //printf("   hack_calc_comp(): ALU instruction %X\n", comp_bits);

    switch (comp_bits)
    {
    case 0x2A:
        return 0;
    case 0x3F:
        return 1;
    case 0x3A:
        return -1;
    case 0x0C:
        return D;
    case 0x30:
        return A;
    case 0x0D:
        return ~D;
    case 0x31:
        return ~A;
    case 0x0F:
        return -D;
    case 0x33:
        return -A;
    case 0x1F:
        return D + 1;
    case 0x37:
        return A + 1;
    case 0x0E:
        return D - 1;
    case 0x32:
        return A - 1;
    case 0x02:
        return D + A;
    case 0x13:
        return D - A;
    case 0x07:
        return A - D;
    case 0x00:
        return D & A;
    case 0x15:
        return D | A;
    case 0x70:
        return M;
    case 0x71:
        return ~M;
    case 0x73:
        return -M;
    case 0x77:
        return M + 1;
    case 0x72:
        return M - 1;
    case 0x42:
        return D + M;
    case 0x53:
        return D - M;
    case 0x47:
        return M - D;
    case 0x40:
        return D & M;
    case 0x55:
        return D | M;
    default:
        return 0;
    }
}

void hack_get_coords(int *x, int *y, uint16_t addr)
{
    *x = addr % 32;
    *y = (addr - SCREEN_ADDR) / 32;
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
    uint16_t instruction = this->rom[this->pc++];
    // Handle an A instruction
    if (!get_bit(instruction, 0))
    {
        this->a_reg = instruction;
	//printf("instruction %d:A %d\n",this->pc, instruction);
    }

    // Handle a C instruction
    else
    {
        // Compute a value
	//printf("instruction %d:D %x\n",this->pc, instruction);
        const int comp = hack_calc_comp(this, instruction);

        // Store computed value in appropriate destinations
        if (get_bit(instruction, DEST_START_BIT + 2))
        {
            this->ram[this->a_reg] = comp;
        }
        if (get_bit(instruction, DEST_START_BIT + 1))
        {
            this->d_reg = comp;
        }
        if (get_bit(instruction, DEST_START_BIT))
        {
            this->a_reg = comp;
        }

        // Decide if a jump should be performed
        if ((get_bit(instruction, JUMP_START_BIT) && comp < 0) ||
            (get_bit(instruction, JUMP_START_BIT + 1) && comp == 0) ||
            (get_bit(instruction, JUMP_START_BIT + 2) && comp > 0))
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

    /* A Hack ROM is an ASCII file with one instruction per line, so we convert
     * each line to an actual number before storing in emulator ROM.
     */
    char line[WORD_SIZE + 2];
    while (fgets(line, WORD_SIZE + 2, fp) != NULL)
    {
        line[WORD_SIZE] = '\0';
        this->rom[this->program_size++] = strtol(line, NULL, 2);
    }

    fclose(fp);
    return true;
}

void hack_print_rom(const Hack *this)
{
    for (int i = 0; i < this->program_size; i++)
    {
        printf("0x%X\n", this->rom[i]);
    }
}

void hack_print_ram(const Hack *this)
{
    for (int i = 0; i < MEM_SIZE; i++)
    {
        int16_t mem = this->ram[i];

        if (mem)
        {
            printf("%d: %d\n", i, mem);
        }
    }
}
