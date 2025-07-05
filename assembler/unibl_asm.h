#include <stdint.h>

#ifndef UNIBL_ASM_H
#define UNIBL_ASM_H

#define MAX_OPERANDS 8

extern int current_pass;

typedef struct {
	uint64_t values[MAX_OPERANDS];
	int count;
} OperandList;

typedef struct Label {
	char *name;
	uint64_t address;
	struct Label *next;
} Label;

OperandList *make_operand_list(uint64_t val);
OperandList *append_operand(OperandList *list, uint64_t val);

void add_i(const char *instr, OperandList *ops, uint64_t *pc);
void directive_i(const char *instr, OperandList *ops, uint64_t *pc);
void add_si(const char *instr, uint64_t *pc);
void directive_si(const char *instr, uint64_t *pc);
void add_label(const char *label, uint64_t *pc);
uint64_t get_label(const char *label, int directive_override);



#endif
