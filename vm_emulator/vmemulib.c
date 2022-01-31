//jmwkrueger@gmail.com 2022 scalvin1
//VM Emulator based in parts on hackemu and other work in C by Kurtis Dinelle
//Context: nand2tetris

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "vmemulib.h"

// Clear the ROM
void vm_clear_vmcode(Vm *this)
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
void vm_clear_ram(Vm *this)
{
    for (int i = 0; i < MEM_SIZE; i++)
    {
        this->ram[i] = 0;
    }
    //set SP back to 256
    this->ram[0] = 256;
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
    this->targetline = calloc(VM_SIZE, sizeof(int32_t));
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
    this->instructioncounter = 0;
    this->quitflag = 0;
}

void vm_init_statics(Vm *this, int nfiles)
{
    //intialize VMSTATICVARS variables for each file (NUM_FILES)
    this->nfiles = nfiles;
    for(int i=0;i<nfiles;i++){
    	this->statics[i]=calloc(VMSTATICVARS, sizeof(uint16_t));
    }
}

// This returns 0 if all labeltargets can be resolved.
// Otherwise 1
// 1 would mean that we need to internally implement OS functions or that stuff is missing
// We will initially not implement internal OS functions for simplicity.
int vm_init_labeltargets(Vm *this)
{
	// We need a table to find the correct (VM) line number for specific labels (goto, if-goto, call)
	// And we need to check if we have a Main.main and Sys.init label

	// Count the labels out of curiosity
	int i, j, labelcount, functioncount, found, missingflag;
	i = 0;
	missingflag = 0;
	labelcount = 0;
	functioncount = 0;
	while(i<this->program_size){
		if(this->vmarg0[i] == 3){ //function
			functioncount++;
		}
		if(this->vmarg0[i] == 6){ //label
			labelcount++;
		}
		i++;
	}
	printf("vm_init_labeltargets(): found %d functions and %d labels.\n", functioncount, labelcount);

	// set the targetline for all goto if-goto and call instructions

	i = 0; //go through all instructions
	j = 0; //find the right label
	while(i<this->program_size){
		if(this->vmarg0[i] == 2 || this->vmarg0[i] == 4 || this->vmarg0[i] == 5){ //call, goto, if-goto
			//check for matching label or function now.
			j =0;
			found =0;
			while(j<this->program_size){
				if(this->vmarg0[j] == 3 ||this->vmarg0[j] == 6){ // if function label, check if it matches
					if(strcmp(this->label[i], this->label[j])==0){
						this->targetline[i] = j;
						found = 1;
						if(DEBUG){
							printf("  %d %s: %d\n", this->vmarg0[j], this->label[i], j); 
								//print the resolved target
						}
						break; //break the j while loop
					}
				}
				j++;
			}
			if(found == 0){
				printf("vm_init_labeltargets(): did not find target for label %s\n", this->label[i]);
				missingflag = 1;
			}
		}
		i++;
	}
	return missingflag; 
}

void vm_destroy(Vm *this)
{
	int i;
	if(this->vmarg0 != NULL) free(this->vmarg0);
	if(this->vmarg1 != NULL) free(this->vmarg1);
	if(this->vmarg2 != NULL) free(this->vmarg2);
	if(this->ram != NULL) free(this->ram);
	if(this->filenum != NULL) free(this->filenum);
	if(this->targetline != NULL) free(this->targetline);
	if(this->label != NULL){
		for(i=0;i<VM_SIZE;i++){
    			if(this->label[i] != NULL) free(this->label[i]);
	    	}
	    	free(this->label);
    	}
	if(this->statics != NULL){
		for(i=0;i<this->nfiles;i++){
    			if(this->statics[i] != NULL) free(this->statics[i]);
	    	}
	    	free(this->statics);
    	}
}

void vm_execute_label(Vm *this)
{
	this->pc++; //just simply skip the label instruction
}

void vm_execute_function(Vm *this)
{
	int i, k;
	k = this->vmarg2[this->pc]; //k local variables to clear
	for(i=0;i<k;i++){
		this->ram[this->ram[0]] = 0;
		this->ram[0]++;
	}
	this->pc++;
}

