#ifndef UNIBL_CODEGEN_H
#define UNIBL_CODEGEN_H

#include <stdint.h>
#include <stddef.h>

extern uint64_t PC;
extern uint8_t* program;
extern size_t program_size;
extern size_t program_capacity;

void emit_byte(uint8_t byte);
void emit_u64(uint64_t val);
void write_program_to_file(const char* filename);

// Instruction wrappers
void _halt(void);
void _lda(uint8_t offset, uint64_t addr, uint8_t width);
void _sta(uint64_t addr, uint8_t offset, uint8_t width);
void _swp(void);
void _jmp(uint64_t addr);
void _jmpbz(uint64_t addr);
void _addab(void);
void _subab(void);
void _ldab(uint8_t offset, uint8_t width);
void _stab(uint8_t offset, uint8_t width);
void _cmpab(void);
void _void(uint64_t _data);
void _ldpca(void);

#endif
