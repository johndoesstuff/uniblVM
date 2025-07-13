#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include "../inc/unibl.h"

uint8_t* MEM;
uint64_t ACC_A;
uint64_t ACC_B;
uint64_t PC = ENTRY_POINT;

int DEBUG;
int DEV;
int DUMP;
int DUMP_START;
int DUMP_END;

// LOAD BYTE FROM PROGRAM
uint8_t read_u8() {
	uint8_t u = MEM[PC++];
	if (DEV) printf("Loaded u8 %" PRIX8 "\n", u);
	return u;
}

// LOAD 8 BYTES FROM PROGRAM (64 BIT NUMBER)
uint64_t read_u64() {
	uint64_t u64 = 0;
	for (int i = 0; i < 8; i++) {
		u64 = (u64 << 8) | read_u8();
	}
	if (DEV) printf("Loaded u64 %" PRIX64 "\n", u64);
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
	if (DEV) printf("Loaded %zu bytes from %s\n", bytes_read, filename);
}

void dump_memory(uint64_t start, uint64_t end) {
	for (uint64_t addr = start; addr <= end; addr += 8) {
		printf("%04" PRIX64 ": ", addr);
		for (int i = 0; i < 8 && addr + i <= end; i++) {
			printf("%02X ", MEM[addr + i]);
		}
		printf("\n");
	}
}

int main(int argc, char** argv) {
	// VM MUST BE USED WITH A PROGRAM BINARY
	if (argc < 2) {
		printf("Usage: %s program.bin\n", argv[0]);
		exit(0);
	}

	MEM = calloc(MEM_SIZE, sizeof(uint8_t));

	// FLAGS
	DEBUG = 0;
	DEV = 0;
	DUMP = 0;
	DUMP_START = 0;
	DUMP_END = 0;
	argv++;
	for (int i = 1; i < argc; argv++, i++) {
		if (strcmp(*argv, "-d") == 0 || strcmp(*argv, "--debug") == 0) {
			DEBUG = 1;
		} else if (strcmp(*argv, "-D") == 0 || strcmp(*argv, "--dev") == 0) {
			DEBUG = 1;
			DEV = 1;
		} else if (strcmp(*argv, "-i") == 0 || strcmp(*argv, "--dump") == 0) {
			if (i + 1 >= argc) {
				fprintf(stderr, "Error: Missing argument for --dump\n");
				exit(1);
			}
			char *range = *(++argv);
			i++;
			char *sep = strchr(range, ':');
			if (!sep) {
				fprintf(stderr, "Error: Invalid dump range format. Use -i start:end\n");
				exit(1);
			}
			*sep = '\0';
			DUMP_START = strtoull(range, NULL, 0);
			DUMP_END = strtoull(sep + 1, NULL, 0);
			DUMP = 1;
		} else {
			load_binary(*argv);
		}
	}

	// LOAD PROGRAM BINARY

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
			if (DEBUG) printf("LDA %u %" PRIX64 "\n", offset, addr);
		} else if (op == STA) {
			uint64_t addr = read_u64();
			uint8_t offset = 8*read_u8();
			if (offset >= 64) continue;
			MEM[addr] = 0xff & (ACC_A >> offset);
			if (DEBUG) printf("STA %" PRIX64 " %u\n", addr, offset);
		} else if (op == SWP) {
			uint64_t tmp = ACC_A;
			ACC_A = ACC_B;
			ACC_B = tmp;
			if (DEBUG) printf("SWP\n");
		} else if (op == JMPA) {
			PC = ACC_A;
			if (DEBUG) printf("JMPA\n");
			if (DEBUG) printf("ACC_A=%" PRIX64 "\n", ACC_A);
		} else if (op == JMPBZ) {
			uint64_t addr = read_u64();
			if (ACC_B == 0) {
				PC = addr;
			}
			if (DEBUG) printf("JMPBZ %" PRIX64 "\n", addr);
		} else if (op == ADDAB) {
			if (DEBUG) printf("ADDAB\n");
			ACC_A += ACC_B;
		} else if (op == SUBAB) {
			if (ACC_B > ACC_A) {
				ACC_A = 0;
			} else {
				ACC_A -= ACC_B;
			}
			if (DEBUG) printf("SUBAB\n");
		} else if (op == LDAB) {
			uint8_t offset = 8*read_u8();
			if (offset >= 64) continue;
			ACC_A &= ~(0xff << offset);
			ACC_A |= MEM[ACC_B + offset/8] << offset;
			if (DEBUG) printf("LDAB %u\n", offset);
		} else if (op == STAB) {
			uint8_t offset = 8*read_u8();
			if (offset >= 64) continue;
			MEM[ACC_B + offset/8] = 0xff & (ACC_A >> offset);
			if (DEBUG) printf("STAB %u\n", offset);
		} else if (op == CMPAB) {
			ACC_B = (ACC_A == ACC_B) ? 0 : 1;
			if (DEBUG) printf("CMPAB\n");
		} else if (op == VOID) {
			uint64_t u64 = read_u64();
			if (DEBUG) printf("VOID %" PRIX64 "\n", u64);
		} else if (op == LDPCA) {
			ACC_A = PC;
			if (DEBUG) printf("LDPCA\n");
		} else {
			fprintf(stderr, "Invalid opcode: %u at PC=%" PRIX64 "\n", op, PC - 1);
			exit(1);
		}
	}

	// DEBUG INFORMATION
	if (DEBUG) {
		printf("Program execution ended at PC=%" PRIX64 "\n", PC - 1);
		printf("ACC_A=%" PRIX64 "\n", ACC_A);
		printf("ACC_B=%" PRIX64 "\n", ACC_B);
		printf("Dumping Standard Output...\n");
	}

	if (DUMP) dump_memory(DUMP_START, DUMP_END);

	// OUTPUT STANDARD OUTPUT (0x0400-0x07FF)
	for (uint64_t addr = 0x0400; addr < 0x07FF; addr++) {
		printf("%c", MEM[addr]);
	}
}