void vm_execute_call(Vm *this)
{
	int line;

	//handle simple OS functions directly
	if(OVERRIDE_OS_FUNCTIONS){
		// Math.vm
		if(strcmp(this->label[this->pc], "Math.init") == 0){
			if(DEBUG) printf("vm_execute_call(): Handling Math.init\n");
			this->pc++; //just do nothing, we simply don't need to set anything up. All OS Math functions handled internally.
			return;
		} else if(strcmp(this->label[this->pc], "Math.multiply") == 0){
			if(DEBUG) printf("vm_execute_call(): Handling Math.multiply\n");
			short a = (short) this->ram[this->ram[0]-2];
			short b = (short) this->ram[this->ram[0]-1];
			this->ram[this->ram[0]-2]= (int)(a*b); //a=a*b
			this->ram[0]--; //SP--
			this->pc++;
			return;
		}else if(strcmp(this->label[this->pc], "Math.divide") == 0){
			if(DEBUG) printf("vm_execute_call(): Handling Math.divide\n");
			short a = (short) this->ram[this->ram[0]-2];
			short b = (short) this->ram[this->ram[0]-1];
			this->ram[this->ram[0]-2]= (int)(a/b); //a=a/b
			this->ram[0]--; //SP--
			this->pc++;
			return;
		}else if(strcmp(this->label[this->pc], "Math.sqrt") == 0){
			if(DEBUG) printf("vm_execute_call(): Handling Math.sqrt\n");
			short a = (short) this->ram[this->ram[0]-1];
			if(a<0){
				printf("internal Math.sqrt() --> negative input error\n");
				exit(1);
			}
			this->ram[this->ram[0]-1]= (int)((short)(sqrt(a))); //a=a/b
			this->pc++;
			return;
		}else if(strcmp(this->label[this->pc], "Math.min") == 0){
			if(DEBUG) printf("vm_execute_call(): Handling Math.min\n");
			short a = (short) this->ram[this->ram[0]-2];
			short b = (short) this->ram[this->ram[0]-1];
			if(a<b){
				this->ram[this->ram[0]-2]= (int) a; //a=a/b
			}else{
				this->ram[this->ram[0]-2]= (int) b;
			}
			this->ram[0]--; //SP--
			this->pc++;
			return;
                }else if(strcmp(this->label[this->pc], "Math.max") == 0){
			if(DEBUG) printf("vm_execute_call(): Handling Math.max\n");
			short a = (short) this->ram[this->ram[0]-2];
			short b = (short) this->ram[this->ram[0]-1];
			if(a<b){
				this->ram[this->ram[0]-2]= (int) b;
			}else{
				this->ram[this->ram[0]-2]= (int) a;
			}
			this->ram[0]--; //SP--
			this->pc++;
			return;
		}else if(strcmp(this->label[this->pc], "Math.abs") == 0){
			if(DEBUG) printf("vm_execute_call(): Handling Math.abs\n");
			short a = (short) this->ram[this->ram[0]-1];
			if(a<0){
				this->ram[this->ram[0]-1]= (int)(-a); //a=-a
			}
			this->pc++;
			return;
		}else

		// Now Sys.vm
		if(strcmp(this->label[this->pc], "Sys.halt") == 0){
                        if(DEBUG) printf("vm_execute_call(): Handling Sys.halt\n");
                        sleep(1); //seconds
                        this->quitflag = 1;
                        //this->pc++;
                        return;
		}
	}

	//save 'environment' on stack
	this->ram[this->ram[0]] = this->pc+1;//push return address to stack
	this->ram[0]++;
	this->ram[this->ram[0]] = this->ram[1];//push LCL
	this->ram[0]++;
	this->ram[this->ram[0]] = this->ram[2];//push ARG
	this->ram[0]++;
	this->ram[this->ram[0]] = this->ram[3];//push THIS
	this->ram[0]++;
	this->ram[this->ram[0]] = this->ram[4];//push THAT
	this->ram[0]++;
	this->ram[2] = this->ram[0]-5-this->vmarg2[this->pc]; //ARG = SP-n-5
	this->ram[1] = this->ram[0]; //LCL = SP

	//goto f
	line = this->targetline[this->pc];
	this->pc=line;
}

void vm_execute_return(Vm *this)
{
	int frame, ret;
	frame = this->ram[1];//LCL
	ret = this->ram[frame - 5];
	this->ram[this->ram[2]] = this->ram[this->ram[0]-1]; // *ARG = pop
	this->ram[0] = this->ram[2]+1; //SP = ARG+1
	this->ram[4] = this->ram[frame - 1]; //THAT
	this->ram[3] = this->ram[frame - 2]; //THAT
	this->ram[2] = this->ram[frame - 3]; //ARG
	this->ram[1] = this->ram[frame - 4]; //LCL
	this->pc=ret; //goto return address
}

void vm_execute_add(Vm *this) 
{
	short a = (short) this->ram[this->ram[0]-2];
	short b = (short) this->ram[this->ram[0]-1];
	this->ram[this->ram[0]-2]= (int)((short)(a+b)); //a=a+b
	this->ram[0]--; //SP--
	this->pc++;
}

