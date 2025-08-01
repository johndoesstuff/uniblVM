# UNIBL
(Universal Bootstrappable Language)

Writing programs in certain languages comes with portability issues, to run a C program you need a C compiler that targets your specific architecture. Some languages like Java try to solve this by implementing a virtual machine that custom bytecode can compile to. While well defined these virtual machines are still complex, the JVM spec is currently sitting at around 160 pages which is far from something trivial to implement in any system. Other esoteric programming languages like BrainF* take this in the opposite direction- trivial to implement but barely useful. My goal with this project is to write a programming language that is complex enough to define itself and implement it's own behavior using itself, I.E "bootstrapping", but also that is simple enough it could reasonably be implemented on any system as an afternoon project rather than a month long undertaking.

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

### Special Addresses

Unfortunately just having a chunk of memory partitioned for input and output doesn't quite suffice. To overcome only having 1024 input addresses and 1024 output addresses specific input and output addresses can be mapped in certain vm implementations to extend the capabilities of I/O. Currently the most relevant special addresses are

```nasm
STDIN_FLUSH  0x3FE
STDIN_MODE   0x3FF
STDOUT_FLUSH 0x7FE
STDOUT_MODE  0x7FF
```

By default all of these addresses are set to 0. Flush addresses are used to indicate when the end of I/O space has been reached in program space and reload for next input/output chunk to be processed. Setting `STDIN_FLUSH` to a nonzero number signals that the input chunk has been fully read and the program is ready to receive the next chunk of input from the virtual machine. Conversely setting `STDOUT_FLUSH` to a nonzero number signals the virtual machine to output the contents of `STDOUT: 0x400-0x7FE` (note that `STDOUT_MODE` is not zeroed and neither `STDOUT_FLUSH` nor `STDOUT_MODE` are outputted).

### Reading From MEM

Memory is read 1 `u8` (8 bit chunk) at a time. Each UNIBL instruction is stored as a `u8` and their arguments are stored in the appropriate size. Some arguments are only `u8`, (for example determining what section of an accumulator to store 8bits, there are only 8 options) whereas some are `u64` (determining the address of the memory to load).

Note: for every `u8` read it is implied that the program counter is incremented

Since program memory starts at `0x0800` this means your program counter will start initialized to `0x0800`. To execute an instruction you must first read what the instruction is. Reading this `u8` will determine whether you need to read additional memory to construct the full instruction, or if it can be executed immediately. For a no argument instruction like `SWP`, which swaps the values of `ACC_A` and `ACC_B`, you can immediately execute this instruction then return to reading the next op code. For a more complicated instruction like `LDA` which takes a `u8` for the offset of `ACC_A` and a `u64` for the address in `MEM`, you must first read a `u8` for the offset and read a `u64` for the address.

### OP Codes

A full definition of UNIBL opcode numbers can be found in `inc/unibl.h`. Currently there exist 11 opcodes not including `HALT`.

**0 = HALT**

End program execution

*If you are encountering a halt it is likely because you are at the end of a program anyway, since all memory addresses default to 0. Implementing this is useful and highly recommended but not necessary.*

**1 = LDA [u8: offset] [u64: address] [u8: width]**

Load address into A

*Having a function that is able to load an address from memory into an accumulator is absolutely necessary. For information on what* `offset` *means see* **What are offsets?**

**2 = STA [u64: address] [u8: offset] [u8: width]**

Store A into address

*Having a function that is able to store into memory is also absolutely necessary.*

**3 = SWP**

Swap values of A and B

*This is what gives power to the B accumulator, SWP allows any function that works on A to share the same behaviour on B. For example:* `LDB` *could be defined as* `SWP -> LDA -> SWP`

**4 = JMP [u64: address]**

Jump to address

*Being able to jump to any position in program during execution is extremely useful and necessary for turing completeness.*

**5 = JMPBZ [u64: address]**

Jump to address if B is zero

*Jumping is useful but having a conditional jump is also required for turing completeness. You may have noticed that* `JMPBZ` *could potentially eliminate the need for* `JMP` *by always setting B to zero, however this is more tedious and memory inefficient than simply implementing this function into the virtual machine. The power of JMP is that it allows for the value of B to transfer without having to store it in memory.*

