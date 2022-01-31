# Hack VM Emulator

This is a VM emulator for the Hack platform implemented in C and SDL.
It allows to run Jack programs compiled to .vm files directly on a slim VM emulator.

Implementation details:
For faster direct implementation of OS functions, a preprocessor flag can be set.
Presently the Math.jack OS functions are implemented (massive speed-up for Math.multiply and Math.divide)
Note that this VM implementation presently needs all the OS .vm files to be present
(the statics memory segment needs to be set to the correct size).
<pre>
	#define OVERRIDE_OS_FUNCTIONS 1
</pre>

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
