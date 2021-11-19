#include <stdio.h>
#include <SDL2/SDL.h>
#include "emulib.h"

int main(int argc, char **argv)
{
    char rom_path[FILENAME_MAX];

    if (argc != 2)
    {
        fprintf(stderr, "Usage: ./hackemu <path-to-file>\n");
        return 1;
    }
    else
    {
        strncpy(rom_path, argv[1], FILENAME_MAX);
        rom_path[FILENAME_MAX - 1] = '\0';
    }

    Hack machine;
    hack_init(&machine);

    if (!hack_load_rom(&machine, rom_path))
    {
        return 1;
    }

    hack_print_rom(&machine);

    return 0;
}
