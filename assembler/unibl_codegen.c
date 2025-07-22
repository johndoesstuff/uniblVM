#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../inc/unibl.h"
#include "unibl_asm.h"
#include "../common/instruction.h"

#define _PC ENTRY_POINT + program_size

uint8_t* program = NULL;
size_t program_size = 0;
size_t program_capacity = 0;
extern int current_pass;

int previous_op = 0;

// WRITES A BYTE TO PROGRAM
void emit_byte(uint8_t byte) {
	if (current_pass == 2) {
		if (program_size >= program_capacity) {
			program_capacity = program_capacity ? program_capacity * 2 : 64;
			program = realloc(program, program_capacity);
			if (!program) {
				perror("Failed to realloc program buffer");
				exit(1);
			}
		}
		program[program_size++] = byte;
	} else {
		program_size++;
	}
}

void delete_byte() {
	if (current_pass == 2) {
		program[program_size--] = (uint8_t)0;
	} else {
		program_size--;
	}
}

// WRITES 8 BYTES TO PROGRAM (64 BIT VALUE)
void emit_u64(uint64_t val) {
	for (int i = 0; i < 8; i++) {
		emit_byte((val >> (8 * i)) & 0xFF);
	}
}

// WRITES PROGRAM TO FILE
void write_program_to_file(const char* filename) {
	FILE* f = fopen(filename, "wb");
	if (!f) {
		perror("Failed to open output file");
		exit(1);
	}

	fwrite(program, 1, program_size, f);
	fclose(f);
}

// EMIT INSTRUCTION
void emit_instruction(InstructionInfo* inst, OperandList* ops) {
	// double swaps do nothing
	if (strcmp(inst->name, "SWP") == 0) {
		if (previous_op == inst->opcode) {
			previous_op = -1;
			delete_byte();
			return;
		}
	}
	emit_byte(inst->opcode);
	for (size_t i = 0; i < inst->argc; i++) {
		if (inst->argsz[i] == 1) {
			emit_byte(ops->values[i]);
		} else {
			emit_u64(ops->values[i]);
		}
	}
	previous_op = inst->opcode;
}
