#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "../inc/unibl.h"
#define DEBUG 0

uint8_t* MEM;
uint64_t ACC_A;
uint64_t ACC_B;
uint64_t PC = ENTRY_POINT;

// LOAD BYTE FROM PROGRAM
uint8_t read_u8() {
	uint8_t u = MEM[PC++];
	//if (DEBUG) printf("Loaded u8 %u\n", u);
	return u;
}

// LOAD 8 BYTES FROM PROGRAM (64 BIT NUMBER)
uint64_t read_u64() {
	uint64_t u64 = 0;
	for (int i = 0; i < 8; i++) {
		u64 = (u64 << 8) | read_u8();
	}
	//if (DEBUG) printf("Loaded u64 %lu\n", u64);
	return u64;
}

// OPEN FILE AND LOAD AS BINARY
void load_binary(const char* filename) {
	FILE* f = fopen(filename, "rb");
	if (!f) {
		fprintf(stderr, "Error opening file %s", filename);
		exit(1);
	}
	
	size_t bytes_read = fread(MEM + ENTRY_POINT, 1, MEM_SIZE - ENTRY_POINT, f);
	if (ferror(f)) {
		fprintf(stderr, "Error reading file %s", filename);
		fclose(f);
		exit(1);
	}

	fclose(f);
	if (DEBUG) printf("Loaded %zu bytes from %s\n", bytes_read, filename);
}

int main(int argc, char** argv) {
	// VM MUST BE USED WITH A PROGRAM BINARY
	if (argc < 2) {
		printf("Usage: %s program.bin\n", argv[0]);
		exit(0);
	}

	MEM = calloc(MEM_SIZE, sizeof(uint8_t));

	// LOAD PROGRAM BINARY
	load_binary(argv[1]);

	while (1) {
		uint8_t op = read_u8();
		//if (DEBUG) printf("Loaded op %u\n", op);

		if (op == HALT) {
			break;
		} else if (op == LDA) {
			uint8_t offset = 8*read_u8();
			uint64_t addr = read_u64();
			if (offset >= 64) continue;
			ACC_A &= ~((uint64_t)0xff << offset);
			ACC_A |= (uint64_t)MEM[addr] << offset;
			if (DEBUG) printf("LDA %u %" PRIu64 "\n", offset, addr);
		} else if (op == STA) {
			uint64_t addr = read_u64();
			uint8_t offset = 8*read_u8();
			if (offset >= 64) continue;
			MEM[addr] = 0xff & (ACC_A >> offset);
			if (DEBUG) printf("STA %" PRIu64 " %u\n", addr, offset);
		} else if (op == SWP) {
			uint64_t tmp = ACC_A;
			ACC_A = ACC_B;
			ACC_B = tmp;
			if (DEBUG) printf("SWP\n");
		} else if (op == JMPA) {
			PC = ACC_A;
			if (DEBUG) printf("JMPA\n");
			if (DEBUG) printf("ACC_A=%" PRIu64 "\n", ACC_A);
		} else if (op == JMPBZ) {
			uint64_t addr = read_u64();
			if (ACC_B == 0) {
				PC = addr;
			}
			if (DEBUG) printf("JMPBZ %" PRIu64 "\n", addr);
		} else if (op == ADDAB) {
			if (DEBUG) printf("ADDAB\n");
			ACC_A += ACC_B;
		} else if (op == SUBAB) {
			ACC_A -= ACC_B;
			if (DEBUG) printf("SUBAB\n");
		} else if (op == LDAB) {
			uint8_t offset = 8*read_u8();
			if (offset >= 64) continue;
			ACC_A &= ~(0xff << offset);
			ACC_A |= MEM[ACC_B] << offset;
			if (DEBUG) printf("LDAB %u\n", offset);
		} else if (op == STAB) {
			uint8_t offset = 8*read_u8();
			if (offset >= 64) continue;
			MEM[ACC_B] = 0xff & (ACC_A >> offset);
			if (DEBUG) printf("STAB %u\n", offset);
		} else if (op == CMPAB) {
			ACC_B = (ACC_A == ACC_B) ? 0 : 1;
			if (CMPAB) printf("CMPAB\n");
		} else if (op == VOID) {
			uint64_t u64 = read_u64();
			if (DEBUG) printf("VOID %" PRIu64 "\n", u64);
		} else {
			fprintf(stderr, "Invalid opcode: %u at PC=%" PRIu64 "\n", op, PC - 1);
			exit(1);
		}
	}

	// DEBUG INFORMATION
	if (DEBUG) {
		printf("Program execution ended at PC=%" PRIu64 "\n", PC - 1);
		printf("ACC_A=%" PRIu64 "\n", ACC_A);
		printf("ACC_B=%" PRIu64 "\n", ACC_B);
		printf("Dumping Standard Output...\n");
	}

	// OUTPUT STANDARD OUTPUT (0x0400-0x07FF)
	for (uint64_t addr = 0x0400; addr < 0x07FF; addr++) {
		printf("%c", MEM[addr]);
	}
}
