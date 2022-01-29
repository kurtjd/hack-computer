#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vmemulib.h"

// Clear the ROM
static void vm_clear_vmcode(Vm *this)
{
    for (int i = 0; i < MEM_SIZE; i++)
    {
        this->vmarg0[i] = -1;
        this->vmarg1[i] = -1;
        this->vmarg2[i] = -1;
        this->label[i][0] = '\0';
    }
}

// Clear the RAM
static void vm_clear_ram(Vm *this)
{
    for (int i = 0; i < MEM_SIZE; i++)
    {
        this->ram[i] = 0;
    }
}

void vm_get_coords(int *x, int *y, uint16_t addr)
{
    *x = addr % 32;
    *y = (addr - SCREEN_ADDR) / 32;
}

void vm_init(Vm *this)
{
    int i;
    this->vmarg0 = calloc(VM_SIZE, sizeof(uint16_t));
    this->vmarg1 = calloc(VM_SIZE, sizeof(uint16_t));
    this->vmarg2 = calloc(VM_SIZE, sizeof(uint16_t));
    this->filenum = calloc(VM_SIZE, sizeof(uint16_t));
    this->ram = calloc(MEM_SIZE, sizeof(int32_t));
    this->label = calloc(VM_SIZE, sizeof(char*));
    this->statics = calloc(MAX_FILES, sizeof(uint16_t*));
    this->program_size = 0;
    this->pc = 0;
    this->nfiles = 0;

    for(i=0;i<VM_SIZE;i++){
    	this->label[i]=calloc(VM_MAXLABEL, sizeof(char));
    }

    vm_clear_vmcode(this);
    vm_clear_ram(this);
    this->ram[0] = 256; //set SP
    this->ram[KEYBD_ADDR] = 0;
}

void vm_init_statics(Vm *this, int nfiles)
{
    //intialize VMSTATICVARS variables for each file (NUM_FILES)
    this->nfiles = nfiles;
    for(int i=0;i<nfiles;i++){
    	this->statics[i]=calloc(VMSTATICVARS, sizeof(uint16_t));
    }
}

void vm_destroy(Vm *this)
{
	int i;
	if(this->vmarg0 != NULL)
		free(this->vmarg0);
	if(this->vmarg1 != NULL)
		free(this->vmarg1);
	if(this->vmarg2 != NULL)
		free(this->vmarg2);
	if(this->ram != NULL)
		free(this->ram);
	if(this->filenum != NULL)
		free(this->filenum);
	if(this->label != NULL){
		for(i=0;i<VM_SIZE;i++){
    			if(this->label[i] != NULL)
    				free(this->label[i]);
	    	}
	    	free(this->label);
    	}
	if(this->statics != NULL){
		for(i=0;i<this->nfiles;i++){
    			if(this->statics[i] != NULL)
    				free(this->statics[i]);
	    	}
	    	free(this->statics);
    	}
}

void vm_execute(Vm *this)
{
}

void vm_print_vmcode(const Vm *this)
{
    for (int i = 0; i < this->program_size; i++)
    {
        printf("line %d: %hi %hi %hi %s\n", i, this->vmarg0[i], this->vmarg1[i], this->vmarg2[i], this->label[i]);
    }
}

void vm_print_ram(const Vm *this)
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
