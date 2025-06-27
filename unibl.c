#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#define HALT 0
#define LDA 1
#define LDB 2
#define STA 3
#define STB 4
#define JMPA 5
#define JMPBZ 6
#define ADDAB 7
#define SUBAB 8
#define LDAB 9
#define STAB 10
#define CMPAB 11
#define VOID 12

#define DEBUG 1

#define ENTRY_POINT 0x0800
#define MEM_SIZE 0xFFFF

uint8_t* MEM;
uint64_t ACC_A;
uint64_t ACC_B;
uint64_t PC = ENTRY_POINT;

uint8_t read_u8() {
	uint8_t u = MEM[PC++];
	if (DEBUG) printf("Loaded u8 %u\n", u);
	return u;
}

uint64_t read_u64() {
	uint64_t u64 = 0;
	for (int i = 0; i < 8; i++) {
		u64 = (u64 << 8) | read_u8();
	}
	if (DEBUG) printf("Loaded u64 %lu\n", u64);
	return u64;
}

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
	if (argc < 2) {
		printf("Usage: %s program.bin\n", argv[0]);
		exit(0);
	}

	MEM = calloc(MEM_SIZE, sizeof(uint8_t));

	load_binary(argv[1]);

	while (1) {
		uint8_t op = read_u8();
		if (DEBUG) {
			printf("Loaded op %u\n", op);
		}

		if (op == HALT) {
			break;
		} else if (op == LDA) {
			uint8_t offset = 8*read_u8();
			uint64_t addr = read_u64();
			if (offset >= 64) continue;
			ACC_A &= ~(0xff << offset);
			ACC_A |= MEM[addr] << offset;
		} else if (op == LDB) {
			uint8_t offset = 8*read_u8();
			uint64_t addr = read_u64();
			if (offset >= 64) continue;
			ACC_B &= ~(0xff << offset);
			ACC_B |= MEM[addr] << offset;
		} else if (op == STA) {
			uint64_t addr = read_u64();
			uint8_t offset = 8*read_u8();
			if (offset >= 64) continue;
			MEM[addr] = 0xff & (ACC_A >> offset);
		} else if (op == STB) {
			uint64_t addr = read_u64();
			uint8_t offset = 8*read_u8();
			if (offset >= 64) continue;
			MEM[addr] = 0xff & (ACC_B >> offset);
		} else if (op == JMPA) {
			PC = ACC_A;
		} else if (op == JMPBZ) {
			uint64_t addr = read_u64();
			if (ACC_B == 0) {
				PC = addr;
			}
		} else if (op == ADDAB) {
			ACC_A += ACC_B;
		} else if (op == SUBAB) {
			ACC_A -= ACC_B;
		} else if (op == LDAB) {
			uint8_t offset = 8*read_u8();
			if (offset >= 64) continue;
			ACC_A &= ~(0xff << offset);
			ACC_A |= MEM[ACC_B] << offset;
		} else if (op == STAB) {
			uint8_t offset = 8*read_u8();
			if (offset >= 64) continue;
			MEM[ACC_B] = 0xff & (ACC_A >> offset);
		} else if (op == CMPAB) {
			ACC_B = (ACC_A == ACC_B) ? 0 : 1;
		} else if (op == VOID) {
			read_u64();
		} else {
			fprintf(stderr, "Invalid opcode: %u at PC=%lu\n", op, PC - 1);
			exit(1);
		}
	}
	printf("Program execution ended at PC=%lu\n", PC - 1);
	printf("Dumping Standard Output...\n");
	for (uint64_t addr = 0x0400; addr < 0x07FF; addr++) {
		printf("%c", MEM[addr]);
	}
}
