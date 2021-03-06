#ifndef EMULIB_H
#define EMULIB_H

#include <stdint.h>
#include <stdbool.h>

#define MEM_SIZE 32768
#define WORD_SIZE 16
#define SCREEN_ADDR 0x4000
#define KEYBD_ADDR 0x6000
#define DISPLAY_WIDTH 512
#define DISPLAY_HEIGHT 256

typedef enum
{
    HACK_KEY_BACKSPACE = 129,
    HACK_KEY_LEFT,
    HACK_KEY_UP,
    HACK_KEY_RIGHT,
    HACK_KEY_DOWN,
    HACK_KEY_HOME,
    HACK_KEY_END,
    HACK_KEY_PAGEUP,
    HACK_KEY_PAGEDOWN,
    HACK_KEY_INSERT,
    HACK_KEY_DELETE,
    HACK_KEY_ESCAPE,
    HACK_KEY_F
} HACK_KEYS;

/* Hack is a 16-bit computer.
 * Therefore, the smallest piece of addressable memory is not a byte but a
 * 16-bit word because the Hack platform offers no other means of addressing
 * within a register of memory.
 * 
 * The Hack platform also naturally assumes all data registers to be signed
 * hence the use of int16_t as opposed to uint16_t.
 * 
 * Finally, the Hack computer utilizes Harvard architecture hence the separated
 * program and data memory.
 */
typedef struct Hack
{
    // Read-only instruction memory.
    uint16_t rom[MEM_SIZE];
    int program_size;

    // Random-access memory
    int16_t ram[MEM_SIZE];

    // A (address), D, and program counter CPU registers
    uint16_t pc;
    int16_t a_reg, d_reg;
} Hack;

// Gets an x and y coordinate from a screen address
void hack_get_coords(int *x, int *y, uint16_t addr);

// Initialize the machine
void hack_init(Hack *this);

// Execute the instruction located by the program counter
void hack_execute(Hack *this);

/* Load a file into the machine's ROM
 * Returns false if unable to open file
 */
bool hack_load_rom(Hack *this, const char *filepath);

// Prints the contents of the machine's ROM one instruction per line
void hack_print_rom(const Hack *this);

// Prints RAM registers with values != 0
void hack_print_ram(const Hack *this);

#endif
