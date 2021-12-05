# Hack VM Translator
This program translates intermediate Hack bytecode (aka 'Virtual Machine' code) into Hack assembly.

You can pass it either a single .vm file or a folder containing multiple .vm files. In either case it will generate a single .asm file.

## Example Input (Hello World):
```
function Main.main 0
push constant 12
call String.new 1
push constant 104
call String.appendChar 2
push constant 101
call String.appendChar 2
push constant 108
call String.appendChar 2
push constant 108
call String.appendChar 2
push constant 111
call String.appendChar 2
push constant 44
call String.appendChar 2
push constant 32
call String.appendChar 2
push constant 119
call String.appendChar 2
push constant 111
call String.appendChar 2
push constant 114
call String.appendChar 2
push constant 108
call String.appendChar 2
push constant 100
call String.appendChar 2
call Output.printString 1
pop temp 0
push constant 0
return
```

## Example Output:
```
// Initialize stack
@256
D=A
@SP
M=D

// Call Sys.init
@Sys.init$ret.0
D=A
@SP
M=M+1
A=M-1
M=D
@SAVE_RET_0
D=A
@R13
M=D
@SAVE
0;JMP
(SAVE_RET_0)
@SP
D=M
@5
D=D-A
@0
D=D-A
@ARG
M=D
@Sys.init
0;JMP
(Sys.init$ret.0)

// Ran everytime a function is called
(SAVE)
@LCL
D=M
@SP
M=M+1
A=M-1
M=D
@ARG
D=M
@SP
M=M+1
A=M-1
M=D
@THIS
D=M
@SP
M=M+1
A=M-1
M=D
@THAT
D=M
@SP
M=M+1
A=M-1
M=D
@SP
D=M
@LCL
M=D
@R13
A=M
0;JMP

// Ran everytime a function is returned
(RESTORE)
@LCL
D=M
@R13
M=D
@5
A=D-A
D=M
@R14
M=D
@SP
AM=M-1
D=M
@ARG
A=M
M=D
@ARG
D=M+1
@SP
M=D
@R13
D=M
@1
A=D-A
D=M
@THAT
M=D
@R13
D=M
@2
A=D-A
D=M
@THIS
M=D
@R13
D=M
@3
A=D-A
D=M
@ARG
M=D
@R13
D=M
@4
A=D-A
D=M
@LCL
M=D
@R14
A=M
0;JMP

// function Main.main 0
(Main.main)
@0
D=A
(Main.main$Lcl)
D=D-1
@Main.main$LclEnd
D;JLT
@SP
M=M+1
A=M-1
M=0
@Main.main$Lcl
0;JMP
(Main.main$LclEnd)

// push constant 12
@12
D=A
@SP
M=M+1
A=M-1
M=D

// call String.new 1
@String.new$ret.1
D=A
@SP
M=M+1
A=M-1
M=D
@SAVE_RET_1
D=A
@R13
M=D
@SAVE
0;JMP
(SAVE_RET_1)
@SP
D=M
@5
D=D-A
@1
D=D-A
@ARG
M=D
@String.new
0;JMP
(String.new$ret.1)

// push constant 104
@104
D=A
@SP
M=M+1
A=M-1
M=D

// call String.appendChar 2
@String.appendChar$ret.2
D=A
@SP
M=M+1
A=M-1
M=D
@SAVE_RET_2
D=A
@R13
M=D
@SAVE
0;JMP
(SAVE_RET_2)
@SP
D=M
@5
D=D-A
@2
D=D-A
@ARG
M=D
@String.appendChar
0;JMP
(String.appendChar$ret.2)

// push constant 101
@101
D=A
@SP
M=M+1
A=M-1
M=D

// call String.appendChar 2
@String.appendChar$ret.3
D=A
@SP
M=M+1
A=M-1
M=D
@SAVE_RET_3
D=A
@R13
M=D
@SAVE
0;JMP
(SAVE_RET_3)
@SP
D=M
@5
D=D-A
@2
D=D-A
@ARG
M=D
@String.appendChar
0;JMP
(String.appendChar$ret.3)

// push constant 108
@108
D=A
@SP
M=M+1
A=M-1
M=D

// call String.appendChar 2
@String.appendChar$ret.4
D=A
@SP
M=M+1
A=M-1
M=D
@SAVE_RET_4
D=A
@R13
M=D
@SAVE
0;JMP
(SAVE_RET_4)
@SP
D=M
@5
D=D-A
@2
D=D-A
@ARG
M=D
@String.appendChar
0;JMP
(String.appendChar$ret.4)

// push constant 108
@108
D=A
@SP
M=M+1
A=M-1
M=D

// call String.appendChar 2
@String.appendChar$ret.5
D=A
@SP
M=M+1
A=M-1
M=D
@SAVE_RET_5
D=A
@R13
M=D
@SAVE
0;JMP
(SAVE_RET_5)
@SP
D=M
@5
D=D-A
@2
D=D-A
@ARG
M=D
@String.appendChar
0;JMP
(String.appendChar$ret.5)

// push constant 111
@111
D=A
@SP
M=M+1
A=M-1
M=D

// call String.appendChar 2
@String.appendChar$ret.6
D=A
@SP
M=M+1
A=M-1
M=D
@SAVE_RET_6
D=A
@R13
M=D
@SAVE
0;JMP
(SAVE_RET_6)
@SP
D=M
@5
D=D-A
@2
D=D-A
@ARG
M=D
@String.appendChar
0;JMP
(String.appendChar$ret.6)

// push constant 44
@44
D=A
@SP
M=M+1
A=M-1
M=D

// call String.appendChar 2
@String.appendChar$ret.7
D=A
@SP
M=M+1
A=M-1
M=D
@SAVE_RET_7
D=A
@R13
M=D
@SAVE
0;JMP
(SAVE_RET_7)
@SP
D=M
@5
D=D-A
@2
D=D-A
@ARG
M=D
@String.appendChar
0;JMP
(String.appendChar$ret.7)

// push constant 32
@32
D=A
@SP
M=M+1
A=M-1
M=D

// call String.appendChar 2
@String.appendChar$ret.8
D=A
@SP
M=M+1
A=M-1
M=D
@SAVE_RET_8
D=A
@R13
M=D
@SAVE
0;JMP
(SAVE_RET_8)
@SP
D=M
@5
D=D-A
@2
D=D-A
@ARG
M=D
@String.appendChar
0;JMP
(String.appendChar$ret.8)

// push constant 119
@119
D=A
@SP
M=M+1
A=M-1
M=D

// call String.appendChar 2
@String.appendChar$ret.9
D=A
@SP
M=M+1
A=M-1
M=D
@SAVE_RET_9
D=A
@R13
M=D
@SAVE
0;JMP
(SAVE_RET_9)
@SP
D=M
@5
D=D-A
@2
D=D-A
@ARG
M=D
@String.appendChar
0;JMP
(String.appendChar$ret.9)

// push constant 111
@111
D=A
@SP
M=M+1
A=M-1
M=D

// call String.appendChar 2
@String.appendChar$ret.10
D=A
@SP
M=M+1
A=M-1
M=D
@SAVE_RET_10
D=A
@R13
M=D
@SAVE
0;JMP
(SAVE_RET_10)
@SP
D=M
@5
D=D-A
@2
D=D-A
@ARG
M=D
@String.appendChar
0;JMP
(String.appendChar$ret.10)

// push constant 114
@114
D=A
@SP
M=M+1
A=M-1
M=D

// call String.appendChar 2
@String.appendChar$ret.11
D=A
@SP
M=M+1
A=M-1
M=D
@SAVE_RET_11
D=A
@R13
M=D
@SAVE
0;JMP
(SAVE_RET_11)
@SP
D=M
@5
D=D-A
@2
D=D-A
@ARG
M=D
@String.appendChar
0;JMP
(String.appendChar$ret.11)

// push constant 108
@108
D=A
@SP
M=M+1
A=M-1
M=D

// call String.appendChar 2
@String.appendChar$ret.12
D=A
@SP
M=M+1
A=M-1
M=D
@SAVE_RET_12
D=A
@R13
M=D
@SAVE
0;JMP
(SAVE_RET_12)
@SP
D=M
@5
D=D-A
@2
D=D-A
@ARG
M=D
@String.appendChar
0;JMP
(String.appendChar$ret.12)

// push constant 100
@100
D=A
@SP
M=M+1
A=M-1
M=D

// call String.appendChar 2
@String.appendChar$ret.13
D=A
@SP
M=M+1
A=M-1
M=D
@SAVE_RET_13
D=A
@R13
M=D
@SAVE
0;JMP
(SAVE_RET_13)
@SP
D=M
@5
D=D-A
@2
D=D-A
@ARG
M=D
@String.appendChar
0;JMP
(String.appendChar$ret.13)

// call Output.printString 1
@Output.printString$ret.14
D=A
@SP
M=M+1
A=M-1
M=D
@SAVE_RET_14
D=A
@R13
M=D
@SAVE
0;JMP
(SAVE_RET_14)
@SP
D=M
@5
D=D-A
@1
D=D-A
@ARG
M=D
@Output.printString
0;JMP
(Output.printString$ret.14)

// pop temp 0
@SP
AM=M-1
D=M
@5
M=D

// push constant 0
@0
D=A
@SP
M=M+1
A=M-1
M=D

// return
@RESTORE
0;JMP

(END_PROGRAM)
@END_PROGRAM
0;JMP
```

## Requirements
A standard C compiler.

## Build
Run `make`

## Run
### Linux
`./hackvm <path-to-file/folder>`
