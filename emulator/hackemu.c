#include <stdio.h>
#include <SDL2/SDL.h>
#include "emulib.h"

#define TITLE "Hack Emulator"
#define FRAME_RATE 60
#define CPU_FREQ 100
#define ON_COLOR 0xFFFFFF
#define OFF_COLOR 0x000000

// Initializes SDL
bool init_SDL(void)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "Could not initialize SDL: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

// Create the SDL window
SDL_Window *create_window(void)
{
    SDL_Window *new_window = SDL_CreateWindow(TITLE,
                                              SDL_WINDOWPOS_CENTERED,
                                              SDL_WINDOWPOS_CENTERED,
                                              DISPLAY_WIDTH,
                                              DISPLAY_HEIGHT,
                                              SDL_WINDOW_SHOWN);
    if (!new_window)
    {
        fprintf(stderr, "Could not create SDL window: %s\n", SDL_GetError());
        return NULL;
    }

    return new_window;
}

// Sets a pixel of the SDL surface to a certain color
void set_pixel(SDL_Surface *surface, int x, int y, long color)
{
    Uint32 *pixels = (Uint32 *)surface->pixels;
    pixels[(y * surface->w) + x] = color;
}

// Makes the physical screen match the emulator display
void draw_display(const Hack *machine, SDL_Window *window, SDL_Surface *surface)
{
    for (int i = SCREEN_ADDR; i < KEYBD_ADDR; i++)
    {
        int x, y;
        hack_get_coords(&x, &y, i);

        for (int j = 0; j < WORD_SIZE; j++)
        {
            int newx = (WORD_SIZE * x) + j;

            long color = ((machine->ram[i] >> j) & 1) ? ON_COLOR : OFF_COLOR;
            set_pixel(surface, newx, y, color);
        }
    }

    SDL_UpdateWindowSurface(window);
}

// Checks for key presses/releases and a quit event.
bool handle_input(Hack *machine, SDL_Event *e)
{
    while (SDL_PollEvent(e))
    {
        switch (e->type)
        {
        case SDL_QUIT:
            return false;
            break;
        case SDL_KEYDOWN:
            machine->ram[KEYBD_ADDR] = e->key.keysym.sym;
            break;
        case SDL_KEYUP:
            machine->ram[KEYBD_ADDR] = 0;
            break;
        }
    }

    return true;
}

// Frees all resources and exits.
void clean_exit(SDL_Window *window, SDL_Surface *surface, int status)
{
    if (surface != NULL)
    {
        SDL_FreeSurface(surface);
        surface = NULL;
    }

    if (window != NULL)
    {
        SDL_DestroyWindow(window);
        window = NULL;
    }

    SDL_Quit();

    exit(status);
}

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

    if (!init_SDL())
    {
        return 1;
    }

    SDL_Window *window = create_window();
    if (!window)
    {
        clean_exit(NULL, NULL, 1);
    }

    SDL_Surface *surface = SDL_GetWindowSurface(window);
    if (!surface)
    {
        fprintf(stderr, "Could not create SDL surface: %s\n", SDL_GetError());
        clean_exit(window, NULL, 1);
    }

    Hack machine;
    hack_init(&machine);

    if (!hack_load_rom(&machine, rom_path))
    {
        clean_exit(window, surface, 1);
    }

    SDL_Event e;
    bool quit = false;
    while (!quit && machine.pc < machine.program_size)
    {
        // Cap execution speed
        if (SDL_GetTicks() % (1000 / CPU_FREQ) <= 1)
        {
            hack_execute(&machine);
        }

        // Cap input/draw rate
        if (SDL_GetTicks() % (1000 / FRAME_RATE) <= 1)
        {
            quit = !handle_input(&machine, &e);
            draw_display(&machine, window, surface);
        }
    }

    clean_exit(window, surface, 0);
}
