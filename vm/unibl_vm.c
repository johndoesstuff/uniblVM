#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <time.h>
#include "../inc/unibl.h"
#include "../common/instruction.h"

uint8_t* MEM;
uint64_t ACC_A;
uint64_t ACC_B;
uint64_t PC = ENTRY_POINT;

int DEBUG;
int DEV;
int DUMP;
uint64_t DUMP_START;
uint64_t DUMP_END;
int EXPAND;
int STATS;
int STATS_DET;
int COMPLEXITY;
size_t bytes_loaded = 0;
size_t bytes_read = 0;

// LOAD BYTE FROM PROGRAM
uint8_t read_u8() {
	uint8_t u = MEM[PC++];
	bytes_loaded++;
	if (DEV) printf("Loaded u8 %" PRIX8 "\n", u);
	return u;
}

// LOAD 8 BYTES FROM PROGRAM (64 BIT NUMBER)
uint64_t read_u64() {
	uint64_t u64 = 0;
	for (int i = 0; i < 8; i++) {
		u64 |= (uint64_t)read_u8() << (i * 8);
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
	
	bytes_read = fread(MEM + ENTRY_POINT, 1, MEM_SIZE - ENTRY_POINT, f);
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
	EXPAND = 0;
	STATS = 0;
	STATS_DET = 0;
	COMPLEXITY = 0;
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
		} else if (strcmp(*argv, "-e") == 0 || strcmp(*argv, "--expand") == 0) {
			DEBUG = 1;
			EXPAND = 1;
		} else if (strcmp(*argv, "-s") == 0 || strcmp(*argv, "--stats") == 0) {
			STATS = 1;
		} else if (strcmp(*argv, "-S") == 0 || strcmp(*argv, "--stats-detail") == 0) {
			STATS = 1;
			STATS_DET = 1;
		} else if (strcmp(*argv, "-c") == 0 || strcmp(*argv, "--complexity") == 0) {
			COMPLEXITY = 1;
		} else {
			load_binary(*argv);
		}
	}

	int stats[16];
	size_t cycles = 0;
	struct timespec start_time, end_time;
	if (COMPLEXITY) {
		clock_gettime(CLOCK_MONOTONIC_RAW, &start_time);
	}


	// LOAD PROGRAM BINARY

	while (1) {
		uint8_t op = read_u8();
		if (STATS) stats[op]++;
		//if (DEBUG) printf("Loaded op %u\n", op);
		uint64_t spc = PC;

		if (op == HALT) {
			break;
		} else if (op == LDA) {
			uint8_t offset = 8*read_u8();
			uint64_t addr = read_u64();
			uint8_t width = read_u8();
			if (offset + (width - 1) * 8 >= 64) continue;
			for (int i = 0; i < width; i++) {
				ACC_A &= ~((uint64_t)0xff << (offset + i*8));
				ACC_A |= (uint64_t)MEM[addr + i] << (offset + i*8);
			}
			if (DEBUG) printf("%-10s %-5u %-5" PRIX64 " %-5u", "LDA", offset, addr, width);
		} else if (op == STA) {
			uint64_t addr = read_u64();
			uint8_t offset = 8*read_u8();
			uint8_t width = read_u8();
			if (offset + (width - 1) * 8 >= 64) continue;
			for (int i = 0; i < width; i++) {
				MEM[addr + i] = 0xff & (ACC_A >> (offset + i*8));
			}
			if (DEBUG) printf("%-10s %-5" PRIX64 " %-5u %-5u", "STA",  addr, offset, width);
		} else if (op == SWP) {
			uint64_t tmp = ACC_A;
			ACC_A = ACC_B;
			ACC_B = tmp;
			if (DEBUG) printf("%-25s", "SWP");
		} else if (op == JMP) {
			uint64_t addr = read_u64();
			PC = addr;
			if (DEBUG) printf("%-10s %-15" PRIX64, "JMP", addr);
		} else if (op == JMPBZ) {
			uint64_t addr = read_u64();
			if (ACC_B == 0) {
				PC = addr;
			}
			if (DEBUG) printf("%-10s %-15" PRIX64, "JMPBZ", addr);
		} else if (op == ADDAB) {
			ACC_A += ACC_B;
			if (DEBUG) printf("%-25s", "ADDAB");
		} else if (op == SUBAB) {
			if (ACC_B > ACC_A) {
				ACC_A = 0;
			} else {
				ACC_A -= ACC_B;
			}
			if (DEBUG) printf("%-25s", "SUBAB");
		} else if (op == CMPAB) {
			ACC_B = (ACC_A == ACC_B) ? 0 : 1;
			if (DEBUG) printf("%-25s", "CMPAB");
		} else if (op == VOID) {
			uint64_t u64 = read_u64();
			if (DEBUG) printf("%-10s %-15" PRIX64, "VOID", u64);
		} else if (op == NANDAB) {
			ACC_A = ~(ACC_A & ACC_B);
			if (DEBUG) printf("%-25s", "NANDAB");
		} else if (op == SHRA) {
			uint8_t shift = read_u8();
			ACC_A >>= shift;
			if (DEBUG) printf("%-10s %-15u", "SHRA", shift);
		} else {
			fprintf(stderr, "Invalid opcode: %u at PC=%" PRIX64 "\n", op, PC - 1);
			exit(1);
		}
		if (EXPAND) printf("\t\t\tA=0x%-8" PRIX64, ACC_A);
		if (EXPAND) printf("\tB=0x%-8" PRIX64, ACC_B);
		if (EXPAND) printf("\tPC=0x%-8" PRIX64, spc);
		if (DEBUG) printf("\n");
		cycles++;
	}

	// DEBUG INFORMATION
	if (DEBUG) {
		printf("Program execution ended at PC=0x%" PRIX64 "\n", PC - 1);
		printf("ACC_A=0x%" PRIX64 "\n", ACC_A);
		printf("ACC_B=0x%" PRIX64 "\n", ACC_B);
	}
	
	if (STATS) {
		int sum = 0;
		for (int i = 0; i < INSTRUCTION_COUNT; i++) {
			sum += stats[i];
		}
		printf("Execution stats:\n");
		for (int i = 0; i < INSTRUCTION_COUNT; i++) {
			printf("%s:\t%5d\t%7.2f%%\n", INSTRUCTION_TABLE[i].name, stats[i], (float)stats[i]/sum*100);
		}
	}

	if (COMPLEXITY) {
		printf("%lu Program Size\n", bytes_read);
		printf("%lu Bytes Loaded\n", bytes_loaded);
		printf("%lu Cycles\n", cycles);
		clock_gettime(CLOCK_MONOTONIC_RAW, &end_time);
		uint64_t micros = (end_time.tv_sec - start_time.tv_sec) * 1000000
			+ (end_time.tv_nsec - start_time.tv_nsec) / 1000;
		printf("%lu Microseconds\n", micros);
	}

	if (DUMP) {
		printf("Dumping Memory 0x%" PRIX64 "-0x%" PRIX64 "\n", DUMP_START, DUMP_END);
		dump_memory(DUMP_START, DUMP_END);
	}

	// OUTPUT STANDARD OUTPUT (0x0400-0x07FF)
	if (DEBUG) {
		printf("Dumping Standard Output...\n");
	}
	for (uint64_t addr = 0x0400; addr < 0x07FF; addr++) {
		printf("%c", MEM[addr]);
	}
}
