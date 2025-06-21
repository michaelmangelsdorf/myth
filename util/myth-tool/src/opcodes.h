
#ifndef OPCODES_H
#define OPCODES_H

typedef struct {
    int opcode;
    const char* group;
    const char* mnemonic;
    const char* desc;
    const char* pseudocode;
} Opcode;

#endif