void vm_execute_sub(Vm *this) 
{
	short a = (short) this->ram[this->ram[0]-2];
	short b = (short) this->ram[this->ram[0]-1];
	this->ram[this->ram[0]-2]= (int)((short)(a-b)); //a=a-b
	this->ram[0]--; //SP--
	this->pc++;
}

void vm_execute_and(Vm *this) 
{
	short a = (short) this->ram[this->ram[0]-2];
	short b = (short) this->ram[this->ram[0]-1];
	this->ram[this->ram[0]-2]= (int)((short)(a&b)); //a=a&b
	this->ram[0]--; //SP--
	this->pc++;
}

void vm_execute_or(Vm *this) 
{
	short a = (short) this->ram[this->ram[0]-2];
	short b = (short) this->ram[this->ram[0]-1];
	this->ram[this->ram[0]-2]= (int)((short)(a|b)); //a=a|b
	this->ram[0]--; //SP--
	this->pc++;
}

void vm_execute_eq(Vm *this) 
{
	short a = (short) this->ram[this->ram[0]-2];
	short b = (short) this->ram[this->ram[0]-1];
	if(a==b){// true --> -1
		this->ram[this->ram[0]-2]= -1;
	}else{// false --> 0
		this->ram[this->ram[0]-2]= 0;
	}
	this->ram[0]--; //SP--
	this->pc++;
}

void vm_execute_lt(Vm *this) 
{
	short a = (short) this->ram[this->ram[0]-2];
	short b = (short) this->ram[this->ram[0]-1];
	if(a<b){// true --> -1
		this->ram[this->ram[0]-2]= -1;
	}else{// false --> 0
		this->ram[this->ram[0]-2]= 0;
	}
	this->ram[0]--; //SP--
	this->pc++;
}

void vm_execute_gt(Vm *this) 
{
	short a = (short) this->ram[this->ram[0]-2];
	short b = (short) this->ram[this->ram[0]-1];
	if(a>b){// true --> -1
		this->ram[this->ram[0]-2]= -1;
	}else{// false --> 0
		this->ram[this->ram[0]-2]= 0;
	}
	this->ram[0]--; //SP--
	this->pc++;
}

void vm_execute_not(Vm *this)
{
	short a = (short) this->ram[this->ram[0]-1];
	this->ram[this->ram[0]-1]= (int)(~a); //a=~a
	this->pc++;
}

void vm_execute_neg(Vm *this)
{
	short a = (short) this->ram[this->ram[0]-1];
	this->ram[this->ram[0]-1]= (int)(-a); //a=-a
	this->pc++;
}

void vm_execute_goto(Vm *this) 
{
	int line;
	line = this->targetline[this->pc];
	this->pc=line;
}

void vm_execute_ifgoto(Vm *this) 
{
	int line;
	if((short) (this->ram[this->ram[0] -1]) == (short)(-1)){
		line = this->targetline[this->pc];
		this->pc=line;
	} else {
		this->pc++;
	}
	this->ram[0]--; //SP--
}

/*
//Segment encoding
0 argument
1 local
2 static  //only this one is special, the rest will live on the normal stack
3 constant
4 this
5 that
6 pointer
7 temp
CPU Register
RAM[0]  SP
RAM[1]  LCL
RAM[2]  ARG
RAM[3]  THIS
RAM[4]  THAT
RAM[5–12] TEMP
RAM[13–15] general purpose (only 3 words)
*/
void vm_execute_push(Vm *this)
{
	short i = 0; //pushvalue
	//Get the push value
	switch (this->vmarg1[this->pc])
	{
	case 0: //ARG RAM[2]
		i = (short) this->ram[this->ram[2]+this->vmarg2[this->pc]];
		break;
	case 1: //LCL RAM[1]
		i = (short) this->ram[this->ram[1]+this->vmarg2[this->pc]];
		break;
	case 2: //static (special)
		i = (short) this->statics[this->filenum[this->pc]][this->vmarg2[this->pc]];
		break;
	case 3: //constant 
		i = (short) this->vmarg2[this->pc]; 
		break;
	case 4: //THIS RAM[3]
		i = (short) this->ram[this->ram[3]+this->vmarg2[this->pc]];
		break;
	case 5: //THAT RAM[4]
		i = (short) this->ram[this->ram[4]+this->vmarg2[this->pc]];
		break;
	case 6: //pointer [0] or [1]
		i = (short) this->ram[3+this->vmarg2[this->pc]];
		break;
	case 7: //TEMP
		i = (short) this->ram[5+this->vmarg2[this->pc]];
		break;
	default:
		printf("vm_execute_push(): unhandled case\n");
		exit(1);
	}

	//Push the value onto stack
	this->ram[this->ram[0]]= (int) i;

	//Increase SP
	this->ram[0]++;

	//Increase PC
	this->pc++;
}

