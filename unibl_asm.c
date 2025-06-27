#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "unibl.h"

uint8_t* program = NULL;
size_t program_size = 0;
size_t program_capacity = 0;

void emit_byte(uint8_t byte) {
	if (program_size >= program_capacity) {
		program_capacity = program_capacity ? program_capacity * 2 : 64;
		program = realloc(program, program_capacity);
		if (!program) {
			perror("Failed to realloc program buffer");
			exit(1);
		}
	}
	program[program_size++] = byte;
}

void emit_u64(uint64_t val) {
	for (int i = 7; i >= 0; i--) {
		emit_byte((val >> (8 * i)) & 0xFF);
	}
}

void write_program_to_file(const char* filename) {
	FILE* f = fopen(filename, "wb");
	if (!f) {
		perror("Failed to open output file");
		exit(1);
	}

	fwrite(program, 1, program_size, f);
	fclose(f);
	printf("Wrote %zu bytes to %s\n", program_size, filename);
}

void _lda(uint8_t offset, uint64_t addr) {
	emit_byte(LDA);
	emit_byte(offset);
	emit_u64(addr);
}

void _ldb(uint8_t offset, uint64_t addr) {
	emit_byte(LDB);
	emit_byte(offset);
	emit_u64(addr);
}

void _sta(uint64_t addr, uint8_t offset) {
	emit_byte(STA);
	emit_u64(addr);
	emit_byte(offset);
}

void _stb(uint64_t addr, uint8_t offset) {
	emit_byte(STB);
	emit_u64(addr);
	emit_byte(offset);
}

void _jmpa() {
	emit_byte(JMPA);
}

void _jmpbz(uint64_t addr) {
	emit_byte(JMPBZ);
	emit_u64(addr);
}

void _addab() {
	emit_byte(ADDAB);
}

void _subab() {
	emit_byte(SUBAB);
}

void _ldab(uint8_t offset) {
	emit_byte(LDAB);
	emit_byte(offset);
}

void _stab(uint8_t offset) {
	emit_byte(STAB);
	emit_byte(offset);
}

void _cmpab() {
	emit_byte(CMPAB);
}

void _void(uint64_t _data) {
	emit_byte(VOID);
	emit_u64(_data);
}

uint64_t _u64_from_char(char* str) {
	int str_l = strlen(str);
	uint64_t u64 = 0;
	for (int i = 0; i < 8; i++) {
		u64 = (u64 << 8) | ((i < str_l) ? str[i] : 0);
	}
	return u64;
}

void _lda64(uint64_t addr) {
	for (int i = 0; i < 8; i++) {
		_lda(i, addr + i);
	}
}

void _sta64(uint64_t addr) {
	for (int i = 0; i < 8; i++) {
		_sta(addr + i, i);
	}
}

int main() {
	_void(_u64_from_char("Hello Wo"));
	_void(_u64_from_char("rld\n"));
	_lda64(0x801);
	_sta64(0x400);
	_lda64(0x80A);
	_sta64(0x408);

	write_program_to_file("program.bin");

	free(program);
	return 0;
}
