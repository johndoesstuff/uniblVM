#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "../inc/unibl.h"

uint8_t* MEM;
uint64_t ACC_A;
uint64_t ACC_B;
uint64_t PC = ENTRY_POINT;

// LOAD BYTE FROM PROGRAM
uint8_t read_u8() {
	// EVERY BYTE READ INCREMENTS THE PROGRAM COUNTER
	uint8_t u = MEM[PC++];
	return u;
}

// LOAD 8 BYTES FROM PROGRAM (64 BIT BIG ENDIAN NUMBER)
uint64_t read_u64() {
	uint64_t u64 = 0;
	// READING A U64 JUST READS 8 BYTES AND COMBINES THEM
	// PROGRAM COUNTER IS UPDATED AS SUCH
	for (int i = 0; i < 8; i++) {
		u64 = (u64 << 8) | read_u8();
	}
	return u64;
}

// OPEN FILE AND LOAD AS BINARY
void load_binary(const char* filename) {
	FILE* file = fopen(filename, "rb");
	if (!file) {
		fprintf(stderr, "Error opening file %s", filename);
		exit(1);
	}
	// IMPLEMENTATION OF IMPORTING CODE IS PLATFORM SPECIFIC
	size_t bytes_read = fread(MEM + ENTRY_POINT, 1, MEM_SIZE - ENTRY_POINT, file);
	if (ferror(file)) {
		fprintf(stderr, "Error reading file %s", filename);
		fclose(file);
		exit(1);
	}
	fclose(file);
}

int main(int argc, char** argv) {
	// VM MUST BE USED WITH A PROGRAM BINARY
	if (argc < 2) {
		printf("Usage: %s program.bin\n", argv[0]);
		exit(0);
	}

	// MEM_SIZE IS ARBITRARY, TO SUPPORT ALL ADDRESS SPACE IMPLEMENTING VIRTUAL MEMORY IS CRUCIAL
	MEM = calloc(MEM_SIZE, sizeof(uint8_t));

	// LOAD PROGRAM BINARY
	load_binary(argv[1]);

	// EXECUTION LOOP
	while (1) {
		uint64_t PC_AT_OP = PC;
		uint8_t op = read_u8();

		if (op == HALT) {
			// A HALT OP WILL JUST EXIT THE EXECUTION LOOP
			break;
		} else if (op == LDA) {
			// INTERNAL OFFSET IS MEASURED IN BITS NOT BYTES
			// FOR BITSHIFTING MATH, AS SUCH MULTIPLY OFFSET
			// BY 8
			uint8_t offset = 8*read_u8();
			// LDA ADDRESS FOLLOWS AFTER OFFSET
			uint64_t addr = read_u64();
			// BIT OFFSET CANNOT BE MORE THAN 64 (BYTE OFFSET
			// OF 8) BEFORE EXCEEDING 64 BIT MEMORY CONSTRAINTS
			if (offset >= 64) continue;
			// CLEAR BITS IN OFFSET OF ACC_A
			ACC_A &= ~((uint64_t)0xFF << offset);
			// SET BITS IN OFFSET OF ACC_A TO MEM[addr]
			ACC_A |= (uint64_t)MEM[addr] << offset;
		} else if (op == STA) {
			// STA OPERATES IDENTICALLY TO LDA BUT SWAPPING
			// OFFSET AND ADDRESS ORDER
			uint64_t addr = read_u64();
			uint8_t offset = 8*read_u8();
			// AGAIN OFFSET CANNOT BE MORE THAN 8 BYTES
			if (offset >= 64) continue;
			// SET MEM[addr] TO OFFSET OF ACC_A
			MEM[addr] = 0xFF & (ACC_A >> offset);
		} else if (op == SWP) {
			// SWAP ACC_A AND ACC_B (DONE USING XOR TO
			// DEMONSTRATE A TEMP VARIABLE IS NOT NECESSARY)
			ACC_A = ACC_A ^ ACC_B;
			ACC_B = ACC_A ^ ACC_B;
			ACC_A = ACC_A ^ ACC_B;
		} else if (op == JMPA) {
			// SET PROGRAM COUNTER TO ACC_A
			PC = ACC_A;
		} else if (op == JMPBZ) {
			// SET PROGRAM COUNTER TO addr IF ACC_B IS ZERO
			uint64_t addr = read_u64();
			if (ACC_B == 0) {
				PC = addr;
			}
		} else if (op == ADDAB) {
			// ADD ACC_B TO ACC_A
			ACC_A += ACC_B;
		} else if (op == SUBAB) {
			// SUBTRACTION IS SATURATED, THIS IS IMPORTANT
			// FOR BOOTSTRAPPING GEQ AND LEQ LOGIC
			if (ACC_B > ACC_A) {
				ACC_A = 0;
			else {
				ACC_A -= ACC_B;
			}
		} else if (op == LDAB) {
			// LDAB IS IMPLEMENTED IDENTICAL TO LDA BUT
			// USES B AS ADDRESS
			uint8_t offset = 8*read_u8();
			if (offset >= 64) continue;
			ACC_A &= ~(0xFF << offset);
			ACC_A |= MEM[ACC_B + offset/8] << offset;
		} else if (op == STAB) {
			// STAB IS IMPLEMENTED IDENTICAL TO STA BUT
			// USES B AS ADDRESS
			uint8_t offset = 8*read_u8();
			if (offset >= 64) continue;
			MEM[ACC_B + offset/8] = 0xFF & (ACC_A >> offset);
		} else if (op == CMPAB) {
			// SET B TO 0 IF ACC_A = ACC_B OTHERWISE SET B TO 1
			ACC_B = (ACC_A == ACC_B) ? 0 : 1;
		} else if (op == VOID) {
			// READ 8 BYTES AND DO NOTHING
			uint64_t u64 = read_u64();
		} else if (op == LDPCA) {
			// ITS CRITICAL THAT LDPCA LOADS THE PC AT THE LDPCA INSTRUCTION
			ACC_A = PC_AT_OP;
		} else {
			// ANY OTHER OPCODE IS NOT VALID
			// THIS IS ALL THAT IS REQUIRED TO MAKE A
			// COMPLETE UNIBL VIRTUAL MACHINE
			fprintf(stderr, "Invalid opcode: %u at PC=%" PRIX64 "\n", op, PC_AT_OP);
			exit(1);
		}
	}

	// OUTPUT STANDARD OUTPUT (0x0400-0x07FF)
	for (uint64_t addr = 0x0400; addr < 0x07FF; addr++) {
		printf("%c", MEM[addr]);
	}
}
