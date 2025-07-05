
# UNIBL
(Universal Bootstrappable Language)

Writing programs in certain languages comes with portability issues, to run a C program you need a C compiler that targets your specific architecture. Some languages like Java try to solve this by implementing a virtual machine that custom bytecode can compile to. While well defined these virtual machines are still complex, the JVM spec is currently sitting at around 160 pages which is far from something trivial to implement in any system. Other esoteric programming languages like BrainF* take this in the opposite direction- trivial to implement but barely useful. My goal with this project is to write a programming language that is complex enough to define itself and implement it's own behaviour using itself, I.E "bootstrapping", but also that is simple enough it could reasonably be implemented on any system as an afternoon project rather than a month long undertaking.

## The UNIBL Virtual Machine

UNIBL operates on a Von Neumann architecture, the virtual machine has a 64bit memory address space comprised of 8bit cells and 2 64bit registers, `ACC_A` and `ACC_B` for accumulator A and accumulator B. There also exists a 64bit program counter `PC` which stores the address of the current instruction being executed.

In sum the environment required to run UNIBL bytecode is such:
```
uint8_t MEM[1<<64];   // 64bit address space
uint64_t ACC_A;       // 64bit accumulator A
uint64_t ACC_B;       // 64bit accumulator B
uint64_t PC;          // 64bit program counter
```

### Address Partitioning

Because all memory is stored in a single tape, it's important to specify which addresses of memory are designated for which tasks.
```
0x0000 : 0x03FF => Standard Input
0x0400 : 0x07FF => Standard Output
0x0800 Onwards  => Program Memory
```
Why separate memory into these chunks instead of storing input, output, etc. in separate variables? Having Standard Input and Output along with other things be assigned to specific places in memory both reduces instruction complexity (less opcodes to implement in a virtual machine) and allows greater freedom of what parts of UNIBL you deem necessary in your implementation. Perhaps you aren't running any programs that need input; instead of having to still implement input features to run your code you can choose to simply not implement it with all code still running as you would expect. This modular system allows you to decide what specifications you deem necessary to add. This also allows you flexibility in extending the functionality of the virtual machine, making it easy to implement new features down the line.

### Reading From MEM

Memory is read 1 `u8` (8 bit chunk) at a time. Each UNIBL instruction is stored as a `u8` and their arguments are stored in the appropriate size. Some arguments are only `u8`, (for example determining what section of an accumulator to store 8bits, there are only 8 options) whereas some are `u64` (determining the address of the memory to load).

Note: for every `u8` read it is implied that the program counter is incremented

Since program memory starts at `0x0800` this means your program counter will start initialized to `0x0800`. To execute an instruction you must first read what the instruction is. Reading this `u8` will determine whether you need to read additional memory to construct the full instruction, or if it can be executed immediately. For a no argument instruction like `SWP`, which swaps the values of `ACC_A` and `ACC_B`, you can immediately execute this instruction then return to reading the next op code. For a more complicated instruction like `LDA` which takes a `u8` for the offset of `ACC_A` and a `u64` for the address in `MEM`, you must first read a `u8` for the offset and read a `u64` for the address.

### OP Codes

A full definition of UNIBL opcode numbers can be found in `inc/unibl.h`. Currently there exist 11 opcodes not including `HALT`.

**0 = HALT**

End program execution

*If you are encountering a halt it is likely because you are at the end of a program anyway, since all memory addresses default to 0. Implementing this is useful and highly recommended but not necessary.*

**1 = LDA [u8: offset] [u64: address]**

Load address into A

*Having a function that is able to load an address from memory into an accumulator is absolutely necessary. For information on what* `offset` *means see* **What are offsets?**

**2 = STA [u64: address] [u8: offset]**

Store A into address

*Having a function that is able to store into memory is also absolutely necessary.*

**3 = SWP**

Swap values of A and B

*This is what gives power to the B accumulator, SWP allows any function that works on A to share the same behaviour on B. For example:* `LDB` *could be defined as* `SWP -> LDA -> SWP`

**4 = JMPA**

Jump to address of A

*Being able to jump to any position in program during execution is extremely useful and necessary for turing completeness.*

**5 = JMPBZ [u64: address]**

Jump to address if B is zero

