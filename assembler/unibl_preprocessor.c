#include "unibl_preprocessor.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define DEBUG 0
#define INITIAL_CAPACITY 2

Macro* current_macro;
MacroList* macros;
extern int preprocessor_pass;
int macro_call_id;

// MACROS ARE STORED IN MACRO OBJECTS
void initialize_macros() {
	macro_call_id = 0;
	if (DEBUG) printf("Initializing Macros\n");
	macros = malloc(sizeof(MacroList));
	macros->count = 0;
	macros->capacity = INITIAL_CAPACITY;
	macros->macros = malloc(sizeof(Macro) * macros->capacity);
}

// CREATE MACRO PARAMETER OBJECT
MacroParams* make_macro_params(char* param) {
	if (preprocessor_pass == 1) {
		if (DEBUG) printf("Making Macro Param %s\n", param);
		MacroParams* mp = malloc(sizeof(MacroParams));
		mp->count = 1;
		if (param[0] == ' ') mp->count = 0;
		mp->capacity = INITIAL_CAPACITY;
		mp->params = malloc(sizeof(char*) * mp->capacity);
		mp->params[0] = strdup(param);
		return mp;
	} else {
		return NULL;
	}
}

// APPEND TO MACRO PARAMETER OBJECT
MacroParams* append_macro_params(MacroParams* mp, char* param) {
	if (preprocessor_pass == 1) {
		if (DEBUG) printf("Making Macro Param %s\n", param);
		if (mp->count >= mp->capacity) {
			mp->capacity *= 2;
			mp->params = realloc(mp->params, sizeof(char*) * mp->capacity);
		}
		mp->params[mp->count++] = strdup(param);
		return mp;
	}
	return NULL;
}

// CREATE MACRO BODY OBJECT
MacroBody* make_macro_body(char* line) {
	if (preprocessor_pass != 3) {
		if (DEBUG) printf("Making Macro Body %s\n", line);
		MacroBody* mb = malloc(sizeof(MacroBody));
		mb->count = 1;
		mb->capacity = INITIAL_CAPACITY;
		mb->lines = malloc(sizeof(char*) * mb->capacity);
		mb->lines[0] = strdup(line);
		return mb;
	}
	return NULL;
}

// APPEND TO MACRO BODY OBJECT
MacroBody* append_macro_body(MacroBody* mb, char* line) {
	if (preprocessor_pass != 3) {
		if (DEBUG) printf("Making Macro Body %s\n", line);
		if (mb->count >= mb->capacity) {
			mb->capacity *= 2;
			mb->lines = realloc(mb->lines, sizeof(char*) * mb->capacity);
		}
		mb->lines[mb->count++] = strdup(line);
		return mb;
	}
	return NULL;
}

// SEARCH MACRO TABLE FOR MACRO OF NAME
Macro* find_macro(char* name) {
	if (DEBUG) printf("Finding macro %s\n", name);
	for (size_t i = 0; i < macros->count; i++) {
		if (strcmp(macros->macros[i]->name, name) == 0) {
			if (DEBUG) printf("Found macro %s\n", name);
			return macros->macros[i];
		};
	}
	return NULL;
}

// DEPENDING ON PASS INITIALIZE MACRO OR FIND MACRO
void start_macro(char* name) {
	if (preprocessor_pass == 1) {
		if (DEBUG) printf("Starting macro %s\n", name);
		if (current_macro) {
			fprintf(stderr, "Already defining macro %s\n", current_macro->name);
			exit(1);
		}
		current_macro = malloc(sizeof(Macro));
		current_macro->name = strdup(name);
		current_macro->params = NULL;
		current_macro->body = NULL;
		current_macro->labels = malloc(sizeof(MacroLabels));
		current_macro->labels->count = 0;
		current_macro->labels->capacity = INITIAL_CAPACITY;
		current_macro->labels->labels = malloc(sizeof(char*) * current_macro->labels->capacity);
	} else if (preprocessor_pass == 2) {
		current_macro = find_macro(name);
	}
}

