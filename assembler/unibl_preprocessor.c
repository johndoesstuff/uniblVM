#include "unibl_preprocessor.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INITIAL_CAPACITY 2

Macro* current_macro;
MacroList* macros;

void initialize_macros() {
	macros = malloc(sizeof(MacroList));
	macros->count = 0;
	macros->capacity = INITIAL_CAPACITY;
	macros->macros = malloc(sizeof(Macro) * macros->capacity);
}

MacroParams* make_macro_params(char* param) {
	MacroParams* mp = malloc(sizeof(MacroParams));
	mp->count = 1;
	mp->capacity = INITIAL_CAPACITY;
	mp->params = malloc(sizeof(char*) * mp->capacity);
	mp->params[0] = strdup(param);
	return mp;
}

MacroParams* append_macro_params(MacroParams* mp, char* param) {
	if (mp->count >= mp->capacity) {
		mp->capacity *= 2;
		mp->params = realloc(mp->params, sizeof(char*) * mp->capacity);
	}
	mp->params[mp->count++] = strdup(param);
	return mp;
}

MacroBody* make_macro_body(char* line) {
	MacroBody* mb = malloc(sizeof(MacroBody));
	mb->count = 1;
	mb->capacity = INITIAL_CAPACITY;
	mb->lines = malloc(sizeof(char*) * mb->capacity);
	mb->lines[0] = strdup(line);
	return mb;
}

MacroBody* append_macro_body(MacroBody* mb, char* line) {
	if (mb->count >= mb->capacity) {
		mb->capacity *= 2;
		mb->lines = realloc(mb->lines, sizeof(char*) * mb->capacity);
	}
	mb->lines[mb->count++] = strdup(line);
	return mb;
}

void start_macro(char* name) {
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
}

void exit_macro() {
	current_macro = NULL;
}

void add_label_to_macro(char* label) {
	MacroLabels* lb = current_macro->labels;
	if (lb->count >= lb->capacity) {
		lb->capacity *= 2;
		lb->labels = realloc(lb->labels, sizeof(char*) * lb->capacity);
	}
	lb->labels[lb->count++] = strdup(label);
}

int check_label_in_macro(char* label) {
	MacroLabels* lb = current_macro->labels;
	for (size_t i = 0; i < lb->count; i++) {
		if (strcmp(lb->labels[i], label) == 0) return 1;
	}
	return 0;
}

void define_macro(char* name, MacroParams* params, MacroBody* body) {
	current_macro->params = params;
	current_macro->body = body;
}
