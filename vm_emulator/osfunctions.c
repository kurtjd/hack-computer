//jmwkrueger@gmail.com 2022 scalvin1
//VM Emulator based in parts on hackemu and other work in C by Kurtis Dinelle
//Context: nand2tetris

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "vmemulib.h"

// Screen.vm
// white is false (0)
// black is true (-1)
void
screen_clearScreen(Vm *this){
 	int i;
	if(DEBUG) printf("vm_execute_call(): Handling Screen.clearScreen\n");
	for(i=16384;i<16384+8192;i++){
		this->ram[i]= 0;
	}
	this->ram[this->ram[0]]= 0; //push 0 (void retrun value still needs to return a 0
	this->ram[0]++; //SP++
	this->pc++;
}

void
screen_setColor(Vm *this){
	this->currentcolor = (short) this->ram[this->ram[0]-1];
	this->ram[this->ram[0]]= 0; //push 0 (void retrun value still needs to return a 0
	this->ram[0]++; //SP++
	this->pc++;
}

void
screen_drawPixel(Vm *this){
//TODO
	this->ram[this->ram[0]]= 0; //push 0 (void retrun value still needs to return a 0
	this->ram[0]++; //SP++
	this->pc++;
}

void
screen_drawLine(Vm *this){
//TODO
	this->ram[this->ram[0]]= 0; //push 0 (void retrun value still needs to return a 0
	this->ram[0]++; //SP++
	this->pc++;
}

void
screen_drawRectangle(Vm *this){
//TODO
	this->ram[this->ram[0]]= 0; //push 0 (void retrun value still needs to return a 0
	this->ram[0]++; //SP++
	this->pc++;
}

void
screen_drawCircle(Vm *this){
//TODO
	this->ram[this->ram[0]]= 0; //push 0 (void retrun value still needs to return a 0
	this->ram[0]++; //SP++
	this->pc++;
}

// Math.vm
void
math_init(Vm *this){
	if(DEBUG) printf("vm_execute_call(): Handling Math.init\n");
	this->pc++; //just do nothing, we simply don't need to set anything up. All OS Math functions handled internally.
}

void
math_multiply(Vm *this){
	if(DEBUG) printf("vm_execute_call(): Handling Math.multiply\n");
	short a = (short) this->ram[this->ram[0]-2];
	short b = (short) this->ram[this->ram[0]-1];
	this->ram[this->ram[0]-2]= (int)(a*b); //a=a*b
	this->ram[0]--; //SP--
	this->pc++;
}

void
math_divide(Vm *this){
	if(DEBUG) printf("vm_execute_call(): Handling Math.divide\n");
	short a = (short) this->ram[this->ram[0]-2];
	short b = (short) this->ram[this->ram[0]-1];
	this->ram[this->ram[0]-2]= (int)(a/b); //a=a/b
	this->ram[0]--; //SP--
	this->pc++;
}

void
math_sqrt(Vm *this){
	if(DEBUG) printf("vm_execute_call(): Handling Math.sqrt\n");
	short a = (short) this->ram[this->ram[0]-1];
	if(a<0){
		printf("internal Math.sqrt() --> negative input error\n");
		exit(1);
	}
	this->ram[this->ram[0]-1]= (int)((short)(sqrt(a))); //a=a/b
	this->pc++;
}

void
math_min(Vm *this){
	if(DEBUG) printf("vm_execute_call(): Handling Math.min\n");
	short a = (short) this->ram[this->ram[0]-2];
	short b = (short) this->ram[this->ram[0]-1];
	if(a<b){
		this->ram[this->ram[0]-2]= (int) a; //a=a/b
	}else{
		this->ram[this->ram[0]-2]= (int) b;
	}
	this->ram[0]--; //SP-- (one less than before on the stack)
	this->pc++;
}

void
math_max(Vm *this){
	if(DEBUG) printf("vm_execute_call(): Handling Math.max\n");
	short a = (short) this->ram[this->ram[0]-2];
	short b = (short) this->ram[this->ram[0]-1];
	if(a<b){
		this->ram[this->ram[0]-2]= (int) b;
	}else{
		this->ram[this->ram[0]-2]= (int) a;
	}
	this->ram[0]--; //SP-- (one less than before on the stack)
	this->pc++;
}

void
math_abs(Vm *this){
	if(DEBUG) printf("vm_execute_call(): Handling Math.abs\n");
	short a = (short) this->ram[this->ram[0]-1];
	if(a<0){
		this->ram[this->ram[0]-1]= (int)(-a); //a=-a
	}
	this->pc++;
}

void
sys_halt(Vm *this){
	if(this->haltcount == 0){
		if(DEBUG)printf("vm_execute_call(): Handling Sys.halt\n");
		this->haltcount++;
	}
	usleep(50000); //0.05s
	//else if(this->haltcount == 3000){
	//	sleep(2); //seconds
	//	this->quitflag = 1;
	//	//this->pc++;
	//}
	//this->haltcount++;
}

int
check_os_function(Vm *this){
	int handled = 0;

	// Math.vm
	if(strcmp(this->label[this->pc], "Math.init") == 0){
		math_init(this);
		handled++;
	} else if(strcmp(this->label[this->pc], "Math.multiply") == 0){
		math_multiply(this);
		handled++;
	}else if(strcmp(this->label[this->pc], "Math.divide") == 0){
		math_divide(this);
		handled++;
	}else if(strcmp(this->label[this->pc], "Math.sqrt") == 0){
		math_sqrt(this);
		handled++;
	}else if(strcmp(this->label[this->pc], "Math.min") == 0){
		math_min(this);
		handled++;
	}else if(strcmp(this->label[this->pc], "Math.max") == 0){
		math_max(this);
		handled++;
	}else if(strcmp(this->label[this->pc], "Math.abs") == 0){
		math_abs(this);
		handled++;
	}
	
	 
	// Sys.vm
	else if(strcmp(this->label[this->pc], "Sys.halt") == 0){
		sys_halt(this);
		handled++;
	}

	
	// Screen.vm
	else if(strcmp(this->label[this->pc], "Screen.clearScreen") == 0){
		screen_clearScreen(this);
		handled++;
	}
/*
	//TODO: Work in progress
	else if(strcmp(this->label[this->pc], "Screen.setColor") == 0){
		screen_setColor(this);
		handled++;
	}
	else if(strcmp(this->label[this->pc], "Screen.drawPixel") == 0){
		screen_drawPixel(this);
		handled++;
	}
	else if(strcmp(this->label[this->pc], "Screen.drawLine") == 0){
		screen_drawLine(this);
		handled++;
	}
	else if(strcmp(this->label[this->pc], "Screen.drawRectangle") == 0){
		screen_drawRectangle(this);
		handled++;
	}
	else if(strcmp(this->label[this->pc], "Screen.drawCircle") == 0){
		screen_drawCircle(this);
		handled++;
	}
*/
	return handled;
}
