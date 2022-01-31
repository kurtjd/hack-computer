//jmwkrueger@gmail.com 2022 scalvin1
//VM Emulator based in parts on hackemu and other work in C by Kurtis Dinelle
//Context: nand2tetris

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <dirent.h>

#include <SDL2/SDL.h>
#include "vmemulib.h"

#define TITLE "VM Emulator"
#define FRAME_RATE 60
#define CPU_FREQ 1000
#define OFF_COLOR 0xFFFFFF
#define ON_COLOR 0x000000


#define VM_MAX_LINE 1024
#define VM_MAX_ARGS 3
#define VM_MAX_ARG_LEN 1024
#define VM_ARG_DELIM " "

#define STACK_START_ADDR 256
#define TEMP_START_ADDR 5

char FOLDER_NAME[FILENAME_MAX];
char FILES[MAX_FILES][FILENAME_MAX] = {'\0'}; //also move this into the Vm struct some time
//int NUM_FILES = 0;

//Turn the type of memory segment from push and pop into a number for storing in Vm->vmarg1[]
uint16_t decode_segment(char *seg)
{
	if(strcmp(seg, "argument") == 0) return 0;
	if(strcmp(seg, "local") == 0) return 1;
	if(strcmp(seg, "static") == 0) return 2;
	if(strcmp(seg, "constant") == 0) return 3;
	if(strcmp(seg, "this") == 0) return 4;
	if(strcmp(seg, "that") == 0) return 5;
	if(strcmp(seg, "pointer") == 0) return 6;
	if(strcmp(seg, "temp") == 0) return 7;
	return -1;
}

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
void draw_display(const Vm *machine, SDL_Window *window, SDL_Surface *surface)
{
    for (int i = SCREEN_ADDR; i < KEYBD_ADDR; i++)
    {
        int x, y;
        vm_get_coords(&x, &y, i);

        for (int j = 0; j < WORD_SIZE; j++)
        {
            int newx = (WORD_SIZE * x) + j;

            long color = ((machine->ram[i] >> j) & 1) ? ON_COLOR : OFF_COLOR;
            set_pixel(surface, newx, y, color);
        }
    }

    SDL_UpdateWindowSurface(window);
}

// Returns the proper key for the emulator to handle
int get_key(SDL_KeyCode key)
{
    //printf("get_key(): keycode is %d\n", key);
    if (key >= SDLK_F1 && key <= SDLK_F12)
    {
        return HACK_KEY_F + (key - SDLK_F1);
    }

    //Translate lower case to upper case to behave just like nand2tetris java CPU Emulator
    //Note: There could be further discrepancies here.
    if (key >= 97 && key <= 122) // a..z
    {
        return (key-32); //-> A..Z
    }

    switch (key)
    {
    case SDLK_BACKSPACE:
        return HACK_KEY_BACKSPACE;
    case SDLK_LEFT:
        return HACK_KEY_LEFT;
    case SDLK_UP:
        return HACK_KEY_UP;
    case SDLK_RIGHT:
        return HACK_KEY_RIGHT;
    case SDLK_DOWN:
        return HACK_KEY_DOWN;
    case SDLK_HOME:
        return HACK_KEY_HOME;
    case SDLK_END:
        return HACK_KEY_END;
    case SDLK_PAGEUP:
        return HACK_KEY_PAGEUP;
    case SDLK_PAGEDOWN:
        return HACK_KEY_PAGEDOWN;
    case SDLK_INSERT:
        return HACK_KEY_INSERT;
    case SDLK_DELETE:
        return HACK_KEY_DELETE;
    case SDLK_ESCAPE:
        return HACK_KEY_ESCAPE;
    default:
        return key;
    }
}

