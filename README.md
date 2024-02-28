# custom-cpu-emulator
This is a custom cpu emulator, written in C/C++, and following a risc style instruction set.

## Errors
At this point, error handling is shoddy at best.

## Controls
There are a few simple controls that you can use while the emulator is running.

- 'q': quit
- 'c': pause/unpause
- 'r': restart program execution

## Assembly Code Documentation
### Sections
At the moment, there are only two section types, **.code**, and **.data**. The data section must precede the code section.

#### Data
Storing global data is done by creating a label for the data, declaring the size of the data, and then providing a value, ([label]: d[b/w] [value]). Using declared data is done by moving the data label into a register, and loading the value stored at that address into a register. If you use a load instruction that is too small, you'll miss some of the stored data. While you can technically use jump instructions to move to the stored data, I would advise against it. You would no longer be reading assembly instructions, and instead would be using whatever you have stored in .data to run in the program.

#### Code
Code consists of a _start label, and a series of assembly instructions. 

##### Labels
A label is just a string that is converted into an immediate value for all of the instructions. This is useful for jumping, as well as for the data section, where declared data is labeled.

A label is declared by simply using a string that doesn't overlap with any of the assembly instructions, followed by a colon.

##### Instructions
There are a number of instructions that can be used.

###### Suffixes
Suffixes are added to the end of instructions, and modify the effects, or even the way that they are "called". Suffixes must be applied in the order that they are listed here.

- 'i': the instruction uses an immediate value instead of a source register as the second operand.
- 's': sets the flags register based on the value stored in the register, (zero sets the zero flag, negative value sets the negative flag, etc...).
- 'u': performs an unsigned operation, or it doesn't preserve the sign bit in the case of shifts.
- 'o': stores overflow.

###### and/or/xor(is)
Each of these instructions is used in the following format: [instr] [dest] [src]. The instruction operates like it is written, so (and ra rb) will be rendered conceptually as ra = ra & rb. If suffixed by 'i' it would be ra = ra & imm.

###### not
not is a pseudo-instruction that wraps xor.

###### add/sub/mul/div/mod(isuo)
Each of these instructions is used in the following format: [instr] [dest] [src]. The instruction operates like it is written, so (sub ra rb) will be rendered conceptually as ra = ra - rb.

###### lsr/lsl/asr/asl(isuo)
None of these instructions have been implemented. I was working on the requirements for a cpu that can print an entire string of character data, and just didn't end up implementing these instructions. The 'u' suffix instead affects whether the shift instruction is logical, or arithmetic.

###### jmp/jal/jeq/jne/jlt/jle/jgt/jge
- jmp: unconditional jump
- jal: unconditional jump and link
- jeq: jump if zero flag is set
- jne: jump if zero flag is not set
- jlt: jump if negative flag is set and zero flag is not set
- jle: jump if negative flag is set and zero flag is set
- jgt: jump if negative flag is not set and zero flag is not set
- jge: jump if negative flag is not set and zero flag is set

###### stb/stw/ldb/ldw
- stb: stores the lower byte in the source register in the address pointed to in the address register.
- stw: stores word in the source register in the address pointed to in the address register.
- ldb: loads the lower byte into the destination register from the address pointed to in the address register.
- ldw: loads word into the destination register from the address pointed to in the address register.

###### mov(i)
- mov: move either the contents of the source register or immediate value into the destination register.

###### in/out
These instructions are a little weird and clunky.

- in: stores a byte from the terminal in the destination register.
- out: loads a byte from the source register into the terminal.

###### hlt
Ends execution.
