#include <stdlib.h>

typedef struct {
	char** lines;
	size_t count;
	size_t capacity;
} MacroBody;

typedef struct {
	char** params;
	size_t count;
	size_t capacity;
} MacroParams;

typedef struct {
	char** labels;
	size_t count;
	size_t capacity;
} MacroLabels;

typedef struct {
	char* name;
	MacroBody* body;
	MacroParams* params;
	MacroLabels* labels;
} Macro;

typedef struct {
	Macro** macros;
	size_t count;
	size_t capacity;
} MacroList;

typedef struct {
	char** args;
	size_t count;
	size_t capacity;
} ArgumentList;

typedef struct {
	char **lines;
	size_t count;
	size_t capacity;
} Program;

extern Program* program_str;
extern Program program_instance;

MacroBody* make_macro_body(char* line);
MacroBody* append_macro_body(MacroBody* body, char* line);

MacroParams* make_macro_params(char* param);
MacroParams* append_macro_params(MacroParams* params, char* param);

void start_macro(char* macro_name);
void exit_macro();

void define_macro(char* macro_name, MacroParams* macro_params, MacroBody* macro_body);

void add_label_to_macro(char* label);
int check_label_in_macro(char* label);

void initialize_macros();

char* check_macro_expansion(char* inst, ArgumentList* args);

ArgumentList* make_argument_list(char* arg);
ArgumentList* append_argument_list(ArgumentList* al, char* arg);

void init_program(Program* program);
void append_line(Program* program, char* line);
