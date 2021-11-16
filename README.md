# Hack Computer
## What?
Hack is a 16-bit computer designed for the [nand2tetris](https://www.nand2tetris.org/) course that is meant to be simple yet Turing complete.

In this course, you are challenged to implement the design using only two primitives: the NAND gate and D flip-flop. By combining these two primitives in different ways, you can achieve all the functionality of a modern computer.

## Why?
I simply wanted to learn more about how computers work at such a low-level. Thus, this repo contains my implementation of the different chips in what is known as a Hardware Description Language (HDL). It will also contain my implementations of an assembler, compiler, operating system, emulator, and some sample programs such as games!

The goal of this project was to learn and not necessarily to design the most efficient and optimized chips, so I'm sure my implementations can use some adjustment. However, my next goal will be to build a computer of my own design in which I will take care to implement efficient designs.

## How?
The chip logic resides in .hdl files and can be loaded into a hardware simulator provided with the course found [here](https://www.nand2tetris.org/software).

From there, one can load binary .hack files into the instruction ROM and begin executing them.

## Disclaimer
This repo simply exists to track and showcase my work on this project. If you are also taking the course, I highly recommend not looking at my implementations if you want the fun of solving things yourself. I also do not claim to have the best designed chips so this repo should not be used as a guide for best practices in hardware design.

## Assembler
See [Assembler README](assembler/README.md)