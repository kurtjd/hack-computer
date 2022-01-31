CFLAGS = -g3 -save-temps

all: vmemu

vmemu.o: vmemu.c vmemulib.h
	gcc $(CFLAGS) -c vmemu.c

vmemulib.o: vmemulib.c vmemulib.h
	gcc $(CFLAGS) -c vmemulib.c
	
vmemu: vmemu.o vmemulib.o
	gcc  $(CFLAGS) vmemu.o vmemulib.o -Wall -Wextra -Wpedantic -lSDL2 -o vmemu

clean:
	rm -f core vmemu vgcore.* vmemu.o vmemulib.o vmemu.i vmemu.s vmemulib.s vmemulib.i
