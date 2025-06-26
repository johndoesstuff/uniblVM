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

#define ENTRY_POINT 0x0800

uint8_t* MEM;
uint64_t ACC_A;
uint64_t ACC_B;
uint64_t PC = ENTRY_POINT;

uint8_t read_u8() {
	return MEM[PC++];
}

uint64_t read_u64() {
	uint64_t r = 0;
	for (int i = 0; i < 8; i++) {
		r = (r << 8) | read_u8();
	}
	return r;
}

int main() {
	MEM = malloc(0xFFFF*sizeof(uint8_t));
	while (1) {
		uint8_t op = read_u8();
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
			if (ACC_B == 0) {
				PC = read_u64();
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
