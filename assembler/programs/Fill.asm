// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed. 
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.
(LOOP)
// Set count to number of display rows
@8192
D=A
@count
M=D

// Get keyboard input and perform respective action
@KBD
D=M
@ON
D;JNE

// Clear Screen
(OFF)
@SCREEN
D=A
@count
A=D+M
M=0

@count
M=M-1
D=M
@OFF
D;JGE
@LOOP
0;JMP

// Fill screen
(ON)
@SCREEN
D=A
@count
A=D+M
M=-1

@count
M=M-1
D=M
@ON
D;JGE
@LOOP
0;JMP
