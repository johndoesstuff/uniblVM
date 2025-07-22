#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unibl_asm.h"
#include "unibl_codegen.h"
#include "include.h"
#include "../inc/unibl.h"
#include <inttypes.h>
#include "unibl_preprocessor.h"
#include "../common/instruction.h"

#define DEBUG 0
#define MAX_MACRO_RECURSION 64

extern FILE *pp_in;
extern FILE *asm_in;
int pp_parse(void);
int asm_parse(void);

int yylineno = 1;

Program program_instance;
Program* program_str = &program_instance;

// ASSEMBLER NEEDS 2 PASSES TO GENERATE LABEL ADDRESSES
int current_pass = 1;
Label *label_table = NULL;

// PREPROCESSOR NEEDS 3 PASSES
// PASS 1: DETERMINE MACROS AND LABELS IN EACH MACRO
// PASS 2: SCOPE LABELS BASED ON MACRO POSITION
// PASS 3: REPLACE MACRO CALLS WITH MACRO CONTENT
int preprocessor_pass = 1;

// OPERANDS ARE STORED AS AN OPERAND LIST
OperandList *make_operand_list(uint64_t val) {
	OperandList *list = malloc(sizeof(OperandList));
	list->count = 1;
	list->values[0] = val;
	return list;
}

// ADD VALUE TO OPERAND LIST
OperandList *append_operand(OperandList *list, uint64_t value) {
	if (list->count < MAX_OPERANDS) list->values[list->count++] = value;
	else {
		printf("Too many operands (max 8)");
		exit(1);
	}
	return list;
}

// ADDS AN INSTRUCTION WITH ARGUMENTS
void add_i(const char *instr_str, OperandList *ops, uint64_t *pc) {
	for (size_t i = 0; i < INSTRUCTION_COUNT; i++) {
		if (strcmp(instr_str, INSTRUCTION_TABLE[i].name) == 0) {
			InstructionInfo* instr = &INSTRUCTION_TABLE[i];
			int opsc = ops == NULL ? 0 : ops->count;
			if (instr->argc != opsc) {
				fprintf(stderr, "Argument mismatch for %s, expected %d but recieved %d", instr_str, instr->argc, opsc);
				exit(1);
			} 
			emit_instruction(instr, ops);
			*pc = ENTRY_POINT + program_size;
			return;
		}
	}
	fprintf(stderr, "Unrecognized instruction: %s\n", instr_str);
	exit(1);
}

// SETS DIRECTIVES WITH ARGUMENTS
void directive_i(const char *instr, OperandList *ops, uint64_t *pc) {
	if (strcmp(instr, "PC") == 0) {
		uint64_t new_pc = ops->values[0];
		if (new_pc < *pc) {
			fprintf(stderr, "Cannot set PC to previous position in execution, %" PRIu64 " %" PRIu64, new_pc, *pc);
			exit(1);
		}
		uint64_t diff = new_pc - *pc;
		for (int i = 0; i < diff; i++) {
			emit_byte(0);
		}
	} else if (strcmp(instr, "DEBUG") == 0) {
		if (current_pass == 2) {
			for (int i = 0; i < ops->count; i++) {
				printf((i == 0 ? "$DEBUG %" PRIu64 : " %" PRIu64), ops->values[i]);
			}
			printf("\n");
		}
	} else {
		fprintf(stderr, "Unrecognized directive: %s\n", instr);
		exit(1);
	}
	*pc = ENTRY_POINT + program_size;
}

// SETS DIRECTIVES WITHOUT ARGUMENTS
void directive_si(const char *instr, uint64_t *pc) {
	if (strcmp(instr, "DEBUG") == 0) {
		if (current_pass == 2) {
			printf("$DEBUG %" PRIu64 "\n", *pc);
		}
	} else {
		fprintf(stderr, "Unrecognized directive: %s\n", instr);
		exit(1);
	}
}

// ADDS A LABEL TO LABEL TABLE
// LABELS ARE JUST THE CURRENT VALUE OF THE PROGRAM COUNTER
void add_label(const char *label, uint64_t *pc) {
	if (current_pass == 1) {
		Label *new_label = malloc(sizeof(Label));
		new_label->next = label_table;
		new_label->name = strdup(label);
		new_label->address = *pc;
		label_table = new_label;
		if (DEBUG) printf("Added label %s at %" PRIu64 "\n", label, *pc);
	}
}

// RETURNS A LABEL FROM THE LABEL TABLE
uint64_t get_label(const char *label, int directive_override) {
	if (current_pass == 2 || directive_override) {
		if (DEBUG) printf("Searching for label %s\n", label);
		Label *current = label_table;
		while (current) {
			if (strcmp(current->name, label) == 0) return current->address;
			current = current->next;
		}
		printf("Could not find label %s\n", label);
	}
	return 0;
}

