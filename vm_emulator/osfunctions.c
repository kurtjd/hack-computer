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
	this->ram[this->ram[0]]= 0; //push 0 (void retrun value still needs to return a 0
	this->ram[0]++; //SP++
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
	this->ram[0]--;
	this->ram[0]--;
	this->ram[0]--; //SP++
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

	return handled;
}