*Jumping is useful but having a conditional jump is also required for turing completeness. You may have noticed that* `JMPBZ` *could potentially eliminate the need for* `JMPA` *by using STA to override the value at the address of the argument then setting B to zero, however this is more tedious and memory inefficient than simply implementing this function into the virtual machine. Especially because JMPA allows for the value of B to transfer without having to store it in memory.*

**6 = ADDAB**

Add the values of A and B and store the result in B

*Some form of addition is absolutely necessary for computation.*

**7 = SUBAB**

Subtract B from A and store the result in A

*Could subtraction be implemented using the already defined instructions? Yes, but very VERY laboriously; to the point where the compute time would make it not worth it.*

**8 = LDAB [u8: offset]**

Same as `LDA` but using B as the address

*Could theoretically be implemented as a reference to* `LDA` *that uses* `SWP` *and* `STA` *to store B in the parameter address of* `LDA` *however this is extremely memory inefficient for such a fundemental dereference operation.*

**9 = STAB [u8: offset]**

Same as `STA` but using B as the address

*The same argument could be made here as* `LDAB` *for its redundancy but everything is a tradeoff and if we were to boil everything down only to completely fundemental operators we would end up with a bytecode that is so memory inefficient and computationally intensive it is virtually useless.*

**10 = CMPAB**

Set B to 0 if A and B are equal, otherwise set B to 1

*Comparison is required for conditional jumps, absolutely necessary.*

**11 = VOID [u64: data]**

Do nothing

*Why is it necessary to have an instruction that takes an argument and literally does nothing? Unexpectedly the* `VOID` *command is one of the most useful commands. Voiding a* `u64` *means that argument will be stored in memory so it is pure data that can be referenced later. For example, if a program needs a constant that constant can be passed into* `VOID` *and will appear in program memory at the address of* `VOID + 1`*, this constant can then be referenced and stored as desired using other instructions. Trying to store memory in program space can only be done as an argument to an instruction and trying to use any instruction that does something with its arguments will cause unexpected behaviour.*

### What are offsets?

In a function such as `LDA [u8: offset] [u64: address]` the offset refers to the section of A that the `u8` from memory is to be stored in. Since A is a `u64` BUT memory is a tape of `u8`'s there would be no purpose in having a `u64` register if the maximum value that could be loaded was `255`. Moreover if `255` was the maximum value loadable then only up to `0xFF` of memory would be addressable and the entire system would fall apart. To avoid this `u8`'s are stored in 8 bit sections of `u64` registers. For example:
```
OFFSET = 0
ACC_A :      0x0000000000000000;
LDA TARGETS: 0x00000000000000FF;
================================
OFFSET = 3
ACC_A :      0x0000000000000000;
LDA TARGETS: 0x00000000FF000000;
================================
OFFSET = 7
ACC_A :      0x0000000000000000;
LDA TARGETS: 0xFF00000000000000;
```
Specifically in my VM implementation `LDA` is explicitly defined as
```C
uint8_t offset = 8 * read_u8();           // LOAD AND CONVERT BYTE OFFSET TO BIT OFFSET
uint64_t addr = read_u64();
if (offset >= 64) continue;               // BIT OFFSET CANNOT BE MORE THAN 63
                                          // ( THIS WOULD IMPLY STORING INTO A NON EXISTANT SECTOR OF A )
ACC_A &= ~((uint64_t)0xFF << offset);     // BITSHIFT MASK BY OFFSET AND RESET SECTOR OF A
ACC_A |= (uint64_t)MEM[addr] << offset;   // ADD MEMORY AT ADDRESS TO SECTOR AT OFFSET OF A
```
For more precise definitions of instruction sets see `vm/unibl_vm.c`

## The UNIBL Assembler

Writing anything in bytecode is incredibly tedious. To save time and frustration, and to make this setup reasonable to work with I have written an assembler that transforms `.uasm` files into program binaries which can be executed on the UNIBL VM.

### Instruction Syntax

USAM Instructions are structured as follows:
`INST operands NEWLINE` where `operands` are separated by commas. For example:
```nasm
; PROGRAM TO LOAD 0x33 INTO B

; STORE 0x33 AT PROGRAM START (0x0800)
VOID 0x33

; LOAD ADDRESS AT 0x0808 INTO A
; VOID emits a u64 in big endian so
; VOID 0x33 is shorthand for
; VOID 0x0000000000000033
; Since the VOID opcode is 1 byte
; and our desired constant is at
; an offset of 7 we get 0x0800 + 1 + 7
; = 0x0808
LDA 0, 0x0808

; SWAP A AND B TO STORE 0x33 INTO B
SWP

; You could add a HALT instruction
; however the program ending implicitly
; means the next instruction will be
; 0 by default so it would be equivalent
```
This way of programming is still very tedious though, there are many important features of the assembler that could could make this program more readable.

