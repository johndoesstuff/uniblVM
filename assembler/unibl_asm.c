#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../inc/unibl.h"

#define _PC ENTRY_POINT + program_size

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

uint64_t _stdout = 0x400;

void _lda(uint8_t offset, uint64_t addr) {
	emit_byte(LDA);
	emit_byte(offset);
	emit_u64(addr);
}

void _sta(uint64_t addr, uint8_t offset) {
	emit_byte(STA);
	emit_u64(addr);
	emit_byte(offset);
}

void _swp() {
	emit_byte(SWP);
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

void _ldb(uint8_t offset, uint64_t addr) {
	_swp();
	_lda(offset, addr);
	_swp();
}

void _lda64(uint64_t addr) {
	for (int i = 0; i < 8; i++) {
		_lda(i, addr + i);
	}
}

void _ldb64(uint64_t addr) {
	_swp();
	_lda64(addr);
	_swp();
}

void _lda64r(uint64_t addr) {
	for (int i = 7; i >= 0; i--) {
		_lda(i, addr + 7 - i);
	}
}

void _ldb64r(uint64_t addr) {
	_swp();
	_lda64r(addr);
	_swp();
}

void _sta64(uint64_t addr) {
	for (int i = 0; i < 8; i++) {
		_sta(addr + i, i);
	}
}

void _stb64(uint64_t addr) {
	_swp();
	_sta64(addr);
	_swp();
}

void _inca() {
	uint64_t ap = _PC;
	_void(0);
	uint64_t bp = _PC;
	_void(0);
	_sta64(ap + 1);
	_stb64(bp + 1);
	uint64_t ldaop = _PC;
	_lda(0, ap); // load void op into a
	_ldb(0, ldaop); // load lda op into b
	_cmpab(); // false so b = 1
	_lda64(ap + 1); // load a
	_addab(); // a += 1
	_ldb64(bp + 1); // load b
}

void _incb() {
	_swp();
	_inca();
	_swp();
}

int main(int argc, char** argv) {
	if (argc < 2) {
		printf("Usage: %s program.uasm\n", argv[0]);
		exit(0);
	}

	FILE *f = fopen(argv[1], "r");
	if (!f) {
		fprintf(stderr, "Failed to open file %s", argv[1]);
		exit(1);
	}

	char *line = NULL;
	size_t len = 0;
	ssize_t read;

	while ((read = getline(&line, &len, file)) != -1) {
		if (line[read - 1] == '\n') line[--read] = '\0';
		if (line[0] == '\0' || line[0] == ';') continue;
	}

	write_program_to_file("program.bin");

	free(program);
	return 0;
}