int main(int argc, char *argv[]) {
	// MUST HAVE FILE PASSED IN
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <input.uasm>\n", argv[0]);
		return 1;
	}

	char* expanded_input;
	char* output_file;

	for (int i = 1; i < argc; argv++, i++) {
		if (strcmp(*argv, "-o") == 0 || strcmp(*argv, "--output") == 0) {
			output_file = strdup(*(++argv));
		} else {
			expanded_input = expand_includes(*argv);
		}
	}

	if (!expanded_input) {
		perror("You must include a file to assemble\n");
		exit(1);
	}

	size_t input_length = strlen(expanded_input);
	//printf("%s", expanded_input);
	FILE* mem_fp = fmemopen(expanded_input, input_length, "r");
	pp_in = mem_fp;

	// PREPROCESSING STAGE
	initialize_macros();
	if (!pp_in) {
		perror("Failed to open input file\n");
		exit(1);
	}

	if (pp_parse() == 0) {
		if (DEBUG) printf("First pass completed successfully.\n");
	} else {
		printf("Parsing failed at PP1.\n");
		exit(1);
	}

	// RESET FOR SECOND PREPROCESSOR PASS
	preprocessor_pass = 2;
	rewind(pp_in);

	if (pp_parse() == 0) {
		if (DEBUG) printf("Second pass completed successfully.\n");
	} else {
		printf("Parsing failed at PP2.\n");
		exit(1);
	}

	// RESET FOR THIRD PREPROCESSOR PASS
	preprocessor_pass = 3;
	rewind(pp_in);
	size_t last_length = 0;

	for (int i = 0; i < MAX_MACRO_RECURSION; i++) {
		// CLEAR program_str BEFORE EACH PASS
		for (int j = 0; j < program_str->count; j++) {
			free(program_str->lines[j]);
		}
		program_str->count = 0;

		if (pp_parse() == 0) {
			if (DEBUG) printf("Third pass completed successfully. (%d/%d)\n", i, MAX_MACRO_RECURSION);
		} else {
			printf("Parsing failed at PP3.\n");
			exit(1);
		}

		// COMBINE PROGRAM INTO STINGLE STRING
		size_t total_length = 0;
		for (int i = 0; i < program_str->count; i++) {
			total_length += strlen(program_str->lines[i]) + 1;
			if (DEBUG) printf("%s\n", program_str->lines[i]);
		}

		char* combined = malloc(total_length + 1);
		combined[0] = '\0';

		for (int j = 0; j < program_str->count; j++) {
			strcat(combined, program_str->lines[j]);
			strcat(combined, "\n");
		}

		// RECYCLE STRING INTO NEW FILE FOR REPARSING
		FILE* new_input = fmemopen(combined, total_length, "r");
		if (!new_input) {
			fprintf(stderr, "fmemopen failed");
			exit(1);
		}

		// THIRD PREPROCESSOR PASS REPEATS UNTIL ALL MACROS ARE RECURSIVELY EXPANDED
		// THIS IS CONFIRMED WHEN THE PROGRAM DOESNT CHANGE SIZES BETWEEN PASSES
		if (last_length == total_length) {
			if (DEBUG) printf("Recursive Expansion Complete\n");
			break;
		}

		// CLEANUP AND RESET
		fclose(pp_in);
		pp_in = new_input;
		last_length = total_length;

		if (i == MAX_MACRO_RECURSION - 1) {
			fprintf(stderr, "MAX_MACRO_RECURSION (%d) reached", MAX_MACRO_RECURSION);
			exit(1);
		}
	}

	// PREPROCESSING OVER, SEND OUTPUT TO ASSEMBLER
	asm_in = pp_in;
	rewind(asm_in);
	if (!asm_in) {
		fprintf(stderr, "Failed to open input file");
		return 1;
	}

	// INITIAL PASS TO GENERATE LABEL ADDRESSES
	if (asm_parse() == 0) {
		if (DEBUG) printf("First pass completed successfully.\n");
	} else {
		printf("Parsing failed at ASM1.\n");
		exit(1);
	}

	// RESET FOR SECOND CODEGEN PASS
	current_pass = 2;
	rewind(asm_in);
	free(program);
	program_capacity = 0;
	uint8_t* program = NULL;
	program_size = 0;

	// SECOND PASS
	if (asm_parse() == 0) {
		if (DEBUG) printf("Second pass completed successfully.\n");
	} else {
		printf("Parsing failed ASM2.\n");
		exit(1);
	}

	fclose(asm_in);

	// EXPORT AS program.bin
	if (!output_file) {
		write_program_to_file("program.bin");
	} else {
		write_program_to_file(output_file);
	}
	return 0;
}
