// instruction.h
#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdint.h>

typedef struct {
	const char* name;
	uint8_t opcode;
	uint8_t argc;
	const uint8_t* argsz;
} InstructionInfo;

#define X(name, opcode, argc, argsz, name_str) extern InstructionInfo INST_##name;
#include "instruction.def"
#undef X

extern InstructionInfo instruction_table[256];

#endif
