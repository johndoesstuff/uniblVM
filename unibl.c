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

int main() {
	MEM = malloc(0xFFFF*sizeof(uint8_t));

	MEM[2048] = VOID; //data: "Hello World"
	MEM[2049] = 72;
	MEM[2050] = 101;
	MEM[2051] = 108;
	MEM[2052] = 108;
	MEM[2053] = 111;
	MEM[2054] = 32;
	MEM[2055] = 87;
	MEM[2056] = 111;
	MEM[2057] = VOID;
	MEM[2058] = 114;
	MEM[2059] = 108;
	MEM[2060] = 100;

	MEM[2066] = LDA; //load into a
	MEM[2067] = 0x0; //offset of 0

	MEM[2074] = 0x08; //load 0x0801
	MEM[2075] = 0x01;

	MEM[2076] = STA; //store into output
	MEM[2083] = 0x04;
	MEM[2084] = 0x00;
	MEM[2085] = 0x00;

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
