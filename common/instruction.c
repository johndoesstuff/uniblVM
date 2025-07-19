// instruction.c
#include "instruction.h"

// Define the actual argument arrays only once
#include "instruction_args.def"

#define X(name_a, opcode_a, argc_a, argsz_a, name_str) const InstructionInfo INSTR_##name_a = { \
        .name = name_str, \
        .opcode = opcode_a, \
        .argc = argc_a, \
        .argsz = argsz_a \
    };
#include "instruction.def"
#undef X
