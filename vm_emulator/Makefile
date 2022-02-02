#CFLAGS = -g3 -Wall -Wpedantic -save-temps
CFLAGS = -O2

all: vmemu

osfunctions.o: osfunctions.c vmemulib.h
	gcc $(CFLAGS) -c osfunctions.c

vmemu.o: vmemu.c vmemulib.h
	gcc $(CFLAGS) -c vmemu.c

vmemulib.o: vmemulib.c vmemulib.h
	gcc $(CFLAGS) -c vmemulib.c
	
vmemu: vmemu.o vmemulib.o osfunctions.o
	gcc $(CFLAGS) vmemu.o vmemulib.o osfunctions.o -Wall -Wextra -Wpedantic -lSDL2 -lm -o vmemu

clean:
	rm -f core vmemu vgcore.* vmemu.o vmemulib.o vmemu.i vmemu.s vmemulib.s vmemulib.i osfunctions.o osfunctions.i osfunctions.s