**6 = ADDAB**

Add the values of A and B and store the result in A

*Some form of addition is absolutely necessary for computation.*

**7 = SUBAB**

Subtract B from A and store the result in A

*Could subtraction be implemented using the already defined instructions? Yes, but very VERY laboriously; to the point where the compute time would make it not worth it.*

**8 = CMPAB**

Set B to 0 if A and B are equal, otherwise set B to 1

*Comparison is required for conditional jumps, absolutely necessary.*

**9 = VOID [u64: data]**

Do nothing

*Why is it necessary to have an instruction that takes an argument and literally does nothing? Unexpectedly the* `VOID` *command is one of the most useful commands. Voiding a* `u64` *means that argument will be stored in memory so it is pure data that can be referenced later. For example, if a program needs a constant that constant can be passed into* `VOID` *and will appear in program memory at the address of* `VOID + 1`*, this constant can then be referenced and stored as desired using other instructions. Trying to store memory in program space can only be done as an argument to an instruction and trying to use any instruction that does something with its arguments will cause unexpected behaviour.*

**10 = NANDAB**

Take the logical NAND of A and B and store it in A

*NAND is a fundemental building block for all logical operations, anything that can't be implemented using addition or subtraction can be implemented using NAND.*

**11 = SHRA [u8: offset]**

Shift right A by offset and store result in A

*SHRA could be implemented using NAND however unlike SHLA which is A+A there is no simple trick to shift right without iterating over every bit in A which ends up being quite expensive especially when multiple SHRA calls are required.*

### What are offsets?

In a function such as `LDA [u8: offset] [u64: address] [u8: width]` the offset refers to the section of A that the `u8` from memory is to be stored in. Since A is a `u64` BUT memory is a tape of `u8`'s there would be no purpose in having a `u64` register if the maximum value that could be loaded was `255`. Moreover if `255` was the maximum value loadable then only up to `0xFF` of memory would be addressable and the entire system would fall apart. To avoid this `u8`'s are stored in 8 bit sections of `u64` registers. For example:
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
uint8_t offset = 8*read_u8();
uint64_t addr = read_u64();
uint8_t width = read_u8();
if (offset + (width - 1) * 8 >= 64) continue;
for (int i = 0; i < width; i++) {
	ACC_A &= ~((uint64_t)0xff << (offset + i*8));
	ACC_A |= (uint64_t)MEM[addr + i] << (offset + i*8);
}
```
For more precise definitions of instruction sets see `vm/example_vm.c`

### What are widths?

Widths are arguments to `LDA` `STA` `LDAB` and `STAB` that tell the virtual machine how many relative bytes to perform the operation on. For example instead of writing 8 `STA` calls of incrementing offsets to store 8 bytes into A you can write `STA [u64: addr] 0, 8` which tells the virtual machine to store 8 bytes of `addr` into A starting at offset `0` ending at offset `offset + width - 1 => 7`. For this obvious reason `width` cannot be more than `8 - offset` without indexing an out of bounds offset.

### Arguments

The provided UNIBL virtual machine comes with a variety of built in arguments to help debug UASM code. These are not necessary to build a complete virtual machine but can be useful during code execution to analyze performance and other metrics.

**-d --debug**

Prints every instruction run on the virtual machine in text form including the arguments passed into it. Also displays the values of `PC` `ACC_A` and `ACC_B` upon program termination.

**-D --dev**

Includes all of the functionality of **-\-debug** but also displays all byte loads and `u64` loads that lead to interpreting instructions.

**-e --expand**

Prints debug instructions and includes the values of `PC` `ACC_A` and `ACC_B` at the end of every instruction.

**-i --dump [range]** 

Dumps hex of the state of the virtual machine at the end of execution for a given range in memory. For example
```
user@root:~/unibl$ ./unibl_asm test.uasm && ./unibl_vm test.ubc -i 0x400:0x41F
Dumping Memory 0x400-0x41F
0400: 54 45 53 54 0A 00 00 00
0408: 48 65 6C 6C 6F 20 57 6F
0410: 72 6C 64 2C 0A 00 00 00
0418: 57 65 6C 63 6F 6D 65 20
TEST
Hello World,
Welcome to UASM
```

**-s --stats**

Displays execution stats on most commonly used instructions including number of calls and percentage breakdowns.

**-c --complexity**

Displays stats on the complexity of the program executed, including program size, how many bytes loads were called, vm cycles, and execution time.

**-b --break**

Instead of ending execution on a `HALT` op wait for character input then continue execution. Receiving 2 `HALT`s in a row will terminate program execution.

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
LDA 0, 0x0808, 1

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
LDA 0, start + 8, 1
SWP
```
Label definitions are shorthand for "this identifier now means the address that this line exists at in program space." Alternatively defining a label sets that label to mean the value of the program counter at that point. In the example above `start` is defined at the beginning of the program so in the entire program `start` will hold a value of `0x0800` since that is the entry point of code. However since it uses labels instead of hard coded constants this program can be put anywhere in memory and will still work. Labels can also be useful in jumping to different points in execution.
```nasm
; LOAD 1 INTO A
init:
VOID 0x01
LDA 0, init + 8, 1

; WE DONT WANT TO SWAP
; B IS 0 BY DEFAULT
JMPBZ skip_swap
SWP
skip_swap:

; A IS STILL 1 AT EXECUTION END
```

