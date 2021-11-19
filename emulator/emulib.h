#include <stdint.h>
#include <stdbool.h>

#define ROM_SIZE 32768
#define RAM_SIZE 24577
#define WORD_SIZE 16
#define SCREEN_ADDR 0x4000
#define KEYBD_ADDR 0x6000

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
    /* Read-only instruction memory.
     * Since .hack ROMs are actually just ASCII files with character 1s and 0s,
     * we keep the ROM in character format because it will actually make parsing
     * opcodes easier in the future.
     */
    char rom[ROM_SIZE][WORD_SIZE + 1];
    int program_size;

    // Random-access memory
    int16_t ram[RAM_SIZE];

    // A (address), D, and program counter CPU registers
    int16_t a_reg, d_reg, pc;
} Hack;

// Initialize the this
void hack_init(Hack *this);

// Execute the instruction located by the program counter
void hack_execute(Hack *this);

/* Load a file into the this's ROM
 * Returns false if unable to open file
 */
bool hack_load_rom(Hack *this, const char *filepath);

// Prints the contents of the this's ROM one instruction per line
void hack_print_rom(const Hack *this);

// Prints RAM registers with values != 0
void hack_print_ram(const Hack *this);