// ENSURE CURRENT MACRO IS OVER IN PARSER
void exit_macro() {
	current_macro = NULL;
}

// ADD A LABEL TO THE MACRO
void add_label_to_macro(char* label) {
	if (preprocessor_pass == 1) {
		if (DEBUG) printf("Adding Macro Label %s\n", label);
		MacroLabels* lb = current_macro->labels;
		if (lb->count >= lb->capacity) {
			lb->capacity *= 2;
			lb->labels = realloc(lb->labels, sizeof(char*) * lb->capacity);
		}
		lb->labels[lb->count++] = strdup(label);
	}
}

// CHECK IF A MACRO CONTAINS A LABEL
// IF A LABEL IS IN A MACRO IT MUST BE HANDLED RELATIVELY, IF NOT IT IS GLOBAL
int check_label_in_macro(char* label) {
	if (preprocessor_pass == 2) {
		if (DEBUG) printf("Searching Macro Label %s\n", label);
		MacroLabels* lb = current_macro->labels;
		for (size_t i = 0; i < lb->count; i++) {
			if (strcmp(lb->labels[i], label) == 0) return 1;
		}
		return 0;
	}
	return 0;
}

// DEFINE OR UPDATE MACRO DEPENDING ON THE PASS
void define_macro(char* name, MacroParams* params, MacroBody* body) {
	if (preprocessor_pass == 1) {
		if (DEBUG) printf("Defined Macro %s\n", name);
		current_macro->params = params;
		current_macro->body = body;
		if (macros->count >= macros->capacity) {
			macros->capacity *= 2;
			macros->macros = realloc(macros->macros, sizeof(Macro) * macros->capacity);
		}
		macros->macros[macros->count++] = current_macro;
	} else if (preprocessor_pass == 2) {
		if (DEBUG) printf("Updated Macro %s\n", name);
		current_macro->body = body;
	}
}

// CONVERT AN INSTRUCTION TO A STRING
char* inst_to_str(char* inst, ArgumentList* arguments) {
	if (!arguments) return inst;

	if (arguments->count > 0) {
		char* st;
		asprintf(&st, "%s %s", inst, arguments->args[0]);
		for (int i = 1; i < arguments->count; i++) {
			char* tmp;
			asprintf(&tmp, "%s, %s", st, arguments->args[i]);
			free(st);
			st = tmp;
		}
		return st;
	}
	return inst;
}

// REPLACE SUBSTRINGS -> USED FOR MACRO SUBSTITUTION (%x -> val)
char* replace_substr(const char* haystack, const char* needle, const char* replacement) {
	if (!haystack || !needle || !replacement) return NULL;

	size_t needle_len = strlen(needle);
	size_t replacement_len = strlen(replacement);

	size_t count = 0;
	const char* tmp = haystack;
	while ((tmp = strstr(tmp, needle))) {
		count++;
		tmp += needle_len;
	}

	size_t new_len = strlen(haystack) + (replacement_len - needle_len) * count + 1;
	char* result = malloc(new_len);
	if (!result) return NULL;

	char* dest = result;
	const char* current = haystack;
	while ((tmp = strstr(current, needle))) {
		size_t len = tmp - current;
		memcpy(dest, current, len);
		dest += len;

		memcpy(dest, replacement, replacement_len);
		dest += replacement_len;

		current = tmp + needle_len;
	}

	strcpy(dest, current);
	return result;
}

