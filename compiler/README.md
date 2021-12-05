# Jack Compiler
This is a compiler for the simple Jack programming language which outputs intermediate Hack virtual machine code. This VM code can then be used as input for the VM translator to produce Hack assembly code.

It does not currently attempt to make any optimizations and the error reporting is pretty bad, but hey, it works!

### Example Input (Hello World):
```
class Main
{
   function void main()
   {
       do Output.printString("hello, world");
       return;
   }
}
```

### Example Output:
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

# Requirements
A C compiler

# Build
Simply run `make`

# Run
`./hackjack <path-to-file/folder>`

# References
[Jack Language Specification](https://www.cs.huji.ac.il/course/2002/nand2tet/docs/ch_9_jack.pdf)