### Labels

Labels allow for more dynamic programming, instead of hard-coding addresses in memory labels and simple math allow you to reference memory relative to where the program exists. Here is the previous program rewritten with labels:
```nasm
start:
VOID 0x33
LDA 0, start + 8
SWP
```
Label definitions are shorthand for "this identifier now means the address that this line exists at in program space." Alternatively defining a label sets that label to mean the value of the program counter at that point. In the example above `start` is defined at the beginning of the program so in the entire program `start` will hold a value of `0x0800` since that is the entry point of code. However since it uses labels instead of hard coded constants this program can be put anywhere in memory and will still work. Labels can also be useful in jumping to different points in execution.
```nasm
; LOAD 1 INTO A
init:
VOID 0x01
LDA 0, init + 8

; WE DONT WANT TO SWAP
; B IS 0 BY DEFAULT
JMPBZ skip_swap
SWP
skip_swap:

; A IS STILL 1 AT EXECUTION END
```

### Arithmetic

In these examples you may have noticed we are using arithmetic to calculate relative addresses. This is very powerful but there are limits, since all arithmetic is done at assembly time the only operators allowed are `+` and `-`. Similarly to pointer math the only sensible operations are relative shifts to numbers or labels. For example, `label1 + label2` is not valid syntax because nothing meaningful comes from adding 2 labels. In the future `label1 - label2` will likely be implemented as finding the program size between labels could be useful in determining memory allocation but currently is not implemented.

### Macros and Preprocessing

The real power of the assembler is in the preprocessor. Since doing very simple tasks is incredibly tedious, even using UASM, the assembler comes with a built in preprocessor that allows for macro definitions. Instead of writing
```nasm
SWP
LDA 0, addr
SWP
```
every time you wanted to load a value to B, you could write
```nasm
.MACRO LDB x, y
SWP
LDA %x, %y
SWP
.ENMAC
```
then call `LDB 0, addr` like a regular instruction in your code. The preprocessor will expand every LDB call into the text between the macro definition line `.MACRO ...` and `.ENMAC`. Parameters are treated as constants since after preprocessing they become constant values before being sent into the codegen stage. Macro definition is as follows: 
```nasm
.MACRO <MACRO NAME> <params>
<macro_body>
.ENMAC
```
Macros can be nested in other macros however since macros are based on text expansion rather than a call stack like functions they cannot be self referential or recursive. Macro expansion is done in passes where all macros are stored then the code is analyzed for macro calls. If a macro call is detected it is expanded based on the values the macro is called with then the entire code is reparsed until there are no more macros to call or the `MAX_MACRO_RECURSION` limit is hit. For example:
```nasm
.MACRO SETA x
v:
VOID %x
LDA 0, v + 8
.ENMAC

.MACRO SETB x
SWP
SETA %x
SWP
.ENMAC

SETB 0x10
```
will be expanded to
```nasm
SWP
SETA 0x10
SWP
```
then to
```nasm
SWP
v_0:
VOID 0x10
LDA 0, v_0 + 8
SWP
```
and finally assembled to
```
03 0B 00 00  00 00 00 00
00 10 01 00  00 00 00 00
00 00 08 09  03
```
### Assembler Directives

Assembler directives are instructions that don't have opcodes and aren't executed on the virtual machine but tell the assembler itself how to generate code. These directives start with the `$` prefix. For example the only current assembler directive implemented is PC which tells the assembler what the value of the program counter should be at the point of byte emission. For example in the UNIBL standard library the first 256 bytes are reserved for the call stack meaning the program counter needs to know to start at `0x900` instead of `0x0800` during both execution and codegen time. To resolve this `$PC` is implemented with an immediate jump to address `0x900`
```nasm
; Jump ahead 0x100 addresses to clear memory for call stack
ENTRY_POINT:
VOID ENTRY_POINT + 0x100
LDA 1, ENTRY_POINT + 7
JMPA

$PC ENTRY_POINT + 0x100
; Assembler is now synced with execution-time PC
...
```

## The UNIBL Standard Library
(Work in progress)

