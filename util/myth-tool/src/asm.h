
#ifndef ASM_H
#define ASM_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>

#include "my.h"
#include "cpu.h"


#define MAX_LABELS    1000
#define MAX_LABEL_LEN 16
#define SYMTAB_OFFS   0x8000  // or wherever appropriate


typedef struct {
        uint8_t isglobal;
        char name[MAX_LABEL_LEN];
        uint16_t address;
        uint8_t data;
} LabelDef;


typedef struct {
        uint8_t bytes_emitted;
        uint16_t objcode_offset;
        char text[81];
} Line;

uint16_t final_offset;


typedef struct {
    int opcode;
    const char* group;
    const char* mnemonic;
    const char* desc;
    const char* pseudocode;
} Opcode;

extern Opcode opcodes[];



int handle_numbers(const char* token, int* out_value);
int handle_hex(const char* token, int* out_value);
int handle_dec(const char* token, int* out_value);
int handle_bin(const char* token, int* out_value);
void handle_labeldef(const char* label_raw, uint16_t* pc, uint8_t pass);
void handle_labelref(const char* label, char direction, uint16_t pc);
void strip_trailing_punctuation(char* token);
int find_opcode(const char* token, uint8_t* opcode);
void strip_comment(char* str);
int handle_singlechar(const char* token, int* value);
int handle_string_literal(const char* token, uint8_t* output_buffer, size_t* output_size);
void write_globals(void);
void handle_constdef(const char* token);
int lookup_constant(const char* token, uint8_t* out);
void remove_parentheses_comment(char* line);
extern void assemble(Line** line_ptr_array, size_t line_count);



#define COLOR_ADDR   "\033[34m"  // Blue
#define COLOR_CODE   "\033[32m"  // Green
#define COLOR_LINE   "\033[36m"  // Cyan
#define COLOR_RESET  "\033[0m"

extern void write_listing(FILE* out, Line** line_ptr_array, size_t line_count);

#endif




