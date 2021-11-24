# Hack VM Translator
This program translates intermediate Hack bytecode (aka 'Virtual Machine' code) into Hack assembly.

You can pass it either a single .vm file or a folder containing multiple .vm files. In either case it will generate a single .asm file.

Windows is not currently supported due to directory/path handling code. Will get that working in the future.

## Requirements
A standard C compiler.

## Build
Run `make`

## Run
### Linux
`./hackvm <path-to-file/folder>`
