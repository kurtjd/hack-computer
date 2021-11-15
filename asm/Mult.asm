// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)
//
// This program only handles arguments that satisfy
// R0 >= 0, R1 >= 0, and R0*R1 < 32768.
@R2
M=0

@R1
D=M

@count
M=D

(LOOP)
    @count
    D=M
    M=M-1

    @END
    D;JLE

    @R0
    D=M

    @R2
    M=D+M

    @LOOP
    0;JMP
(END)
@END
0;JMP