### Arithmetic

In these examples you may have noticed we are using arithmetic to calculate relative addresses. This is very powerful but there are limits, since all arithmetic is done at assembly time the only operators allowed are `+` and `-`. Since the introduction of the `DEF` directive labels can hold meaningful values outside of pointers so arithmetic is allowed in any order between labels and constants and will be computed at codegen time. Since `+` and `-` are the only allowed operations all math is evaluated term by term left to right.

### Constants

Constants are just data that can be immediately evaluated as a number in the assembler. These include static numbers but also characters and strings. The following are valid constants:
```nasm
117
0x75
'u'
"unibl"
```
For strings each character is one byte of memory and strings are encoded using little endian, this is because all of UNIBL uses little endian encoding to store and retrieve values.
```nasm
"unibl" == 0x6c62696e75000000
;            l b i n u . . . 
```
Since arguments to functions can only hold up to 64 bits of information string length is also capped at 8 characters.

Hello World example using strings:
```nasm
; Load 64 bit value into A
.MACRO LDA64 x
LDA 0, %x + 0, 8
.ENMAC

; Reverse and store 64 bits of A
.MACRO STA64 x
STA %x + 0, 0, 8
.ENMAC

; Output starts at 0x0400
$DEF STDOUT 0x400

; Load text into memory
text_0:
VOID "Hello Wo"
text_1:
VOID "rld\n"

; Write to output
LDA64 text_0 + 1
STA64 STDOUT
LDA64 text_1 + 1
STA64 STDOUT + 8
```

```bash
./unibl_asm helloworld.uasm
./unibl_vm helloworld.bin
Hello World
```

### Macros and Preprocessing

The real power of the assembler is in the preprocessor. Since doing very simple tasks is incredibly tedious, even using UASM, the assembler comes with a built in preprocessor that allows for macro definitions. Instead of writing
```nasm
SWP
LDA 0, addr, 1
SWP
```
every time you wanted to load a value to B, you could write
```nasm
.MACRO LDB x, y, z
SWP
LDA %x, %y, %z
SWP
.ENMAC
```
then call `LDB 0, addr, 1` like a regular instruction in your code. The preprocessor will expand every LDB call into the text between the macro definition line `.MACRO ...` and `.ENMAC`. Parameters are treated as constants since after preprocessing they become constant values before being sent into the codegen stage. Macro definition is as follows: 
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
LDA 0, v + 8, 1
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
LDA 0, v_0 + 8, 1
SWP
```
and finally assembled to
```
03 0B 00 00  00 00 00 00
00 10 01 00  00 00 00 00
00 00 08 09  03
```
### Assembler Directives

Assembler directives are instructions that don't have opcodes and aren't executed on the virtual machine but tell the assembler itself how to generate code. These directives start with the `$` prefix.

**$PC**

The assembler directive PC tells the assembler what the value of the program counter should be at the point of byte emission. As an example, this is useful in the UNIBL standard library where the first `0x400` bytes are reserved for the call stack. The program counter needs to know to start at `0x0C00` instead of `0x0800` during both execution and codegen time. To resolve this `$PC` is implemented with an immediate jump to address `0xC00`
```nasm
; Jump ahead 0x400 addresses to clear memory for call stack
$DEF STACK_SIZE 0x400
ENTRY_POINT:
CALLSTACK:
VOID ENTRY_POINT + STACK_SIZE
_CSLDA:
LDA 1, ENTRY_POINT + 7
_CSJMPA:
JMPA

