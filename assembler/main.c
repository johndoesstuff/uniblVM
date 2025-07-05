#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "unibl_asm.h"
#include "unibl_codegen.h"
#include "../inc/unibl.h"
#include <inttypes.h>
#include "unibl_preprocessor.h"

#define DEBUG 0
#define MAX_MACRO_RECURSION 3

extern FILE *pp_in;
extern FILE *asm_in;
int pp_parse(void);
int asm_parse(void);

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
void add_i(const char *instr, OperandList *ops, uint64_t *pc) {
	if (strcmp(instr, "LDA") == 0) {
		_lda(ops->values[0], ops->values[1]);
	} else if (strcmp(instr, "STA") == 0) {
		_sta(ops->values[0], ops->values[1]);
	} else if (strcmp(instr, "JMPBZ") == 0) {
		_jmpbz(ops->values[0]);
	} else if (strcmp(instr, "LDAB") == 0) {
		_ldab(ops->values[0]);
	} else if (strcmp(instr, "STAB") == 0) {
		_stab(ops->values[0]);
	} else if (strcmp(instr, "VOID") == 0) {
		_void(ops->values[0]);
	} else {
		fprintf(stderr, "Unrecognized instruction: %s\n", instr);
		exit(1);
	}
	*pc = ENTRY_POINT + program_size;
}

// ADDS A SINGLE INSTRUCTION (NO ARGUMENTS)
void add_si(const char *instr, uint64_t *pc) {
	if (strcmp(instr, "JMPA") == 0) {
		_jmpa();
	} else if (strcmp(instr, "SWP") == 0) {
		_swp();
	} else if (strcmp(instr, "ADDAB") == 0) {
		_addab();
	} else if (strcmp(instr, "SUBAB") == 0) {
		_subab();
	} else if (strcmp(instr, "CMPAB") == 0) {
		_cmpab();
	} else if (strcmp(instr, "HALT") == 0) {
		_halt();
	} else {
		fprintf(stderr, "Unrecognized instruction: %s\n", instr);
		exit(1);
	}
	*pc = ENTRY_POINT + program_size;
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
uint64_t get_label(const char *label) {
	if (current_pass == 2) {
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

	// PREPROCESSING STAGE
	initialize_macros();
	pp_in = fopen(argv[1], "r");
	if (!pp_in) {
		perror("Failed to open input file");
		return 1;
	}

	if (pp_parse() == 0) {
		if (DEBUG) printf("First pass completed successfully.\n");
	} else {
		printf("Parsing failed.\n");
	}

	// RESET FOR SECOND PREPROCESSOR PASS
	preprocessor_pass = 2;
	rewind(pp_in);

	if (pp_parse() == 0) {
		if (DEBUG) printf("Second pass completed successfully.\n");
	} else {
		printf("Parsing failed.\n");
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
			printf("Parsing failed.\n");
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
			fprintf(stderr, "MAX_MACRO_RECURSION reached; (%d)", MAX_MACRO_RECURSION);
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
		printf("Parsing failed.\n");
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
		printf("Parsing failed.\n");
	}

	fclose(asm_in);

	// EXPORT AS program.bin
	write_program_to_file("program.bin");
	return 0;
}
