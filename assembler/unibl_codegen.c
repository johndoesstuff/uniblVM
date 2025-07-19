#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../inc/unibl.h"

#define _PC ENTRY_POINT + program_size

uint8_t* program = NULL;
size_t program_size = 0;
size_t program_capacity = 0;
extern int current_pass;

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
	//printf("Wrote %zu bytes to %s\n", program_size, filename);
}

// HALT PROGRAM
void _halt() {
	emit_byte(HALT);
}

// LOAD INTO REGISTER A
void _lda(uint8_t offset, uint64_t addr, uint8_t width) {
	emit_byte(LDA);
	emit_byte(offset);
	emit_u64(addr);
	emit_byte(width);
}

// STORE REGISTER A INTO MEMORY
void _sta(uint64_t addr, uint8_t offset, uint8_t width) {
	emit_byte(STA);
	emit_u64(addr);
	emit_byte(offset);
	emit_byte(width);
}

// SWAP REGISTER A AND B
void _swp() {
	emit_byte(SWP);
}

// JUMP TO VALUE
void _jmp(uint64_t addr) {
	emit_byte(JMP);
	emit_u64(addr);
}

// JUMP TO ADDRESS IF REGISTER B IS ZERO
void _jmpbz(uint64_t addr) {
	emit_byte(JMPBZ);
	emit_u64(addr);
}

// ADD REGISTER A AND REGISTER B AND STORE IN REGISTER A
void _addab() {
	emit_byte(ADDAB);
}

// SUBTRACT REGISTER A AND REGISTER B AND STORE IN REGISTER A
void _subab() {
	emit_byte(SUBAB);
}

// LOAD THE ADDRESS AT REGISTRY B INTO REGISTRY A
void _ldab(uint8_t offset, uint8_t width) {
	emit_byte(LDAB);
	emit_byte(offset);
	emit_byte(width);
}

// STORE THE VALUE AT REGISTRY A INTO THE ADDRESS IN REGISTRY B
void _stab(uint8_t offset, uint8_t width) {
	emit_byte(STAB);
	emit_byte(offset);
	emit_byte(width);
}

// SET TO 0 IF REGISTRY A AND B ARE EQUAL AND 1 IF NOT
void _cmpab() {
	emit_byte(CMPAB);
}

// DO NOTHING
void _void(uint64_t _data) {
	emit_byte(VOID);
	emit_u64(_data);
}

// LOAD PC INTO REGISTRY A
void _ldpca() {
	emit_byte(LDPCA);
}
