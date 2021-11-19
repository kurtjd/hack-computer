#include <stdio.h>
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

void hack_init(Hack *machine)
{
    machine->program_size = 0;
    machine->pc = 0;
    machine->ram[KEYBD_ADDR] = 0;

    hack_clear_rom(machine);
    hack_clear_display(machine);
}

bool hack_load_rom(Hack *machine, char *filepath)
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

void hack_print_rom(Hack *machine)
{
    for (int i = 0; i < machine->program_size; i++)
    {
        printf("%s\n", machine->rom[i]);
    }
}
