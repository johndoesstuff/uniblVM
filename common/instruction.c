#include "instruction.h"

#include "instruction_args.def"

#define X(name_a, opcode_a, argc_a, argsz_a, name_str) const InstructionInfo INSTR_##name_a = { \
	.name = name_str, \
	.opcode = opcode_a, \
	.argc = argc_a, \
	.argsz = argsz_a \
};
#include "instruction.def"
#undef X

// idc fuck X macros, bite me!
InstructionInfo INSTRUCTION_TABLE[] = {
	{ "HALT",   0x00, 0, HALT_args },
	{ "LDA",    0x01, 3, LDA_args },
	{ "STA",    0x02, 3, STA_args },
	{ "SWP",    0x03, 0, SWP_args },
	{ "JMP",    0x04, 1, JMP_args },
	{ "JMPBZ",  0x05, 1, JMPBZ_args },
	{ "ADDAB",  0x06, 0, ADDAB_args },
	{ "SUBAB",  0x07, 0, SUBAB_args },
	{ "LDAB",   0x08, 2, LDAB_args },
	{ "STAB",   0x09, 2, STAB_args },
	{ "CMPAB",  0x0A, 0, CMPAB_args },
	{ "VOID",   0x0B, 1, VOID_args },
	{ "NANDAB",   0x0C, 0, NANDAB_args },
};
int INSTRUCTION_COUNT = sizeof(INSTRUCTION_TABLE) / sizeof(INSTRUCTION_TABLE[0]);