void vm_execute_pop(Vm *this)
{
	short i; //popvalue
	//Get the pop value
	i = (short) this->ram[this->ram[0] - 1];
	this->ram[0]--; //SP--

	//put the value where it should go
	switch (this->vmarg1[this->pc])
	{
	case 0: //ARG RAM[2]
		this->ram[this->ram[2]+this->vmarg2[this->pc]] = (int) i;
		break;
	case 1: //LCL RAM[1]
		this->ram[this->ram[1]+this->vmarg2[this->pc]] = (int) i;
		break;
	case 2: //static (special)
		this->statics[this->filenum[this->pc]][this->vmarg2[this->pc]] = (int) i;
		break;
	case 3: //constant makes no sense
		printf("vm_execute_pop(): popping to constant makes no sense!\n");
		break;
	case 4: //THIS RAM[3]
		this->ram[this->ram[3]+this->vmarg2[this->pc]] = (int) i;
		break;
	case 5: //THAT RAM[4]
		this->ram[this->ram[4]+this->vmarg2[this->pc]] = (int) i;
		break;
	case 6: //pointer [0] or [1]
		this->ram[3+this->vmarg2[this->pc]] = (int) i;
		break;
	case 7: //TEMP
		this->ram[5+this->vmarg2[this->pc]] = (int) i;
		break;
	default:
		printf("vm_execute_pop(): unhandled case\n");
		exit(1);
	}

	//Increase PC
	this->pc++;
}

void vm_execute(Vm *this)
{
	//check line and process it.
	if(DEBUG) printf("   line: %d %s | ", this->pc, this->label[this->pc]);
	switch(this->vmarg0[this->pc])
	{
	case 0:
		vm_execute_push(this); //3 args; push segment index
		if(DEBUG) printf(" push SP: %d\n", this->ram[0]);
		break;
	case 1:
		vm_execute_pop(this); //3 args; pop segment index
		if(DEBUG) printf(" pop SP: %d\n", this->ram[0]);
		break;
	case 2:
		vm_execute_call(this); //3 args; call bla nargs
		if(DEBUG) printf(" call SP: %d\n", this->ram[0]);
		break;
	case 3:
		vm_execute_function(this); //3 args; function label nlocals
		if(DEBUG) printf(" fn SP: %d\n", this->ram[0]);
		break;
	case 4:
		vm_execute_goto(this); //2 args; goto label
		if(DEBUG) printf(" goto SP: %d\n", this->ram[0]);
		break;
	case 5:
		vm_execute_ifgoto(this); //2 args; if-goto label
		if(DEBUG) printf(" ifgoto SP: %d\n", this->ram[0]);
		break;
	case 6:
		vm_execute_label(this); //2 args; label label //This is a do nothing statement
		if(DEBUG) printf(" label SP: %d\n", this->ram[0]);
		break;
	case 7:
		vm_execute_add(this);
		if(DEBUG) printf(" add SP: %d\n", this->ram[0]);
		break;
	case 8:
		vm_execute_and(this);
		if(DEBUG) printf(" and SP: %d\n", this->ram[0]);
		break;
	case 9:
		vm_execute_eq(this);
		if(DEBUG) printf(" eq SP: %d\n", this->ram[0]);
		break;
	case 10:
		vm_execute_gt(this);
		if(DEBUG) printf(" gt SP: %d\n", this->ram[0]);
		break;
	case 11:
		vm_execute_lt(this);
		if(DEBUG) printf(" lt SP: %d\n", this->ram[0]);
		break;
	case 12:
		vm_execute_neg(this);
		if(DEBUG) printf(" neg SP: %d\n", this->ram[0]);
		break;
	case 13:
		vm_execute_not(this);
		if(DEBUG) printf(" not SP: %d\n", this->ram[0]);
		break;
	case 14:
		vm_execute_or(this);
		if(DEBUG) printf(" or SP: %d\n", this->ram[0]);
		break;
	case 15:
		vm_execute_return(this);
		if(DEBUG) printf("  ret SP: %d\n", this->ram[0]);
		break;
	case 16:
		vm_execute_sub(this);
		if(DEBUG) printf("  sub SP: %d\n", this->ram[0]);
		break;
	default:
		printf("vm_execute(): Panic! Unhandled VM instruction!\n");
		exit(1);
	}
	this->instructioncounter++;
}

void vm_print_vmcode(Vm *this)
{
    for (int i = 0; i < this->program_size; i++)
    {
        printf("line %d: %hi %hi %hi %s\n", i, this->vmarg0[i], this->vmarg1[i], this->vmarg2[i], this->label[i]);
    }
}

void vm_print_ram(Vm *this)
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
