# Hack VM Emulator

This is a VM emulator for the Hack platform implemented in C and SDL.
It allows to run Jack programs compiled to .vm files directly on a slim VM emulator.

Implementation details:
For faster direct implementation of OS functions, a preprocessor flag can be set.
Presently the Math.jack OS functions are implemented (massive speed-up for Math.multiply and Math.divide)
Note that this VM implementation presently needs the OS .vm files to be present as long as not all
functions are implemented within the VM emulator yet.
(the statics memory segment needs to be set to the correct size).
Update: Math.vm is not needed anymore.

<pre>
	#define OVERRIDE_OS_FUNCTIONS 1
</pre>

Known problems:
~~There must be a subtle difference to the Java VM Emulator still. Mark Armbrust's Float.jack stuff does not
finish the self-test. Please take a look and find the bug!~~
<pre>
	http://nand2tetris-questions-and-answers-forum.52.s1.nabble.com/file/n4025143/Float_1_5.zip
	http://nand2tetris-questions-and-answers-forum.52.s1.nabble.com/Floating-point-arithmetic-td4025143.html
</pre>
This has been fixed. The problem was the if(x) check in the `if-goto` execution.
It should operate as if(~(x=0)) in Jack language.

## Requirements
* C compiler
* SDL2

## Build
Simply run `make`

## Run
### Linux
`./vmemu <path-to-files>`

### Windows
(untested)
`vmemu.exe <path-to-files>`
