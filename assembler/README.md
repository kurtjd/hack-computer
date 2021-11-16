# Hack Assembler/Disassembler
This is a C implementation of an assembler/disassembler for the Hack machine language capable of handling symbols.

Assembled files appear in out.hack and disassembled files appear in out.asm.

## Requirements
A basic C compiler

## Build
Simply run `make`

## Run
### Linux
`./hackasm [-d] <path-to-file>`

### Windows
`hackasm.exe [-d] <path-to-file>`

## Options
Supply the `-d` flag if you wish to disassemble a .hack file rather than assemble a .asm file.

## References
[Hack Machine Language](https://b1391bd6-da3d-477d-8c01-38cdf774495a.filesusr.com/ugd/44046b_d70026d8c1424487a451eaba3e372132.pdf)