$PC ENTRY_POINT + STACK_SIZE
; Assembler is now synced with execution-time PC
...
```
Note that the PC directive is able to use expressions and labels to determine the program counter at codegen. This may appear to allow for impossible labeling behaviour but the way that directive labels and standard labels work is slightly different. Directive labels are not able to forward reference future labels, only previous labels. This stops impossible situations from arising such as
```nasm
$PC LABEL + 0x100
:LABEL
```
which would recursively tell the assembler that the program counter should be 256 addresses forward of wherever it is after it makes this jump.

**$DEF**

The DEF directive works identically to a label (and is actually syntactic sugar for a label under the hood) that uses a value other than the program counter to define the label. This works since labels are just identifiers that store the value of the program counter. Important to note is that because the DEF directive is operating on assigning values to an identifier there is no comma required during value assignment. For example
```nasm
$DEF IDENT 0x100
```
is valid whereas most instructions require a comma to differentiate arguments. There is potential that in the future this syntax will change and DEF will require a comma to make syntax more cohesive.

**$INCLUDE [FString: file]**

To build useful programs it's essential to be able to include code from previous programs defining macros, constants, and setting up desired environments. Since in UASM string constants are limited to 8 characters and are evaluated to `u64` integers the include directive takes an FString (Full String, File String, I don't care or know) which is a string constructed using angle brackets instead of quotes. For example, to include the standard library in your program:
```nasm
$INCLUDE <uasmlib/stdlib.uasm>
```
This directive works by replacing itself with the entire contents of the file included and it expands recursively

**$DEBUG**

The DEBUG directive is very simple, it will print the values passed into it upon assembly and it can be used to test if labels and definitions are as expected. If no arguments are passed it will print the current value of the program counter.

## The UNIBL Standard Library

### stdlib.uasm

Despite the UNIBL virtual machine only needing to manage memory for input and output, the UNIBL standard library `stdlib.uasm` defines necessary blocks of memory to implement additional functionality into the UASM lanuage. Including the standard library implements the ability to call and return from subroutines, adds variables for standard locations, and adds many macros for essential features.

**CALL [address]**

Add current position to callstack and go into subroutine. When subroutine is finished return to previous execution point.

**RET**

Return from current subroutine to previous position in execution.

**LDB [u8: offset] [u64: address] [u8: width]**

Functions identical to LDA but uses B register.

**STB [u64: address] [u8: offset] [u8: width]**

Functions identical to STA but uses B register.

**LDAB [u8: offset] [u8: width]**

Same as `LDA` but using B + offset as the address.

**STAB [u8: offset] [u8: width]**

Same as `STA` but using B + offset as the address.

**JMPA**

Jump to position in execution at register A.

**LDPCA**

Load the current program counter value into register A.

**LDA64 [u64: address]**

Shorthand for LDA into all of A.

**STA64 [u64: address]**

Shorthand for STA into all of A.

**LDB64 [u64: address]**

Shorthand for LDB into all of B.

**STB64 [u64: address]**

Shorthand for STB into all of B.

### math.uasm

The `math.uasm` library includes functions for doing more complex math operations including additional logical operators and arithmetic operators.

**NOTA**

Logical NOT of A.

**SHL1A**

Left shift A by 1.

**ACCA [u64: address]**

Accumulate the value of A to a position in memory.

**ANDAB**

Logical AND of A and B stored into A.

**SHRB [u8: offset]**

Identical to SHRA but using B register.

**MULAB**

Multiply A and B, store result in A.

**DIVAB**

Divide A and B, store result in B.

### print.uasm

The `print.uasm` library allows for printing to STDOUT of strings and values.

**PRINTA**

Print the hex value of A to STDOUT.

**PRINTB**

Print the hex value of B to STDOUT.

**PRINTN**

Print a newline character to STDOUT.

**PRINT [u64: string]**

Print string to STDOUT.
