#ifndef UNIBL_CODEGEN_H
#define UNIBL_CODEGEN_H

#include <stdint.h>
#include <stddef.h>
#include "../common/instruction.h"

extern uint64_t PC;
extern uint8_t* program;
extern size_t program_size;
extern size_t program_capacity;

void emit_byte(uint8_t byte);
void emit_u64(uint64_t val);
void emit_instruction(InstructionInfo* instruction, OperandList* ops);
void write_program_to_file(const char* filename);

#endif
