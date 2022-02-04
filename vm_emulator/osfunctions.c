//jmwkrueger@gmail.com 2022 scalvin1
//VM Emulator based in parts on hackemu and other work in C by Kurtis Dinelle
//Context: nand2tetris

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "vmemulib.h"

// Output.vm

void 
create_char_map(short **map, int c, int line0, int line1, int line2, 
	int line3, int line4, int line5,
	int line6, int line7, int line8,
   	int line9, int line10) {
	map[c] = calloc(11, sizeof(short));
        map[c][0] = line0;
        map[c][1] = line1;
        map[c][2] = line2;
        map[c][3] = line3;
        map[c][4] = line4;
        map[c][5] = line5;
        map[c][6] = line6;
        map[c][7] = line7;
        map[c][8] = line8;
        map[c][9] = line9;
        map[c][10] = line10;
}

void 
init_charmap(Vm *this){
	if(this->charmap != NULL){
		printf("init_charmap(): charmap is not NULL\n");
		return;
	}
	short **charmap;
	charmap = calloc(127, sizeof(short*));

        create_char_map(charmap, 0, 63, 63, 63, 63, 63, 63, 63, 63, 63, 0, 0);
        create_char_map(charmap, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        create_char_map(charmap, 33, 12, 30, 30, 30, 12, 12, 0, 12, 12, 0, 0);
        create_char_map(charmap, 34, 54, 54, 20, 0, 0, 0, 0, 0, 0, 0, 0);
        create_char_map(charmap, 35, 0, 18, 18, 63, 18, 18, 63, 18, 18, 0, 0);
        create_char_map(charmap, 36, 12, 30, 51, 3, 30, 48, 51, 30, 12, 12, 0);
        create_char_map(charmap, 37, 0, 0, 35, 51, 24, 12, 6, 51, 49, 0, 0);
        create_char_map(charmap, 38, 12, 30, 30, 12, 54, 27, 27, 27, 54, 0, 0);
        create_char_map(charmap, 39, 12, 12, 6, 0, 0, 0, 0, 0, 0, 0, 0);
        create_char_map(charmap, 40, 24, 12, 6, 6, 6, 6, 6, 12, 24, 0, 0);
        create_char_map(charmap, 41, 6, 12, 24, 24, 24, 24, 24, 12, 6, 0, 0);
        create_char_map(charmap, 42, 0, 0, 0, 51, 30, 63, 30, 51, 0, 0, 0);
        create_char_map(charmap, 43, 0, 0, 0, 12, 12, 63, 12, 12, 0, 0, 0);
        create_char_map(charmap, 44, 0, 0, 0, 0, 0, 0, 0, 12, 12, 6, 0);
        create_char_map(charmap, 45, 0, 0, 0, 0, 0, 63, 0, 0, 0, 0, 0);
        create_char_map(charmap, 46, 0, 0, 0, 0, 0, 0, 0, 12, 12, 0, 0);
        create_char_map(charmap, 47, 0, 0, 32, 48, 24, 12, 6, 3, 1, 0, 0);
        create_char_map(charmap, 48, 12, 30, 51, 51, 51, 51, 51, 30, 12, 0, 0);
        create_char_map(charmap, 49, 12, 14, 15, 12, 12, 12, 12, 12, 63, 0, 0);
        create_char_map(charmap, 50, 30, 51, 48, 24, 12, 6, 3, 51, 63, 0, 0);
        create_char_map(charmap, 51, 30, 51, 48, 48, 28, 48, 48, 51, 30, 0, 0);
        create_char_map(charmap, 52, 16, 24, 28, 26, 25, 63, 24, 24, 60, 0, 0);
        create_char_map(charmap, 53, 63, 3, 3, 31, 48, 48, 48, 51, 30, 0, 0);
        create_char_map(charmap, 54, 28, 6, 3, 3, 31, 51, 51, 51, 30, 0, 0);
        create_char_map(charmap, 55, 63, 49, 48, 48, 24, 12, 12, 12, 12, 0, 0);
        create_char_map(charmap, 56, 30, 51, 51, 51, 30, 51, 51, 51, 30, 0, 0);
        create_char_map(charmap, 57, 30, 51, 51, 51, 62, 48, 48, 24, 14, 0, 0);
        create_char_map(charmap, 58, 0, 0, 12, 12, 0, 0, 12, 12, 0, 0, 0);
        create_char_map(charmap, 59, 0, 0, 12, 12, 0, 0, 12, 12, 6, 0, 0);
        create_char_map(charmap, 60, 0, 0, 24, 12, 6, 3, 6, 12, 24, 0, 0);
        create_char_map(charmap, 61, 0, 0, 0, 63, 0, 0, 63, 0, 0, 0, 0);
        create_char_map(charmap, 62, 0, 0, 3, 6, 12, 24, 12, 6, 3, 0, 0);
        create_char_map(charmap, 64, 30, 51, 51, 59, 59, 59, 27, 3, 30, 0, 0);
        create_char_map(charmap, 63, 30, 51, 51, 24, 12, 12, 0, 12, 12, 0, 0);
        create_char_map(charmap, 65, 12, 30, 51, 51, 63, 51, 51, 51, 51, 0, 0);
        create_char_map(charmap, 66, 31, 51, 51, 51, 31, 51, 51, 51, 31, 0, 0);
        create_char_map(charmap, 67, 28, 54, 35, 3, 3, 3, 35, 54, 28, 0, 0);
        create_char_map(charmap, 68, 15, 27, 51, 51, 51, 51, 51, 27, 15, 0, 0);
        create_char_map(charmap, 69, 63, 51, 35, 11, 15, 11, 35, 51, 63, 0, 0);
        create_char_map(charmap, 70, 63, 51, 35, 11, 15, 11, 3, 3, 3, 0, 0);
        create_char_map(charmap, 71, 28, 54, 35, 3, 59, 51, 51, 54, 44, 0, 0);
        create_char_map(charmap, 72, 51, 51, 51, 51, 63, 51, 51, 51, 51, 0, 0);
        create_char_map(charmap, 73, 30, 12, 12, 12, 12, 12, 12, 12, 30, 0, 0);
        create_char_map(charmap, 74, 60, 24, 24, 24, 24, 24, 27, 27, 14, 0, 0);
        create_char_map(charmap, 75, 51, 51, 51, 27, 15, 27, 51, 51, 51, 0, 0);
        create_char_map(charmap, 76, 3, 3, 3, 3, 3, 3, 35, 51, 63, 0, 0);
        create_char_map(charmap, 77, 33, 51, 63, 63, 51, 51, 51, 51, 51, 0, 0);
        create_char_map(charmap, 78, 51, 51, 55, 55, 63, 59, 59, 51, 51, 0, 0);
        create_char_map(charmap, 79, 30, 51, 51, 51, 51, 51, 51, 51, 30, 0, 0);
        create_char_map(charmap, 80, 31, 51, 51, 51, 31, 3, 3, 3, 3, 0, 0);
        create_char_map(charmap, 81, 30, 51, 51, 51, 51, 51, 63, 59, 30, 48, 0);
        create_char_map(charmap, 82, 31, 51, 51, 51, 31, 27, 51, 51, 51, 0, 0);
        create_char_map(charmap, 83, 30, 51, 51, 6, 28, 48, 51, 51, 30, 0, 0);
        create_char_map(charmap, 84, 63, 63, 45, 12, 12, 12, 12, 12, 30, 0, 0);
        create_char_map(charmap, 85, 51, 51, 51, 51, 51, 51, 51, 51, 30, 0, 0);
        create_char_map(charmap, 86, 51, 51, 51, 51, 51, 30, 30, 12, 12, 0, 0);
        create_char_map(charmap, 87, 51, 51, 51, 51, 51, 63, 63, 63, 18, 0, 0);
        create_char_map(charmap, 88, 51, 51, 30, 30, 12, 30, 30, 51, 51, 0, 0);
        create_char_map(charmap, 89, 51, 51, 51, 51, 30, 12, 12, 12, 30, 0, 0);
        create_char_map(charmap, 90, 63, 51, 49, 24, 12, 6, 35, 51, 63, 0, 0);
        create_char_map(charmap, 91, 30, 6, 6, 6, 6, 6, 6, 6, 30, 0, 0);
        create_char_map(charmap, 92, 0, 0, 1, 3, 6, 12, 24, 48, 32, 0, 0);
        create_char_map(charmap, 93, 30, 24, 24, 24, 24, 24, 24, 24, 30, 0, 0);
        create_char_map(charmap, 94, 8, 28, 54, 0, 0, 0, 0, 0, 0, 0, 0);
        create_char_map(charmap, 95, 0, 0, 0, 0, 0, 0, 0, 0, 0, 63, 0);
        create_char_map(charmap, 96, 6, 12, 24, 0, 0, 0, 0, 0, 0, 0, 0);
        create_char_map(charmap, 97, 0, 0, 0, 14, 24, 30, 27, 27, 54, 0, 0);
        create_char_map(charmap, 98, 3, 3, 3, 15, 27, 51, 51, 51, 30, 0, 0);
        create_char_map(charmap, 99, 0, 0, 0, 30, 51, 3, 3, 51, 30, 0, 0);
        create_char_map(charmap, 100, 48, 48, 48, 60, 54, 51, 51, 51, 30, 0, 0);
        create_char_map(charmap, 101, 0, 0, 0, 30, 51, 63, 3, 51, 30, 0, 0);
        create_char_map(charmap, 102, 28, 54, 38, 6, 15, 6, 6, 6, 15, 0, 0);
        create_char_map(charmap, 103, 0, 0, 30, 51, 51, 51, 62, 48, 51, 30, 0);
        create_char_map(charmap, 104, 3, 3, 3, 27, 55, 51, 51, 51, 51, 0, 0);
        create_char_map(charmap, 105, 12, 12, 0, 14, 12, 12, 12, 12, 30, 0, 0);
        create_char_map(charmap, 106, 48, 48, 0, 56, 48, 48, 48, 48, 51, 30, 0);
        create_char_map(charmap, 107, 3, 3, 3, 51, 27, 15, 15, 27, 51, 0, 0);
        create_char_map(charmap, 108, 14, 12, 12, 12, 12, 12, 12, 12, 30, 0, 0);
        create_char_map(charmap, 109, 0, 0, 0, 29, 63, 43, 43, 43, 43, 0, 0);
        create_char_map(charmap, 110, 0, 0, 0, 29, 51, 51, 51, 51, 51, 0, 0);
        create_char_map(charmap, 111, 0, 0, 0, 30, 51, 51, 51, 51, 30, 0, 0);
        create_char_map(charmap, 112, 0, 0, 0, 30, 51, 51, 51, 31, 3, 3, 0);
        create_char_map(charmap, 113, 0, 0, 0, 30, 51, 51, 51, 62, 48, 48, 0);
        create_char_map(charmap, 114, 0, 0, 0, 29, 55, 51, 3, 3, 7, 0, 0);
        create_char_map(charmap, 115, 0, 0, 0, 30, 51, 6, 24, 51, 30, 0, 0);
        create_char_map(charmap, 116, 4, 6, 6, 15, 6, 6, 6, 54, 28, 0, 0);
        create_char_map(charmap, 117, 0, 0, 0, 27, 27, 27, 27, 27, 54, 0, 0);
        create_char_map(charmap, 118, 0, 0, 0, 51, 51, 51, 51, 30, 12, 0, 0);
        create_char_map(charmap, 119, 0, 0, 0, 51, 51, 51, 63, 63, 18, 0, 0);
        create_char_map(charmap, 120, 0, 0, 0, 51, 30, 12, 12, 30, 51, 0, 0);
        create_char_map(charmap, 121, 0, 0, 0, 51, 51, 51, 62, 48, 24, 15, 0);
        create_char_map(charmap, 122, 0, 0, 0, 63, 27, 12, 6, 51, 63, 0, 0);
        create_char_map(charmap, 123, 56, 12, 12, 12, 7, 12, 12, 12, 56, 0, 0);
        create_char_map(charmap, 124, 12, 12, 12, 12, 12, 12, 12, 12, 12, 0, 0);
        create_char_map(charmap, 125, 7, 12, 12, 12, 56, 12, 12, 12, 7, 0, 0);
        create_char_map(charmap, 126, 38, 45, 25, 0, 0, 0, 0, 0, 0, 0, 0);

	this->charmap = charmap;
	return;
	//int j, i;
	//for(j=0;j<127;j++){
	//	for(i=0;i<11;i++){
	//		if(&charmap[j][i]==NULL) break;
	//		printf("map[%d][%d] %p %d\n", j, i, &charmap[j][i], charmap[j][i]);
	//	}
	//}
}

void
output_init(Vm *this){
	if(DEBUG) printf("vm_execute_call(): Handling Output.init\n");
	this->pc++; //just do nothing, we simply don't need to set anything up. All OS Math functions handled internally.

//	init_charmap(this);
//	int j, i;
//	short **charmap;
//	charmap = this->charmap;
//	for(j=0;j<127;j++){
//		for(i=0;i<11;i++){
//			if(&charmap[j][i]==NULL) break;
//			printf("map[%d][%d] %p %d\n", j, i, (void *) &charmap[j][i], charmap[j][i]);
//		}
//	}
}

// Array.vm
// can simply be mapped to Memory.alloc and Memory.deAlloc I just realized. Who would have thought!

// Memory.vm
void
memory_init(Vm *this){
	if(DEBUG) printf("vm_execute_call(): Handling Memory.init\n");
	this->pc++; //just do nothing, we simply don't need to set anything up. All OS Math functions handled internally.
}

void
memory_poke(Vm *this){
	int address, value;
	if(DEBUG) printf("vm_execute_call(): Handling Memory.poke\n");
	address = this->ram[this->ram[0]-2];
	value = this->ram[this->ram[0]-1];
	//printf("poke ram[%d] = %d\n", address, value);
	this->ram[address] = value;
	this->ram[this->ram[0]-2]= 0; //push 0 (void return value still needs to return a 0
	this->ram[0]--;
	this->pc++;
}

void
memory_peek(Vm *this){
	if(DEBUG) printf("vm_execute_call(): Handling Memory.peek\n");
	int address = this->ram[this->ram[0]-1];
	this->ram[this->ram[0]-1]= (short) this->ram[address];
	this->pc++;
}


void
memory_alloc(Vm *this){
	if(DEBUG) printf("vm_execute_call(): Handling Memory.alloc\n");
	int size = this->ram[this->ram[0]-1];

    	int32_t *ptr; //points at a free block, push it along through the free list if the free block is too small
	int32_t *ptr2;
	int fit;
    	int32_t *block;
	
    	//This implements greedy, first fit
	ptr = this->freelist;
    	while(ptr[1]<(size+2)){//still not big enough, keep looking
		if(ptr[0]==0){//NULL, end of the list
			//error, ran out of options
			//do Sys.error(7777);
			//do Sys.halt();
			printf("memory_alloc (built-in): cannot find enough memory.\n");
			exit(1);
		}
    		ptr = this->ram+ptr[0];//advance along the list
    	}
    	//end first fit concept

    	//==============================
	//following implements 'best fit' add on. D.Knuth thinks this is worse when the defrag is done below during deAlloc()
	fit = ptr[1]; //remember the size
	ptr2 = ptr; //save the present pointer as viable option
	while(ptr[0]>0){ //try to find a better less wasteful fit
		ptr = this->ram+ptr[0];//advance
		if((ptr[1]>(size+1))&(ptr[1]<fit)){//better fit (must be at least size+2 --> '>(size+1)'
			ptr2 = ptr;
			fit = ptr[1];
		}
	}
	//set ptr back to best fit and carry on as before below
	ptr = ptr2;
	//end best fit addon
	//===============================

    	//Now prepare a block and update freelist

    	//ptr is now po,inting to the large enough block
    	//take the required space at the end, then we only need to update size, not the previous pointer.
    	ptr[1]=ptr[1]-(size+2); //adjust the free list entry
    	block = ptr+ptr[1]+4; 
    	    	//Advance segment pointer by 2 to data segment. Then +ptr[1] (remaining bytes) then +2 to point to allocated data
    					//first to in the allocated data are [-1] size and [-2]=0 where the freelinked list pointer goes.
    	//Now set block[-1] and block[-2]
    	block[-2]=0; //this will contain a pointer to next free list entry (could be done without wasting this byte, but this is easier
    	block[-1]=size;

	this->ram[this->ram[0]-1]= (int) (block-this->ram); //push return value
	this->pc++;
	return;
}

void
memory_dealloc(Vm *this){
	int o;
	int32_t  *seg, *prev_seg, *next_seg;
	if(DEBUG) printf("vm_execute_call(): Handling Memory.deAlloc\n");
	o = this->ram[this->ram[0]-1];
	//let data[5]=o;
    	if(o==0){
    		//do Sys.error(8888);//NULL pointer dereference
    		//do Sys.halt();
		printf("memory_dealloc() (built-in): trying to free NULL pointer.\n");
    		exit(1);
    	}
	seg = this->ram+o-2; //use segment pointer from here (o-2 is the address of this segment)

	//find place to insert
	prev_seg = this->freelist;
	next_seg = this->ram+this->freelist[0];
	while(((next_seg-this->ram)>0)&(next_seg<seg)){//keep sorted by pointer addresses
		prev_seg = next_seg;
		next_seg = this->ram+prev_seg[0];
	}
	//insert
	prev_seg[0] = (int) (seg-this->ram);
	seg[0] = (int) (next_seg-this->ram);

	//Now check if we can defrag [prev_seg---seg---next_seg] segments:
	if ((seg + seg[1] + 2) == next_seg){//combine with next
 		seg[1] = seg[1] + next_seg[1] + 2;
		seg[0] = next_seg[0];
	}
	if ((prev_seg + prev_seg[1] + 2) == seg){ //combine with previous
		prev_seg[1] = prev_seg[1] + seg[1] + 2;
		prev_seg[0] = seg[0];
	}

	this->ram[this->ram[0]-1]= 0; //push 0 void return value
	this->pc++;
	return;
}

// Screen.vm
// white is false (0)
// black is true (-1)
void
screen_init(Vm *this){
	if(DEBUG) printf("vm_execute_call(): Handling Screen.init\n");
	this->pc++; //just do nothing, we simply don't need to set anything up. All OS Math functions handled internally.
}

void
screen_clearScreen(Vm *this){
 	int i;
	if(DEBUG) printf("vm_execute_call(): Handling Screen.clearScreen\n");
	for(i=16384;i<16384+8192;i++){
		this->ram[i]= 0;
	}
	this->currentcolor = -1; //black
	this->ram[this->ram[0]]= 0; //push 0 (void retrun value still needs to return a 0
	this->ram[0]++; //SP++
	this->pc++;
}

void
screen_darkScreen(Vm *this){
 	int i;
	if(DEBUG) printf("vm_execute_call(): Handling Screen.clearScreen\n");
	for(i=16384;i<16384+8192;i++){
		this->ram[i]= -1;
	}
	this->currentcolor = 0; //white
	this->ram[this->ram[0]]= 0; //push 0 (void retrun value still needs to return a 0
	this->ram[0]++; //SP++
	this->pc++;
}

void
screen_invertScreen(Vm *this){
 	int i;
	if(DEBUG) printf("vm_execute_call(): Handling Screen.invertScreen\n");
	for(i=16384;i<16384+8192;i++){
		this->ram[i]= ~this->ram[i];
	}
	this->currentcolor = ~this->currentcolor;
	this->ram[this->ram[0]]= 0; //push 0 (void retrun value still needs to return a 0
	this->ram[0]++; //SP++
	this->pc++;
}

void
screen_setColor(Vm *this){
	if(DEBUG) printf("vm_execute_call(): Handling Screen.setColor\n");
	this->currentcolor = (short) this->ram[this->ram[0]-1];
	this->ram[this->ram[0]-1]= 0; //push 0 (void retrun value still needs to return a 0
	this->pc++;
}

void
screen_drawPixel(Vm *this){
	if(DEBUG) printf("vm_execute_call(): Handling Screen.drawPixel\n");
	short x = (short) this->ram[this->ram[0]-2];
	short y = (short) this->ram[this->ram[0]-1];
	short tmp = x/16;
	short address = (32*y)+tmp;
	short kbit = x&15;//use mask. This is equivalent to modulo16 (bit position): x-(16*(x/16)) 
	if(this->currentcolor == -1){//black
		this->ram[16384+address] = (short) (this->ram[16384+address]|(1<<kbit));
	} else {
		this->ram[16384+address] = (short) (this->ram[16384+address]&(~(1<<kbit)));
	}
	this->ram[this->ram[0]-2]= 0; //push 0 (void retrun value still needs to return a 0
	this->ram[0]--;
	this->pc++;
}

void
screen_drawLine(Vm *this){
	short tmp, i, dx, dy, a, b, direction, diff;
	short tmp2, address, kbit;
	if(DEBUG) printf("vm_execute_call(): Handling Screen.drawLine\n");
	short x1 = (short) this->ram[this->ram[0]-4];
	short y1 = (short) this->ram[this->ram[0]-3];
	short x2 = (short) this->ram[this->ram[0]-2];
	short y2 = (short) this->ram[this->ram[0]-1];

	this->ram[this->ram[0]-4]= 0; //push 0 (void return value still needs to return a 0
	this->ram[0]--; //SP--
	this->ram[0]--;
	this->ram[0]--;
	this->pc++;
	//Always draw from left to right (increasing x), i.e. swap if x2<x1
	if(x2<x1){//swap
		tmp=x2;
		x2=x1;
		x1=tmp;
		tmp=y2;
		y2=y1;
		y1=tmp;
	}
	direction = 1;
	if(y2<y1){
		direction= -direction;
	}
	//handle simple cases: 
	//x1=x2
	if(x1==x2){ //vertical
		if(y1<y2){
			i=y1;
			while(i<(y2+1)){
				//do Screen.drawPixel(x1,i);
				tmp2 = x1/16;
				address = (32*i)+tmp2;
				kbit = x1&15;//use mask. This is equivalent to modulo16 (bit position): x-(16*(x/16)) 
				if(this->currentcolor == -1){//black
					this->ram[16384+address] = (short) (this->ram[16384+address]|(1<<kbit));
				} else {
					this->ram[16384+address] = (short) (this->ram[16384+address]&(~(1<<kbit)));
				}
				i=i+1;
			}
		} else {
			i=y2;
			while(i<(y1+1)){
				//do Screen.drawPixel(x1,i);
				tmp2 = x1/16;
				address = (32*i)+tmp2;
				kbit = x1&15;//use mask. This is equivalent to modulo16 (bit position): x-(16*(x/16)) 
				if(this->currentcolor == -1){//black
					this->ram[16384+address] = (short) (this->ram[16384+address]|(1<<kbit));
				} else {
					this->ram[16384+address] = (short) (this->ram[16384+address]&(~(1<<kbit)));
				}
				i=i+1;
			}
		}
		return; //done here
	}
	//y1 =y2
	if(y1==y2){//horizontal 
		if(x1<x2){
			i=x1;
			while(i<(x2+1)){
				//do Screen.drawPixel(i, y1);
				tmp2 = i/16;
				address = (32*y1)+tmp2;
				kbit = i&15;//use mask. This is equivalent to modulo16 (bit position): x-(16*(x/16)) 
				if(this->currentcolor == -1){//black
					this->ram[16384+address] = (short) (this->ram[16384+address]|(1<<kbit));
				} else {
					this->ram[16384+address] = (short) (this->ram[16384+address]&(~(1<<kbit)));
				}
				i=i+1;
			}
		} else{
			i=x2;
			while(i<(x1+1)){
				//do Screen.drawPixel(i, y1);
				tmp2 = i/16;
				address = (32*y1)+tmp2;
				kbit = i&15;//use mask. This is equivalent to modulo16 (bit position): x-(16*(x/16)) 
				if(this->currentcolor == -1){//black
					this->ram[16384+address] = (short) (this->ram[16384+address]|(1<<kbit));
				} else {
					this->ram[16384+address] = (short) (this->ram[16384+address]&(~(1<<kbit)));
				}
				i=i+1;
			}
		}
		return; //done here
	}
	if(y2>y1){ //easy case
		dy = y2 -y1;
		dx = x2 -x1; //always the case, we checked and swapped
		diff=0;
		a=0;
		b=0;
		while((a<(dx+1))&(b<(dy+1))){
			//do Screen.drawPixel(x1+a, y1+b);
			tmp2 = (x1+a)/16;
			address = (32*(y1+b))+tmp2;
			kbit = (x1+a)&15;//use mask. This is equivalent to modulo16 (bit position): x-(16*(x/16)) 
			if(this->currentcolor == -1){//black
				this->ram[16384+address] = (short) (this->ram[16384+address]|(1<<kbit));
			} else {
				this->ram[16384+address] = (short) (this->ram[16384+address]&(~(1<<kbit)));
			}

			if(diff<0){
				a=a+1;
				diff=diff+dy;
			}else{
				b=b+1;
				diff=diff-dx;
			}
		}
		return;
	}
	if(y1>y2){ //other case
		dy = y2 -y1;
		dx = x2 -x1; //always the case, we checked and swapped
		diff=0;
		a=0;
		b=0;
		while((a<(dx+1))&(b>(dy-1))){
			//do Screen.drawPixel(x1+a, y1+b);
			tmp2 = (x1+a)/16;
			address = (32*(y1+b))+tmp2;
			kbit = (x1+a)&15;//use mask. This is equivalent to modulo16 (bit position): x-(16*(x/16)) 
			if(this->currentcolor == -1){//black
				this->ram[16384+address] = (short) (this->ram[16384+address]|(1<<kbit));
			} else {
				this->ram[16384+address] = (short) (this->ram[16384+address]&(~(1<<kbit)));
			}
			if(diff<0){
				a=a+1;
				diff=diff-dy;
			}else{
				b=b-1;
				diff=diff-dx;
			}
		}
		return;
	}
	return;
}

void
screen_drawRectangle(Vm *this){
	if(DEBUG) printf("vm_execute_call(): Handling Screen.drawRectangle\n");
	short tmp2, address, kbit;
	short x1 = (short) this->ram[this->ram[0]-4];
	short y1 = (short) this->ram[this->ram[0]-3];
	short x2 = (short) this->ram[this->ram[0]-2];
	short y2 = (short) this->ram[this->ram[0]-1];

	this->ram[this->ram[0]-4]= 0; //push 0 (void return value still needs to return a 0
	this->ram[0]--;
	this->ram[0]--;
	this->ram[0]--; //SP++
	this->pc++;

	int i=y1;
	if(x1>x2){
		short tmp=x1;
		x1=x2;
		x2=tmp;
	}
    	while(i<(y2+1)){
    		//do Screen.drawLine(x1,i,x2,i);
		int j=x1;
		while(j<(x2+1)){
			//do Screen.drawPixel(j, i);
			tmp2 = j/16;
			address = (32*i)+tmp2;
			kbit = j&15;//use mask. This is equivalent to modulo16 (bit position): x-(16*(x/16)) 
			if(this->currentcolor == -1){//black
				this->ram[16384+address] = (short) (this->ram[16384+address]|(1<<kbit));
			} else {
				this->ram[16384+address] = (short) (this->ram[16384+address]&(~(1<<kbit)));
			}
			j=j+1;
		}
		i=i+1;
	}
	return;
}

void
screen_drawCircle(Vm *this){
	if(DEBUG) printf("vm_execute_call(): Handling Screen.drawCircle\n");
	short x = (short) this->ram[this->ram[0]-3];
	short y = (short) this->ram[this->ram[0]-2];
	short r = (short) this->ram[this->ram[0]-1];

	this->ram[this->ram[0]-3]= 0; //push 0 (void return value still needs to return a 0
	this->ram[0]--;
	this->ram[0]--; //SP++
	this->pc++;

	short rsq, dy, dx, tmp2, i, kbit, address;
	if(abs(r)>181){//will cause overflow
		return;
	}
	rsq = r*r;
	dy = -r;
	while(dy<r){
		dx=sqrt(rsq-(dy*dy));
		//do Screen.drawLine(x-dx,y+dy,x+dx,y+dy);
		i=x-dx;
		while(i<(x+dx+1)){
			//do Screen.drawPixel(i, y1);
			tmp2 = i/16;
			address = (32*(y+dy))+tmp2;
			kbit = i&15;//use mask. This is equivalent to modulo16 (bit position): x-(16*(x/16)) 
			if(this->currentcolor == -1){//black
				this->ram[16384+address] = (short) (this->ram[16384+address]|(1<<kbit));
			} else {
				this->ram[16384+address] = (short) (this->ram[16384+address]&(~(1<<kbit)));
			}
			i=i+1;
		}
		dy=dy+1;
	}
	return;
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
	if(strcmp(this->label[this->pc], "Math.multiply") == 0){
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
	} else if(strcmp(this->label[this->pc], "Math.init") == 0){
		math_init(this);
		handled++;
	} else


	// Sys.vm
	if(strcmp(this->label[this->pc], "Sys.halt") == 0){
		sys_halt(this);
		handled++;
	}


	// Screen.vm
	else if(strcmp(this->label[this->pc], "Screen.init") == 0){
		screen_init(this);
		handled++;
	}
	else if(strcmp(this->label[this->pc], "Screen.clearScreen") == 0){
		screen_clearScreen(this);
		handled++;
	}
	else if(strcmp(this->label[this->pc], "Screen.darkScreen") == 0){
		screen_darkScreen(this);
		handled++;
	}
	else if(strcmp(this->label[this->pc], "Screen.invertScreen") == 0){
		screen_invertScreen(this);
		handled++;
	}
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

	// Memory.vm
	else if(strcmp(this->label[this->pc], "Memory.init") == 0){
		memory_init(this);
		handled++;
	}
	else if(strcmp(this->label[this->pc], "Memory.alloc") == 0){
		memory_alloc(this);
		handled++;
	}
	else if(strcmp(this->label[this->pc], "Memory.deAlloc") == 0){
		memory_dealloc(this);
		handled++;
	}
	else if(strcmp(this->label[this->pc], "Memory.peek") == 0){
		memory_peek(this);
		handled++;
	}
	else if(strcmp(this->label[this->pc], "Memory.poke") == 0){
		memory_poke(this);
		handled++;
	}

	// Array.vm (note that this is mapped to Memory.alloc and Memory.deAlloc)
	else if(strcmp(this->label[this->pc], "Array.new") == 0){
		memory_alloc(this);
		handled++;
	} else if(strcmp(this->label[this->pc], "Array.dispose") == 0){
		memory_dealloc(this);
		handled++;
	}

	return handled;
}