// Checks for key presses/releases and a quit event.
bool handle_input(Vm *machine, SDL_Event *e)
{
    while (SDL_PollEvent(e))
    {
        switch (e->type)
        {
        case SDL_QUIT:
            return false;
            break;
        case SDL_KEYDOWN:
        {
            machine->ram[KEYBD_ADDR] = get_key(e->key.keysym.sym);
            break;
        }
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

// Remove all comments from the line
void trim_comments(char *line)
{
    for (size_t i = 0; i < strlen(line); i++)
    {
        if (line[i] == '/' && line[i + 1] == '/')
        {
            line[i] = '\0';
            break;
        }
    }
}

// Remove newline characters from the line.
void trim_nl(char *str)
{
    char buf[VM_MAX_LINE] = "";
    for (size_t i = 0; i < strlen(str); i++)
    {
        if (str[i] != '\n' && str[i] != '\r')
        {
            buf[strlen(buf)] = str[i];
        }
    }
    strcpy(str, buf);
}

// Check if line is empty
bool line_is_empty(const char *line)
{
    while (*line != '\0')
    {
        if (!isspace(*line))
        {
            return false;
        }

        line++;
    }

    return true;
}

// WIP just store in memory, representation TBD
bool parse(Vm *this, char *line, int filenum, /* AsmProg *prog, const char *filename, */ char *cur_func, char *cur_subfun)
{
    // The 'arguments' of a line (the instruction itself plus additional arguments)
    char args[VM_MAX_ARGS][VM_MAX_ARG_LEN] = {'\0'};
    char temp_line[VM_MAX_LINE];

    // Split the line into arguments based on a delimeter
    strncpy(temp_line, line, VM_MAX_LINE);
    char *token = strtok(temp_line, VM_ARG_DELIM);

    int i = 0;
    while (token != NULL && i < VM_MAX_ARGS)
    {
        strcpy(args[i++], token);
        token = strtok(NULL, VM_ARG_DELIM);
    }

    if (strcmp(args[0], "push") == 0)
    {
    	this->vmarg0[this->pc] = 0; //code for push
    	this->vmarg1[this->pc] = decode_segment(args[1]);
    	this->vmarg2[this->pc] = atoi(args[2]); //Turn the number that this part of the instruction string into an int
    	strncpy(this->label[this->pc], line, VM_MAXLABEL); //keep a copy of the line for debugging (wherever we do not need the label)
    	if(DEBUG) printf("parse push:pc=%d, %hi %hi %hi .. %s\n", this->pc, this->vmarg0[this->pc],
    		this->vmarg1[this->pc], this->vmarg2[this->pc], this->label[this->pc]);
	this->filenum[this->pc] = filenum; //important to know which static segment to target
    	this->pc++;
    }
    else if (strcmp(args[0], "pop") == 0)
    {
    	this->vmarg0[this->pc] = 1; //code for pop
    	this->vmarg1[this->pc] = decode_segment(args[1]);
    	this->vmarg2[this->pc] = atoi(args[2]);
    	strncpy(this->label[this->pc], line, VM_MAXLABEL);
    	if(DEBUG) printf("parse pop:pc=%d, %hi %hi %hi ..%s\n", this->pc, this->vmarg0[this->pc],
    		 this->vmarg1[this->pc], this->vmarg2[this->pc], this->label[this->pc]);
	this->filenum[this->pc] = filenum; //important to know which static segment to target
  	this->pc++;
    }
    else if (strcmp(args[0], "call") == 0)
    {
    	this->vmarg0[this->pc] = 2; //code for pop
    	this->vmarg1[this->pc] = -1; //not used
    	this->vmarg2[this->pc] = atoi(args[2]); //nargs
    	strncpy(this->label[this->pc], args[1], VM_MAXLABEL);
    	if(DEBUG) printf("parse call:pc=%d, %hi %hi %hi ..%s\n", this->pc, this->vmarg0[this->pc],
    		 this->vmarg1[this->pc], this->vmarg2[this->pc], this->label[this->pc]);
	this->filenum[this->pc] = filenum; //important to know which static segment to target
  	this->pc++;
    }
    else if (strcmp(args[0], "function") == 0)
    {
    	this->vmarg0[this->pc] = 3; //code for pop
    	this->vmarg1[this->pc] = -1; //not used
    	this->vmarg2[this->pc] = atoi(args[2]); //nlocals
    	strncpy(this->label[this->pc], args[1], VM_MAXLABEL);
    	strncpy(cur_subfun, args[1], VM_MAXLABEL);
    	if(DEBUG) printf("parse function:pc=%d, %hi %hi %hi ..%s\n", this->pc, this->vmarg0[this->pc],
    		 this->vmarg1[this->pc], this->vmarg2[this->pc], this->label[this->pc]);
	if(DEBUG) printf("  updated cur_subfun to |%s|\n", cur_subfun);
	this->filenum[this->pc] = filenum; //important to know which static segment to target
  	this->pc++;
    }
    // Branching
    else if (strcmp(args[0], "goto") == 0)
    {
    	this->vmarg0[this->pc] = 4; //code for goto
    	this->vmarg1[this->pc] = -1; //not used
    	this->vmarg2[this->pc] = -1;
//    	strcpy(this->label[this->pc],cur_func);//GLOB_
    	strcpy(this->label[this->pc],cur_subfun);
    	strcat(this->label[this->pc],"$");
    	strncat(this->label[this->pc], args[1], VM_MAXLABEL);
    	if(DEBUG) printf("parse goto:pc=%d, %hi %hi %hi ..%s\n", this->pc, this->vmarg0[this->pc],
    		 this->vmarg1[this->pc], this->vmarg2[this->pc], this->label[this->pc]);
	this->filenum[this->pc] = filenum; //important to know which static segment to target
  	this->pc++;
    }
    else if (strcmp(args[0], "if-goto") == 0)
    {
    	this->vmarg0[this->pc] = 5; //code for if-goto
    	this->vmarg1[this->pc] = -1; //not used
    	this->vmarg2[this->pc] = -1;
//    	strcpy(this->label[this->pc],cur_func);
    	strcpy(this->label[this->pc],cur_subfun);
    	strcat(this->label[this->pc],"$");
    	strncat(this->label[this->pc], args[1], VM_MAXLABEL);
    	if(DEBUG) printf("parse if-goto:pc=%d, %hi %hi %hi ..%s\n", this->pc, this->vmarg0[this->pc],
    		 this->vmarg1[this->pc], this->vmarg2[this->pc], this->label[this->pc]);
	this->filenum[this->pc] = filenum; //important to know which static segment to target
  	this->pc++;
    }
    else if (strcmp(args[0], "label") == 0)
    {
    	this->vmarg0[this->pc] = 6; //code for label
    	this->vmarg1[this->pc] = -1; //not used
    	this->vmarg2[this->pc] = -1;
//    	strcpy(this->label[this->pc],cur_func);
    	strcpy(this->label[this->pc],cur_subfun);
    	strcat(this->label[this->pc],"$");
    	strncat(this->label[this->pc], args[1], VM_MAXLABEL);
    	if(DEBUG) printf("parse label:pc=%d, %hi %hi %hi ..%s\n", this->pc, this->vmarg0[this->pc],
    		 this->vmarg1[this->pc], this->vmarg2[this->pc], this->label[this->pc]);
	this->filenum[this->pc] = filenum; //important to know which static segment to target
  	this->pc++;
    }

    // Arithmetic
    else if (strcmp(args[0], "add") == 0)
    {
    	this->vmarg0[this->pc] = 7; //code for add
    	this->vmarg1[this->pc] = -1; //not used
    	this->vmarg2[this->pc] = -1;
    	if(DEBUG) printf("parse add:pc=%d, %hi %hi %hi\n", this->pc, this->vmarg0[this->pc],
    		this->vmarg1[this->pc], this->vmarg2[this->pc]);
	this->filenum[this->pc] = filenum; //important to know which static segment to target
  	this->pc++;
    }
    else if (strcmp(args[0], "and") == 0)
    {
    	this->vmarg0[this->pc] = 8; //code for and
    	this->vmarg1[this->pc] = -1; //not used
    	this->vmarg2[this->pc] = -1;
    	if(DEBUG) printf("parse and:pc=%d, %hi %hi %hi\n", this->pc, this->vmarg0[this->pc],
    		this->vmarg1[this->pc], this->vmarg2[this->pc]);
	this->filenum[this->pc] = filenum; //important to know which static segment to target
  	this->pc++;
    }
    else if (strcmp(args[0], "eq") == 0)
    {
    	this->vmarg0[this->pc] = 9; //code for eq
    	this->vmarg1[this->pc] = -1; //not used
    	this->vmarg2[this->pc] = -1;
    	if(DEBUG) printf("parse eq:pc=%d, %hi %hi %hi\n", this->pc, this->vmarg0[this->pc],
    		this->vmarg1[this->pc], this->vmarg2[this->pc]);
	this->filenum[this->pc] = filenum; //important to know which static segment to target
  	this->pc++;
    }
    else if (strcmp(args[0], "gt") == 0)
    {
    	this->vmarg0[this->pc] = 10; //code for gt
    	this->vmarg1[this->pc] = -1; //not used
    	this->vmarg2[this->pc] = -1;
    	if(DEBUG) printf("parse gt:pc=%d, %hi %hi %hi\n", this->pc, this->vmarg0[this->pc],
    		this->vmarg1[this->pc], this->vmarg2[this->pc]);
	this->filenum[this->pc] = filenum; //important to know which static segment to target
  	this->pc++;
    }
    else if (strcmp(args[0], "lt") == 0)
    {
    	this->vmarg0[this->pc] = 11; //code for lt
    	this->vmarg1[this->pc] = -1; //not used
    	this->vmarg2[this->pc] = -1;
    	if(DEBUG) printf("parse lt:pc=%d, %hi %hi %hi\n", this->pc, this->vmarg0[this->pc],
    		this->vmarg1[this->pc], this->vmarg2[this->pc]);
	this->filenum[this->pc] = filenum; //important to know which static segment to target
  	this->pc++;
    }
    else if (strcmp(args[0], "neg") == 0)
    {
    	this->vmarg0[this->pc] = 12; //code for neg
    	this->vmarg1[this->pc] = -1; //not used
    	this->vmarg2[this->pc] = -1;
    	if(DEBUG) printf("parse neg:pc=%d, %hi %hi %hi\n", this->pc, this->vmarg0[this->pc],
    		this->vmarg1[this->pc], this->vmarg2[this->pc]);
	this->filenum[this->pc] = filenum; //important to know which static segment to target
  	this->pc++;
    }
    else if (strcmp(args[0], "not") == 0)
    {
       	this->vmarg0[this->pc] = 13; //code for not
    	this->vmarg1[this->pc] = -1; //not used
    	this->vmarg2[this->pc] = -1;
    	if(DEBUG) printf("parse not:pc=%d, %hi %hi %hi\n", this->pc, this->vmarg0[this->pc],
    		this->vmarg1[this->pc], this->vmarg2[this->pc]);
	this->filenum[this->pc] = filenum; //important to know which static segment to target
  	this->pc++;
    }

    else if (strcmp(args[0], "or") == 0)
    {
    	this->vmarg0[this->pc] = 14; //code for or
    	this->vmarg1[this->pc] = -1; //not used
    	this->vmarg2[this->pc] = -1;
    	if(DEBUG) printf("parse or:pc=%d, %hi %hi %hi\n", this->pc, this->vmarg0[this->pc],
    		this->vmarg1[this->pc], this->vmarg2[this->pc]);
	this->filenum[this->pc] = filenum; //important to know which static segment to target
  	this->pc++;
    }
    else if (strcmp(args[0], "return") == 0)
    {
    	this->vmarg0[this->pc] = 15; //code for return
    	this->vmarg1[this->pc] = -1; //not used
    	this->vmarg2[this->pc] = -1;
    	if(DEBUG) printf("parse return:pc=%d, %hi %hi %hi\n", this->pc, this->vmarg0[this->pc],
    		this->vmarg1[this->pc], this->vmarg2[this->pc]);
	this->filenum[this->pc] = filenum; //important to know which static segment to target
  	this->pc++;
    }
    else if (strcmp(args[0], "sub") == 0)
    {
    	this->vmarg0[this->pc] = 16; //code for sub
    	this->vmarg1[this->pc] = -1; //not used
    	this->vmarg2[this->pc] = -1;
    	if(DEBUG) printf("parse sub:pc=%d, %hi %hi %hi\n", this->pc, this->vmarg0[this->pc],
    		this->vmarg1[this->pc], this->vmarg2[this->pc]);
	this->filenum[this->pc] = filenum; //important to know which static segment to target
  	this->pc++;
    }
    else
    {
        fprintf(stderr, "Unrecognized instruction.\n");
        return false;
    }
    if(this->pc > this->program_size) this->program_size = this->pc; //increase program_size to this->pc
    return true;
}

// Gets a list of VM files from a filepath
int get_files(const char *filepath)
{
    DIR *d;
    int nfiles = 0;
    struct dirent *dir;
    d = opendir(filepath);

    /* If path is directory read in all the VM files, otherwise treat path as
     * the file itself
     */
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            char *fname = dir->d_name;
            int flen = strlen(fname);

            if (strcmp(fname + (flen - 3), ".vm") == 0)
            {
                sprintf(FILES[nfiles++], "%s/%s", filepath, fname);
            }
        }

        char folder[FILENAME_MAX];
        strcpy(folder, filepath);

        // Save the folder name for later
        if (folder[strlen(folder) - 1] == '/')
        {
            folder[strlen(folder) - 1] = '\0';
        }
        if (strrchr(folder, '/') != NULL)
        {
            strcpy(FOLDER_NAME, strrchr(folder, '/') + 1);
        }
        else
        {
            strcpy(FOLDER_NAME, folder);
        }

        closedir(d);
    }
    else
    {
        strcpy(FILES[0], filepath);
        nfiles = 1;
    }
    return nfiles;
}

// Gets the filename from a filepath
void get_filename(const char *filepath, char *filename)
{
    // Get all the characters after the last / and before the last .
    if (strrchr(filepath, '/') != NULL)
    {
        strcpy(filename, strrchr(filepath, '/') + 1);
    }
    else
    {
        strcpy(filename, filepath);
    }

    for (size_t i = 0; i < strlen(filename); i++)
    {
        if (filename[i] == '.')
        {
            filename[i] = '\0';
            break;
        }
    }
}

bool read_vm_files(Vm *this)
{
    //add_bootstrap(prog);
    this->vmarg0[this->pc] = 2; //code for call
    this->vmarg1[this->pc] = -1; //not used
    this->vmarg2[this->pc] = 0; //nargs
    strncpy(this->label[this->pc], "Sys.init", VM_MAXLABEL);
    if(DEBUG) printf("initial line/ bootstrap: call:pc=%d, %hi %hi %hi ..%s\n", this->pc, this->vmarg0[this->pc],
    	 this->vmarg1[this->pc], this->vmarg2[this->pc], this->label[this->pc]);
    this->filenum[this->pc] = 0; //important to know which static segment to target IRRELEVANT HERE
    this->pc++;

    //add Sys.halt, just in case
    this->vmarg0[this->pc] = 2; //code for call
    this->vmarg1[this->pc] = -1; //not used
    this->vmarg2[this->pc] = 0; //nargs
    strncpy(this->label[this->pc], "Sys.halt", VM_MAXLABEL);
    if(DEBUG) printf("initial line/ bootstrap: call:pc=%d, %hi %hi %hi ..%s\n", this->pc, this->vmarg0[this->pc],
    	 this->vmarg1[this->pc], this->vmarg2[this->pc], this->label[this->pc]);
    this->filenum[this->pc] = 0; //important to know which static segment to target IRRELEVANT HERE
    this->pc++;


    // read all the files
    printf("read_vm_files(): %d files\n", this->nfiles);
    for (int i = 0; i < this->nfiles; i++)
    {
    	char cur_subfun[VM_MAXLABEL];
        char filename[FILENAME_MAX];
        get_filename(FILES[i], filename);

	cur_subfun[0] = '\0';
	printf("read_vm_files(): Opening %s\n", FILES[i]);
        FILE *fp = fopen(FILES[i], "r");
        if (fp == NULL)
        {
            fprintf(stderr, "Unable to open %s\n", FILES[i]);
            return false;
        }

        // Tracks the current function we are in to generate unique labels
	// WIP: label scope is current function. Maybe we need to prepend every label we find with the function name to avoid trouble?
        char cur_func[FILENAME_MAX + VM_MAX_LINE];
        sprintf(cur_func, "GLOB$%s", filename);

        char line[VM_MAX_LINE] = {'\0'};
        while (fgets(line, VM_MAX_LINE, fp) != NULL)
        {
            // Strip comments and newline characters
            trim_comments(line);
            trim_nl(line);

            // Disregard blank lines
            if (!line_is_empty(line))
            {
                if (!parse(this, line, i,/* prog, filename, */ cur_func, cur_subfun))
                {
                    return false;
                }
            }
        }

        fclose(fp);
    }
    printf("read_vm_files(): Finished reading %d files. Total number of instructions %d\n", this->nfiles, this->program_size);

    //reset the program counter
    this->pc = 0;
    return true;
}


int main(int argc, char **argv)
{
    char vm_path[FILENAME_MAX];

    if (argc != 2)
    {
        fprintf(stderr, "Usage: ./vmemu <path-to-files>\n");
        return 1;
    }
    else
    {
        strncpy(vm_path, argv[1], FILENAME_MAX);
        vm_path[FILENAME_MAX - 1] = '\0';
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

    Vm machine;
    vm_init(&machine);

    int nfiles = get_files(argv[1]); //from vmtranslator

    vm_init_statics(&machine, nfiles);

    if (!read_vm_files(&machine))
    {
        vm_destroy(&machine);
        clean_exit(window, surface, 1);
    }
    vm_init_labeltargets(&machine);
    if(DEBUG) vm_print_vmcode(&machine);

    machine.pc = 0; //set pc to 0 to start at the beginning

    SDL_Event e;
    bool quit = false;
    while (!machine.quitflag && !quit && machine.pc < machine.program_size)
    {
        // Cap execution speed
        if (SDL_GetTicks() % (1000 / CPU_FREQ) <= 1)
        {
	    // WIP: Everything is set up. Now run the VM instructions. Implement them
            vm_execute(&machine);
        }

        // Cap input/draw rate
        if (SDL_GetTicks() % (1000 / FRAME_RATE) <= 1)
        {
            quit = !handle_input(&machine, &e);
            draw_display(&machine, window, surface);
        }
    }

    vm_destroy(&machine);
    clean_exit(window, surface, 0);
}