// IF A MACRO IS EXPANDABLE EXPAND IT, IF NOT RETURN THE INSTRUCTION STRING
char* check_macro_expansion(char* inst, ArgumentList* arguments) {

	// DONT EXPAND BEFORE EXPANSION STAGE
	if (preprocessor_pass != 3) {
		return inst_to_str(inst, arguments);
	}
	
	// SKIP DIRECTIVES
	if (inst[0] == '$') {
		return inst_to_str(inst, arguments);
	}
	
	// DONT EXPAND KNOWN OPCODES
	static const char* ops[] = {"HALT", "LDA", "STA", "SWP", "JMPA", "JMPBZ", "ADDAB", "SUBAB", "LDAB", "STAB", "CMPAB", "VOID", "LDPCA"};
	for (int i = 0; i < 13; i++) {
		if (strcmp(inst, ops[i]) == 0) return inst_to_str(inst, arguments);
	}

	if (DEBUG) printf("Found macro to expand: %s\n", inst);
	Macro* macro = find_macro(inst);
	if (macro == NULL) {
		fprintf(stderr, "Unexpected instruction %s\n", inst);
		exit(1);
	}
	MacroBody* body = macro->body;
	MacroParams* params = macro->params;
	MacroLabels* labels = macro->labels;
	if (!arguments && params->count != 0) {
		fprintf(stderr, "Mismatch of macro arguments for %s; expected %ld but recieved 0\n", inst, params->count);
		exit(1);
	}
	if (arguments) {
		if (params->count != arguments->count) {
			fprintf(stderr, "Mismatch of macro arguments for %s; expected %ld but recieved %ld\n", inst, params->count, arguments->count);
			exit(1);
		}
	}
	char* st = strdup("");
	for (int i = 0; i < body->count; i++) {
		char* line = strdup(body->lines[i]);

		for (int j = 0; j < params->count; j++) {
			char* param = params->params[j];
			char* arg = arguments->args[j];
			char* key;
			asprintf(&key, "%%%s", param);
			char* replaced = replace_substr(line, key, arg);
			free(line);
			free(key);
			line = replaced;
		}

		for (int j = 0; j < labels->count; j++) {
			char* label = labels->labels[j];
			char* key;
			asprintf(&key, "%%%s", label);
			char* val;
			asprintf(&val, "__macro_%s_%s_%d", inst, label, macro_call_id);
			char* replaced = replace_substr(line, key, val);
			free(line);
			free(key);
			free(val);
			line = replaced;
		}

		char* new_st;
		// NO NEWLINE IF NO MACRO TO EXPAND
		if (line[0] != '\0') {
			if (st[0] != '\0') {
				asprintf(&new_st, "%s\n%s", st, line);
			} else {
				asprintf(&new_st, "%s", line);
			}
		} else {
			asprintf(&new_st, "%s", st);
		}
		free(st);
		free(line);
		st = new_st;
	}
	macro_call_id++;
	return st;
}

// CREATE ARGUMENT LIST OBJECT
ArgumentList* make_argument_list(char* arg) {
	if (preprocessor_pass == 3) {
		if (DEBUG) printf("Making Arg %s\n", arg);
		ArgumentList* al = malloc(sizeof(ArgumentList));
		al->count = 1;
		al->capacity = INITIAL_CAPACITY;
		al->args = malloc(sizeof(char*) * al->capacity);
		al->args[0] = strdup(arg);
		return al;
	}
	return NULL;
}

// APPEND TO ARGUMENT LIST OBJECT
ArgumentList* append_argument_list(ArgumentList* al, char* arg) {
	if (preprocessor_pass == 3) {
		if (DEBUG) printf("Making Arg %s\n", arg);
		if (al->count >= al->capacity) {
			al->capacity *= 2;
			al->args = realloc(al->args, sizeof(char*) * al->capacity);
		}
		al->args[al->count++] = strdup(arg);
		return al;
	}
	return NULL;
}

// INITIALIZE PROGRAM OBJECT
void init_program(Program *program) {
	program = malloc(sizeof(Program));
	program->count = 0;
	program->capacity = INITIAL_CAPACITY;
	program->lines = malloc(sizeof(char*) * program->capacity);
}

// APPEND A LINE TO PROGRAM OBJECT
void append_line(Program *program, char *line) {
	if (preprocessor_pass != 3) return;
	if (program->count >= program->capacity) {
		size_t new_capacity = (program->capacity == 0) ? 8 : program->capacity * 2;
		char **new_lines = realloc(program->lines, new_capacity * sizeof(char *));
		if (!new_lines) {
			fprintf(stderr, "Failed to realloc program lines");
			exit(1);
		}
		program->lines = new_lines;
		program->capacity = new_capacity;
	}

	program->lines[program->count++] = strdup(line);
}
