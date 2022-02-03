//jmwkrueger@gmail.com 2022 scalvin1
//VM Emulator based in parts on hackemu and other work in C by Kurtis Dinelle
//Context: nand2tetris

#define DEBUG 0
#define OVERRIDE_OS_FUNCTIONS 1

#ifndef EMULIB_H
#define EMULIB_H

#include <stdint.h>
#include <stdbool.h>
#include <unistd.h> //sleep

#define MEM_SIZE 32768
#define VM_SIZE 262144 //This should dynamically adjust to the number of VM instruction lines. For now MEM_SIZE*8
		//do some allocs and proper pointers in the struct otherwise stack problem
#define WORD_SIZE 16
#define SCREEN_ADDR 0x4000
#define KEYBD_ADDR 0x6000
#define DISPLAY_WIDTH 512
#define DISPLAY_HEIGHT 256
#define VM_MAXLABEL 128 //maximum number of characters in a label
#define VMSTATICVARS 256 //maximum number of characters in a label
#define MAX_FILES 64 //be generous
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
 * Finally, the Hack computer utilizes Harvard architecture hence the separated
 * program and data memory.
 */

// In the  VM code
// we always have instructions with 1, 2 or 3 arguments, separated by space
// I chose an arbitrary numbering here for the encoding...:
/*
0 push //3          push what n
1 pop //3           pop where n
2 call //3          call label nargs
3 function //3      function label nlocals
4 goto //2          goto label
5 if-goto //2       if-goto label
6 label //2         label label; Watch out for label scope in function it is defined (new book 8.2.1)
7 add //1
8 and //1
9 eq //1
10 gt //1
11 lt //1
12 neg //1
13 not //1
14 or //1
15 return //1
16 sub //1

//Segment encoding
0 argument
1 local
2 static  //only this one is special, the rest will live on the normal stack
3 constant
4 this
5 that
6 pointer
7 temp

Registers in the Hack CPU architecture:
RAM[0]  SP
RAM[1]  LCL
RAM[2]  ARG
RAM[3]  THIS
RAM[4]  THAT
RAM[5–12] TEMP
RAM[13–15] general purpose (only 3 words)
*/

typedef struct Vm
{
    // Read-only instruction memory.
    int16_t *vmarg0, *vmarg1, *vmarg2;
    int32_t *targetline; //where the labels are (for goto, if-goto, call)
    int16_t *filenum; //track which file we are in in 'line' pc for static element
    char **label; //[VM_SIZE][VM_MAXLABEL]

    // Random-access memory
    int16_t **statics; //this is where we put the 'static' memory segment
    int32_t *ram;
    	//Let's try this with int and clip to 16bit (&0xffff)
    	//That way we can sneakily have larger than 32k programs (save pc with more than 16bits)

    // Some VM variables
    int32_t program_size; //keep track of it here
    int32_t pc; //points to the next line to be processed
    int nfiles; //number of vmfiles (to simplify statics segment handling)
    //uint16_t sp; //not needed, we put everything else in ram and use ram[0] for SP
    int instructioncounter; //used for debugging
    int quitflag; //set this and the machine will be destroyed by the main loop
    int haltcount; //Need to return a few times from Sys.halt (internal) to update the screen before we quit

    //Screen.vm persistence (when handled by built-in functions in osfunctions.c)
    short currentcolor;

    //Memory.vm persistence
    int32_t *freelist;

} Vm;

// Gets an x and y coordinate from a screen address
void vm_get_coords(int *x, int *y, uint16_t addr);

// Initialize the machine
void vm_init(Vm *this);

// Initialize statics segment
void vm_init_statics(Vm *this, int nfiles);

// Generate label table
// return 0 for ok, 1 for missing >=1 label
int vm_init_labeltargets(Vm *this);

// Clear the 'ROM'
void vm_clear_vmcode(Vm *this);

// Clear the RAM
void vm_clear_ram(Vm *this);

// Destroy the machine
void vm_destroy(Vm *this);

// Execute the instruction located by the program counter
void vm_execute(Vm *this);

// Load a file into the machine's VM code 'ROM'
// Returns false if unable to open file
bool vm_load_vmcode(Vm *this, char *filepath);

// Prints the contents of the machine's VM code 'ROM' one instruction per line
void vm_print_vmcode(Vm *this);

// Prints RAM registers with values != 0
void vm_print_ram(Vm *this);

// Prints statics RAM registers with values != 0
void vm_print_statics(Vm *this);

// Checks if the function can be handled by built-in code.
// Returns 1 if it got handled, 0 if not
int check_os_function(Vm *this);
#endif
