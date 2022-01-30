#define DEBUG 0

#ifndef EMULIB_H
#define EMULIB_H

#include <stdint.h>
#include <stdbool.h>

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
 *
 * The Hack platform also naturally assumes all data registers to be signed
 * hence the use of int16_t as opposed to uint16_t.
 *
 * Finally, the Hack computer utilizes Harvard architecture hence the separated
 * program and data memory.
 */

//WIP: needs thought for the vmcode
// we always have sets of 1, 2 or 3, separated by space
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


Register
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
    //THE FOLLOWING CAUSES STACK PROBLEMS AND CRASHES... FIX WITH POINTERS AND ALLOC
//    uint16_t vmarg0[VM_SIZE];
//    char label[VM_SIZE][VM_MAXLABEL];// = {'\0'}; //This is ugly. But I want to get going
//    uint16_t vmarg1[VM_SIZE];
//    uint16_t vmarg2[VM_SIZE];

    uint16_t *vmarg0, *vmarg1, *vmarg2;
    int32_t *targetline; //where the labels are (for goto, if-goto, call)
    uint16_t *filenum; //track which file we are in in 'line' pc for static element
    char **label; //[VM_SIZE][VM_MAXLABEL]
    
    // Random-access memory
//    int16_t ram[VM_SIZE];
    int32_t *ram; //Let's try this with int and clip to 16bit (&0xffff)
    		//That way we can sneakily have larger than 32k programs (save pc with more than 16bits)
		//remember to set type correctly in vm_init()

    //uint16_t 
    int32_t program_size;

    // A (address), D, and program counter CPU registers
    int32_t pc;
    //int16_t a_reg, d_reg;
    int nfiles;
    uint16_t **statics;
    //uint16_t sp; //use ram[0]

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

/* Load a file into the machine's ROM
 * Returns false if unable to open file
 */
bool vm_load_vmcode(Vm *this, const char *filepath);

// Prints the contents of the machine's ROM one instruction per line
void vm_print_vmcode(const Vm *this);

// Prints RAM registers with values != 0
void vm_print_ram(const Vm *this);

#endif
