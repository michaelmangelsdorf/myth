

#include "asm.h"
#include "binfile.h"

Opcode opcodes[] = {
    { 0, "SYS", "NOP", "Pass the turn (no operation)", "PC++" },
    { 1, "SYS", "SSI", "Shift serial bit in", "SIR=(SIR<<1)+MISO PC++" },
    { 2, "SYS", "SSO", "Shift serial bit out", "MOSI=(SOR&80h)?1:0 SOR<<=1 PC++" },
    { 3, "SYS", "SCL", "Set serial clock low", "SCLK=0 PC++" },
    { 4, "SYS", "SCH", "Set serial clock high", "SCLK=1 PC++" },
    { 5, "SYS", "RTS", "Return from interrupt", "BUSY=0 C=B PC=O L++" },
    { 6, "SYS", "RTI", "Return from subroutine", "C=B PC=O L++" },
    { 7, "SYS", "COR", "Set C to B. Set PC to O. Save return pointer into B:O", "B0=C C=B B=B0 O0=PC PC=O O=O0" },
    { 8, "BOP", "P1BO", "Copy pointer P1 into B:O", "BO=P1 PC++" },
    { 9, "BOP", "BOP1", "Copy B:O into pointer P1", "P1=BO PC++" },
    { 10, "BOP", "P2BO", "Copy pointer P2 into B:O", "BO=P2 PC++" },
    { 11, "BOP", "BOP2", "Copy B:O into pointer P2", "P2=BO PC++" },
    { 12, "BOP", "P3BO", "Copy pointer P3 into B:O", "BO=P3 PC++" },
    { 13, "BOP", "BOP3", "Copy B:O into pointer P3", "P3=BO PC++" },
    { 14, "BOP", "P4BO", "Copy pointer P4 into B:O", "BO=P4 PC++" },
    { 15, "BOP", "BOP4", "Copy B:O into pointer p4", "P4=BO PC++" },
    { 16, "ALU", "NOT", "Set A to one's complement of A , X unchanged", "A=NOT(A) PC++" },
    { 17, "ALU", "ALX", "Flag (A<X) in A (255 if true, 0 if false), X unchanged", "A=(A<X)?255:0 PC++" },
    { 18, "ALU", "AEX", "Flag (A==X) in A (255 if true, 0 if false), X unchanged", "A=(A==X)?255:0 PC++" },
    { 19, "ALU", "AGX", "Flag (A>X) in A (255 if true, 0 if false), X unchanged", "A=(A>X)?255:0 PC++" },
    { 20, "ALU", "AND", "Set A to (A AND X), X unchanged", "A=(A&X) PC++" },
    { 21, "ALU", "IOR", "Set A to (A OR X), X unchanged", "A=(A|X) PC++" },
    { 22, "ALU", "EOR", "Set A to (A XOR X), X unchanged", "A=(A^X) PC++" },
    { 23, "ALU", "XA", "Set A equal to X, X unchanged", "A=X PC++" },
    { 24, "ALU", "AX", "Set X equal to A", "X=A PC++" },
    { 25, "ALU", "SWAP", "Swap A and X", "X0=A A=X X=X0 PC++" },
    { 26, "ALU", "SHL", "Shift A left, result in A, set X to previous MSB of A as LSB (0 or 1)", "A0=A A=(A<<1) X=(A0&80h)?1:0 PC++" },
    { 27, "ALU", "SHR", "Shift A right logically, result in A, set X to previous LSB of A as MSB (0 or 80h)", "A0=A A=(X>>1) X=(A0&1h)?80h:0 PC++" },
    { 28, "ALU", "ASR", "Shift A right arithmetically, set X to previous LSB of A as MSB (0 or 80h)", "A0=A A=(X>>1)+(A0&80h) X=(A0&1h)?80h:0 PC++" },
    { 29, "ALU", "ADDC", "Add A to X, result in A, CARRY bit in X (0 or 1)", "A0=A A=X+A X=CARRY-X-plus-A0 PC++" },
    { 30, "ALU", "ADDV", "Add A to X, result in A, OVERFLOW flag in X (255 if OVF, else 0)", "A0=A A=X+A X=(OVERFLOW-A0+X)?255:0 PC++" },
    { 31, "ALU", "SUBB", "Subtract A from X, result in A, BORROW bit in X (0 or 1)", "A0=A A=X-A X=BORROW-X-minus-A0 PC++" },
    { 32, "TRAP", "*0", "Trap call to page 0, offset 0 - Set BUSY flag", "BUSY=1 B0=C C=0 B=B0 O=PC PC=0" },
    { 33, "TRAP", "*1", "Trap call to page 1, offset 0", "B0=C C=1 B=B0 O=PC PC=0" },
    { 34, "TRAP", "*2", "Trap call to page 2, offset 0", "B0=C C=2 B=B0 O=PC PC=0" },
    { 35, "TRAP", "*3", "Trap call to page 3, offset 0", "B0=C C=3 B=B0 O=PC PC=0" },
    { 36, "TRAP", "*4", "Trap call to page 4, offset 0", "B0=C C=4 B=B0 O=PC PC=0" },
    { 37, "TRAP", "*5", "Trap call to page 5, offset 0", "B0=C C=5 B=B0 O=PC PC=0" },
    { 38, "TRAP", "*6", "Trap call to page 6, offset 0", "B0=C C=6 B=B0 O=PC PC=0" },
    { 39, "TRAP", "*7", "Trap call to page 7, offset 0", "B0=C C=7 B=B0 O=PC PC=0" },
    { 40, "TRAP", "*8", "Trap call to page 8, offset 0", "B0=C C=8 B=B0 O=PC PC=0" },
    { 41, "TRAP", "*9", "Trap call to page 9, offset 0", "B0=C C=9 B=B0 O=PC PC=0" },
    { 42, "TRAP", "*10", "Trap call to page 10, offset 0", "B0=C C=10 B=B0 O=PC PC=0" },
    { 43, "TRAP", "*11", "Trap call to page 11, offset 0", "B0=C C=11 B=B0 O=PC PC=0" },
    { 44, "TRAP", "*12", "Trap call to page 12, offset 0", "B0=C C=12 B=B0 O=PC PC=0" },
    { 45, "TRAP", "*13", "Trap call to page 13, offset 0", "B0=C C=13 B=B0 O=PC PC=0" },
    { 46, "TRAP", "*14", "Trap call to page 14, offset 0", "B0=C C=14 B=B0 O=PC PC=0" },
    { 47, "TRAP", "*15", "Trap call to page 15, offset 0", "B0=C C=15 B=B0 O=PC PC=0" },
    { 48, "TRAP", "*16", "Trap call to page 16, offset 0", "B0=C C=16 B=B0 O=PC PC=0" },
    { 49, "TRAP", "*17", "Trap call to page 17, offset 0", "B0=C C=17 B=B0 O=PC PC=0" },
    { 50, "TRAP", "*18", "Trap call to page 18, offset 0", "B0=C C=18 B=B0 O=PC PC=0" },
    { 51, "TRAP", "*19", "Trap call to page 19, offset 0", "B0=C C=19 B=B0 O=PC PC=0" },
    { 52, "TRAP", "*20", "Trap call to page 20, offset 0", "B0=C C=20 B=B0 O=PC PC=0" },
    { 53, "TRAP", "*21", "Trap call to page 21, offset 0", "B0=C C=21 B=B0 O=PC PC=0" },
    { 54, "TRAP", "*22", "Trap call to page 22, offset 0", "B0=C C=22 B=B0 O=PC PC=0" },
    { 55, "TRAP", "*23", "Trap call to page 23, offset 0", "B0=C C=23 B=B0 O=PC PC=0" },
    { 56, "TRAP", "*24", "Trap call to page 24, offset 0", "B0=C C=24 B=B0 O=PC PC=0" },
    { 57, "TRAP", "*25", "Trap call to page 25, offset 0", "B0=C C=25 B=B0 O=PC PC=0" },
    { 58, "TRAP", "*26", "Trap call to page 26, offset 0", "B0=C C=26 B=B0 O=PC PC=0" },
    { 59, "TRAP", "*27", "Trap call to page 27, offset 0", "B0=C C=27 B=B0 O=PC PC=0" },
    { 60, "TRAP", "*28", "Trap call to page 28, offset 0", "B0=C C=28 B=B0 O=PC PC=0" },
    { 61, "TRAP", "*29", "Trap call to page 29, offset 0", "B0=C C=29 B=B0 O=PC PC=0" },
    { 62, "TRAP", "*30", "Trap call to page 30, offset 0", "B0=C C=30 B=B0 O=PC PC=0" },
    { 63, "TRAP", "*31", "Trap call to page 31, offset 0", "B0=C C=31 B=B0 O=PC PC=0" },
    { 64, "GETPUT", "1b", "Load B from L1 (M[L:F8h])", "b=M[L:F8h] PC++" },
    { 65, "GETPUT", "2b", "Load B from L2 (M[L:F9h])", "b=M[L:F9h] PC++" },
    { 66, "GETPUT", "3b", "Load B from L3 (M[L:FAh])", "b=M[L:FAh] PC++" },
    { 67, "GETPUT", "4b", "Load B from L4 (M[L:FBh])", "b=M[L:FBh] PC++" },
    { 68, "GETPUT", "5b", "Load B from L5 (M[L:FCh])", "b=M[L:FCh] PC++" },
    { 69, "GETPUT", "6b", "Load B from L6 (M[L:FDh])", "b=M[L:FDh] PC++" },
    { 70, "GETPUT", "7b", "Load B from L7 (M[L:FEh])", "b=M[L:FEh] PC++" },
    { 71, "GETPUT", "8b", "Load B from L8 (M[L:FFh])", "b=M[L:FFh] PC++" },
    { 72, "GETPUT", "b1", "Store B into L1 (M[L:F8h])", "M[L:F8h]=b PC++" },
    { 73, "GETPUT", "b2", "Store B into L2 (M[L:F9h])", "M[L:F9h]=b PC++" },
    { 74, "GETPUT", "b3", "Store B into L3 (M[L:FAh])", "M[L:FAh]=b PC++" },
    { 75, "GETPUT", "b4", "Store B into L4 (M[L:FBh])", "M[L:FBh]=b PC++" },
    { 76, "GETPUT", "b5", "Store B into L5 (M[L:FCh])", "M[L:FCh]=b PC++" },
    { 77, "GETPUT", "b6", "Store B into L6 (M[L:FDh])", "M[L:FDh]=b PC++" },
    { 78, "GETPUT", "b7", "Store B into L7 (M[L:FEh])", "M[L:FEh]=b PC++" },
    { 79, "GETPUT", "b8", "Store B into L8 (M[L:FFh])", "M[L:FFh]=b PC++" },
    { 80, "GETPUT", "1o", "Load O from L1 (M[L:F8h])", "o=M[L:F8h] PC++" },
    { 81, "GETPUT", "2o", "Load O from L2 (M[L:F9h])", "o=M[L:F9h] PC++" },
    { 82, "GETPUT", "3o", "Load O from L3 (M[L:FAh])", "o=M[L:FAh] PC++" },
    { 83, "GETPUT", "4o", "Load O from L4 (M[L:FBh])", "o=M[L:FBh] PC++" },
    { 84, "GETPUT", "5o", "Load O from L5 (M[L:FCh])", "o=M[L:FCh] PC++" },
    { 85, "GETPUT", "6o", "Load O from L6 (M[L:FDh])", "o=M[L:FDh] PC++" },
    { 86, "GETPUT", "7o", "Load O from L7 (M[L:FEh])", "o=M[L:FEh] PC++" },
    { 87, "GETPUT", "8o", "Load O from L8 (M[L:FFh])", "o=M[L:FFh] PC++" },
    { 88, "GETPUT", "o1", "Store O into L1 (M[L:F8h])", "M[L:F8h]=o PC++" },
    { 89, "GETPUT", "o2", "Store O into L2 (M[L:F9h])", "M[L:F9h]=o PC++" },
    { 90, "GETPUT", "o3", "Store O into L3 (M[L:FAh])", "M[L:FAh]=o PC++" },
    { 91, "GETPUT", "o4", "Store O into L4 (M[L:FBh])", "M[L:FBh]=o PC++" },
    { 92, "GETPUT", "o5", "Store O into L5 (M[L:FCh])", "M[L:FCh]=o PC++" },
    { 93, "GETPUT", "o6", "Store O into L6 (M[L:FDh])", "M[L:FDh]=o PC++" },
    { 94, "GETPUT", "o7", "Store O into L7 (M[L:FEh])", "M[L:FEh]=o PC++" },
    { 95, "GETPUT", "o8", "Store O into L8 (M[L:FFh])", "M[L:FFh]=o PC++" },
    { 96, "GETPUT", "1a", "Load A from L1 (M[L:F8h])", "a=M[L:F8h] PC++" },
    { 97, "GETPUT", "2a", "Load A from L2 (M[L:F9h])", "a=M[L:F9h] PC++" },
    { 98, "GETPUT", "3a", "Load A from L3 (M[L:FAh])", "a=M[L:FAh] PC++" },
    { 99, "GETPUT", "4a", "Load A from L4 (M[L:FBh])", "a=M[L:FBh] PC++" },
    { 100, "GETPUT", "5a", "Load A from L5 (M[L:FCh])", "a=M[L:FCh] PC++" },
    { 101, "GETPUT", "6a", "Load A from L6 (M[L:FDh])", "a=M[L:FDh] PC++" },
    { 102, "GETPUT", "7a", "Load A from L7 (M[L:FEh])", "a=M[L:FEh] PC++" },
    { 103, "GETPUT", "8a", "Load A from L8 (M[L:FFh])", "a=M[L:FFh] PC++" },
    { 104, "GETPUT", "a1", "Store A into L1 (M[L:F8h])", "M[L:F8h]=a PC++" },
    { 105, "GETPUT", "a2", "Store A into L2 (M[L:F9h])", "M[L:F9h]=a PC++" },
    { 106, "GETPUT", "a3", "Store A into L3 (M[L:FAh])", "M[L:FAh]=a PC++" },
    { 107, "GETPUT", "a4", "Store A into L4 (M[L:FBh])", "M[L:FBh]=a PC++" },
    { 108, "GETPUT", "a5", "Store A into L5 (M[L:FCh])", "M[L:FCh]=a PC++" },
    { 109, "GETPUT", "a6", "Store A into L6 (M[L:FDh])", "M[L:FDh]=a PC++" },
    { 110, "GETPUT", "a7", "Store A into L7 (M[L:FEh])", "M[L:FEh]=a PC++" },
    { 111, "GETPUT", "a8", "Store A into L8 (M[L:FFh])", "M[L:FFh]=a PC++" },
    { 112, "GETPUT", "1d", "Load D from L1 (M[L:F8h])", "d=M[L:F8h] PC++" },
    { 113, "GETPUT", "2d", "Load D from L2 (M[L:F9h])", "d=M[L:F9h] PC++" },
    { 114, "GETPUT", "3d", "Load D from L3 (M[L:FAh])", "d=M[L:FAh] PC++" },
    { 115, "GETPUT", "4d", "Load D from L4 (M[L:FBh])", "d=M[L:FBh] PC++" },
    { 116, "GETPUT", "5d", "Load D from L5 (M[L:FCh])", "d=M[L:FCh] PC++" },
    { 117, "GETPUT", "6d", "Load D from L6 (M[L:FDh])", "d=M[L:FDh] PC++" },
    { 118, "GETPUT", "7d", "Load D from L7 (M[L:FEh])", "d=M[L:FEh] PC++" },
    { 119, "GETPUT", "8d", "Load D from L8 (M[L:FFh])", "d=M[L:FFh] PC++" },
    { 120, "GETPUT", "d1", "Store D into L1 (M[L:F8h])", "M[L:F8h]=d PC++" },
    { 121, "GETPUT", "d2", "Store D into L2 (M[L:F9h])", "M[L:F9h]=d PC++" },
    { 122, "GETPUT", "d3", "Store D into L3 (M[L:FAh])", "M[L:FAh]=d PC++" },
    { 123, "GETPUT", "d4", "Store D into L4 (M[L:FBh])", "M[L:FBh]=d PC++" },
    { 124, "GETPUT", "d5", "Store D into L5 (M[L:FCh])", "M[L:FCh]=d PC++" },
    { 125, "GETPUT", "d6", "Store D into L6 (M[L:FDh])", "M[L:FDh]=d PC++" },
    { 126, "GETPUT", "d7", "Store D into L7 (M[L:FEh])", "M[L:FEh]=d PC++" },
    { 127, "GETPUT", "d8", "Store D into L8 (M[L:FFh])", "M[L:FFh]=d PC++" },
    { 128, "PAIR", "FC", "Take M[C:PC++] as page-index, load the index into C, set PC to 0. Save return pointer into B:O. Decrement L", "T=M[C:PC] PC++ B=C C=T O=PC PC=0 L--" },
    { 129, "PAIR", "KEY", "Copy register B into K (done instead of FM!)", "B=C O=PC PC++" },
    { 130, "PAIR", "FB", "Take M[C:PC++] into B", "T=M[C:PC] PC++ B=T PC++" },
    { 131, "PAIR", "FO", "Take M[C:PC++] into O", "T=M[C:PC] PC++ O=T PC++" },
    { 132, "PAIR", "FA", "Take M[C:PC++] into A", "T=M[C:PC] PC++ A=T PC++" },
    { 133, "PAIR", "FD", "Take M[C:PC++] into D", "T=M[C:PC] PC++ D=T PC++" },
    { 134, "PAIR", "FS", "Take M[C:PC++] into SOR", "T=M[C:PC] PC++ SOR=T PC++" },
    { 135, "PAIR", "FP", "Take M[C:PC++] into POR", "T=M[C:PC] PC++ POR=T PC++" },
    { 136, "PAIR", "FD", "Take M[C:PC++] into E, sets device enable signals", "T=M[C:PC] PC++ E=T PC++ (Device Enable!)" },
    { 137, "PAIR", "FK", "Take M[C:PC++] into O, load K into B", "T=M[C:PC] PC++ O=T B=K PC++" },
    { 138, "PAIR", "FU", "Take M[C:PC++] as 8-bit signed number and add it to 16-bit pointer B:O", "T=M[C:PC] PC++ (B:O)+=T(signed) PC++" },
    { 139, "PAIR", "FW", "Take M[C:PC++] as page offset and store it into PC - while register D is not zero. In either case, decrement D", "T=M[C:PC] PC++ PC=(D?T:PC+1) D--" },
    { 140, "PAIR", "FJ", "Take M[C:PC++] as page offset and store it into PC - always", "T=M[C:PC] PC++ PC=T" },
    { 141, "PAIR", "FH", "Take M[C:PC++] as page offset and store it into PC - if A is not equal to zero", "T=M[C:PC] PC++ PC=(A?T:PC+1)" },
    { 142, "PAIR", "FZ", "Take M[C:PC++] as page offset and store it into PC - if A is equal to zero", "T=M[C:PC] PC++ PC=(A?PC+1:T)" },
    { 143, "PAIR", "FN", "Take M[C:PC++] as page offset and store it into PC - if A is negative (has bit 7 set)", "T=M[C:PC] PC++ PC=(A&80h?T:PC+1)" },
    { 144, "PAIR", "MC", "Take M[B:O] as page-index, load the index into C, set PC to 0. Save return pointer into B:O. Decrement L", "T=M[B:O] B=C C=T O=PC PC=0 L--" },
    { 145, "PAIR", "CODE", "Copy pointer C:PC into B:O (done instead of MM!)", "B=L O=F7h PC++" },
    { 146, "PAIR", "MB", "Take M[B:O] into B", "T=M[B:O] B=T PC++" },
    { 147, "PAIR", "MO", "Take M[B:O] into O", "T=M[B:O] O=T PC++" },
    { 148, "PAIR", "MA", "Take M[B:O] into A", "T=M[B:O] A=T PC++" },
    { 149, "PAIR", "MD", "Take M[B:O] into D", "T=M[B:O] D=T PC++" },
    { 150, "PAIR", "MS", "Take M[B:O] into SOR", "T=M[B:O] SOR=T PC++" },
    { 151, "PAIR", "MP", "Take M[B:O] into POR", "T=M[B:O] POR=T PC++" },
    { 152, "PAIR", "MD", "Take M[B:O] into E, sets device enable signals", "T=M[B:O] E=T PC++ (Device Enable!)" },
    { 153, "PAIR", "MK", "Take M[B:O] into O, load K into B", "T=M[B:O] O=T B=K PC++" },
    { 154, "PAIR", "MU", "Take M[B:O] as 8-bit signed number and add it to 16-bit pointer B:O", "T=M[B:O] (B:O)+=T(signed) PC++" },
    { 155, "PAIR", "MW", "Take M[B:O] as page offset and store it into PC - while register D is not zero. In either case, decrement D", "T=M[B:O] PC=(D?T:PC+1) D--" },
    { 156, "PAIR", "MJ", "Take M[B:O] as page offset and store it into PC - always", "T=M[B:O] PC=T" },
    { 157, "PAIR", "MH", "Take M[B:O] as page offset and store it into PC - if A is not equal to zero", "T=M[B:O] PC=(A?T:PC+1)" },
    { 158, "PAIR", "MZ", "Take M[B:O] as page offset and store it into PC - if A is equal to zero", "T=M[B:O] PC=(A?PC+1:T)" },
    { 159, "PAIR", "MN", "Take M[B:O] as page offset and store it into PC - if A is negative (has bit 7 set)", "T=M[B:O] PC=(A&80h?T:PC+1)" },
    { 160, "PAIR", "BC", "Take B as page-index, load the index into C, set PC to 0. Save return pointer into B:O. Decrement L", "T=B B=C C=T O=PC PC=0 L--" },
    { 161, "PAIR", "BM", "Take B into M[B:O]", "T=B M[B:O]=T PC++" },
    { 162, "PAIR", "LOCAL", "Copy pointer L:F7h (L0) into B:O (done instead of BB!)", "L++ PC++" },
    { 163, "PAIR", "BO", "Take B into O", "T=B O=T PC++" },
    { 164, "PAIR", "BA", "Take B into A", "T=B A=T PC++" },
    { 165, "PAIR", "BD", "Take B into D", "T=B D=T PC++" },
    { 166, "PAIR", "BS", "Take B into SOR", "T=B SOR=T PC++" },
    { 167, "PAIR", "BP", "Take B into POR", "T=B POR=T PC++" },
    { 168, "PAIR", "BD", "Take B into E, sets device enable signals", "T=B E=T PC++ (Device Enable!)" },
    { 169, "PAIR", "BK", "Take B into O, load K into B", "T=B O=T B=K PC++" },
    { 170, "PAIR", "BU", "Take B as 8-bit signed number and add it to 16-bit pointer B:O", "T=B (B:O)+=T(signed) PC++" },
    { 171, "PAIR", "BW", "Take B as page offset and store it into PC - while register D is not zero. In either case, decrement D", "T=B PC=(D?T:PC+1) D--" },
    { 172, "PAIR", "BJ", "Take B as page offset and store it into PC - always", "T=B PC=T" },
    { 173, "PAIR", "BH", "Take B as page offset and store it into PC - if A is not equal to zero", "T=B PC=(A?T:PC+1)" },
    { 174, "PAIR", "BZ", "Take B as page offset and store it into PC - if A is equal to zero", "T=B PC=(A?PC+1:T)" },
    { 175, "PAIR", "BN", "Take B as page offset and store it into PC - if A is negative (has bit 7 set)", "T=B PC=(A&80h?T:PC+1)" },
    { 176, "PAIR", "OC", "Take O as page-index, load the index into C, set PC to 0. Save return pointer into B:O. Decrement L", "T=O B=C C=T O=PC PC=0 L--" },
    { 177, "PAIR", "OM", "Take O into M[B:O]", "T=O M[B:O]=T PC++" },
    { 178, "PAIR", "OB", "Take O into B", "T=O B=T PC++" },
    { 179, "PAIR", "LEAVE", "Increment L (done instead of OO!)", "L-- PC++" },
    { 180, "PAIR", "OA", "Take O into A", "T=O A=T PC++" },
    { 181, "PAIR", "OD", "Take O into D", "T=O D=T PC++" },
    { 182, "PAIR", "OS", "Take O into SOR", "T=O SOR=T PC++" },
    { 183, "PAIR", "OP", "Take O into POR", "T=O POR=T PC++" },
    { 184, "PAIR", "OD", "Take O into E, sets device enable signals", "T=O E=T PC++ (Device Enable!)" },
    { 185, "PAIR", "OK", "Take O into O, load K into B", "T=O O=T B=K PC++" },
    { 186, "PAIR", "OU", "Take O as 8-bit signed number and add it to 16-bit pointer B:O", "T=O (B:O)+=T(signed) PC++" },
    { 187, "PAIR", "OW", "Take O as page offset and store it into PC - while register D is not zero. In either case, decrement D", "T=O PC=(D?T:PC+1) D--" },
    { 188, "PAIR", "OJ", "Take O as page offset and store it into PC - always", "T=O PC=T" },
    { 189, "PAIR", "OH", "Take O as page offset and store it into PC - if A is not equal to zero", "T=O PC=(A?T:PC+1)" },
    { 190, "PAIR", "OZ", "Take O as page offset and store it into PC - if A is equal to zero", "T=O PC=(A?PC+1:T)" },
    { 191, "PAIR", "ON", "Take O as page offset and store it into PC - if A is negative (has bit 7 set)", "T=O PC=(A&80h?T:PC+1)" },
    { 192, "PAIR", "AC", "Take A as page-index, load the index into C, set PC to 0. Save return pointer into B:O. Decrement L", "T=A B=C C=T O=PC PC=0 L--" },
    { 193, "PAIR", "AM", "Take A into M[B:O]", "T=A M[B:O]=T PC++" },
    { 194, "PAIR", "AB", "Take A into B", "T=A B=T PC++" },
    { 195, "PAIR", "AO", "Take A into O", "T=A O=T PC++" },
    { 196, "PAIR", "ENTER", "Decrement L (done instead of AA!)", "A++ PC++" },
    { 197, "PAIR", "AD", "Take A into D", "T=A D=T PC++" },
    { 198, "PAIR", "AS", "Take A into SOR", "T=A SOR=T PC++" },
    { 199, "PAIR", "AP", "Take A into POR", "T=A POR=T PC++" },
    { 200, "PAIR", "AD", "Take A into E, sets device enable signals", "T=A E=T PC++ (Device Enable!)" },
    { 201, "PAIR", "AK", "Take A into O, load K into B", "T=A O=T B=K PC++" },
    { 202, "PAIR", "AU", "Take A as 8-bit signed number and add it to 16-bit pointer B:O", "T=A (B:O)+=T(signed) PC++" },
    { 203, "PAIR", "AW", "Take A as page offset and store it into PC - while register D is not zero. In either case, decrement D", "T=A PC=(D?T:PC+1) D--" },
    { 204, "PAIR", "AJ", "Take A as page offset and store it into PC - always", "T=A PC=T" },
    { 205, "PAIR", "AH", "Take A as page offset and store it into PC - if A is not equal to zero", "T=A PC=(A?T:PC+1)" },
    { 206, "PAIR", "AZ", "Take A as page offset and store it into PC - if A is equal to zero", "T=A PC=(A?PC+1:T)" },
    { 207, "PAIR", "AN", "Take A as page offset and store it into PC - if A is negative (has bit 7 set)", "T=A PC=(A&80h?T:PC+1)" },
    { 208, "PAIR", "DC", "Take D as page-index, load the index into C, set PC to 0. Save return pointer into B:O. Decrement L", "T=D B=C C=T O=PC PC=0 L--" },
    { 209, "PAIR", "DM", "Take D into M[B:O]", "T=D M[B:O]=T PC++" },
    { 210, "PAIR", "DB", "Take D into B", "T=D B=T PC++" },
    { 211, "PAIR", "DO", "Take D into O", "T=D O=T PC++" },
    { 212, "PAIR", "DA", "Take D into A", "T=D A=T PC++" },
    { 213, "PAIR", "INC", "Increment A (done instead of DD!)", "A-- PC++" },
    { 214, "PAIR", "DS", "Take D into SOR", "T=D SOR=T PC++" },
    { 215, "PAIR", "DP", "Take D into POR", "T=D POR=T PC++" },
    { 216, "PAIR", "DD", "Take D into E, sets device enable signals", "T=D E=T PC++ (Device Enable!)" },
    { 217, "PAIR", "DK", "Take D into O, load K into B", "T=D O=T B=K PC++" },
    { 218, "PAIR", "DU", "Take D as 8-bit signed number and add it to 16-bit pointer B:O", "T=D (B:O)+=T(signed) PC++" },
    { 219, "PAIR", "DW", "Take D as page offset and store it into PC - while register D is not zero. In either case, decrement D", "T=D PC=(D?T:PC+1) D--" },
    { 220, "PAIR", "DJ", "Take D as page offset and store it into PC - always", "T=D PC=T" },
    { 221, "PAIR", "DH", "Take D as page offset and store it into PC - if A is not equal to zero", "T=D PC=(A?T:PC+1)" },
    { 222, "PAIR", "DZ", "Take D as page offset and store it into PC - if A is equal to zero", "T=D PC=(A?PC+1:T)" },
    { 223, "PAIR", "DN", "Take D as page offset and store it into PC - if A is negative (has bit 7 set)", "T=D PC=(A&80h?T:PC+1)" },
    { 224, "PAIR", "SC", "Take SIR as page-index, load the index into C, set PC to 0. Save return pointer into B:O. Decrement L", "T=SIR B=C C=T O=PC PC=0 L--" },
    { 225, "PAIR", "SM", "Take SIR into M[B:O]", "T=SIR M[B:O]=T PC++" },
    { 226, "PAIR", "SB", "Take SIR into B", "T=SIR B=T PC++" },
    { 227, "PAIR", "SO", "Take SIR into O", "T=SIR O=T PC++" },
    { 228, "PAIR", "SA", "Take SIR into A", "T=SIR A=T PC++" },
    { 229, "PAIR", "SD", "Take SIR into D", "T=SIR D=T PC++" },
    { 230, "PAIR", "DEC", "Decrement A (done instead of SS!)", "A=E PC++" },
    { 231, "PAIR", "SP", "Take SIR into POR", "T=SIR POR=T PC++" },
    { 232, "PAIR", "SD", "Take SIR into E, sets device enable signals", "T=SIR E=T PC++ (Device Enable!)" },
    { 233, "PAIR", "SK", "Take SIR into O, load K into B", "T=SIR O=T B=K PC++" },
    { 234, "PAIR", "SU", "Take SIR as 8-bit signed number and add it to 16-bit pointer B:O", "T=SIR (B:O)+=T(signed) PC++" },
    { 235, "PAIR", "SW", "Take SIR as page offset and store it into PC - while register D is not zero. In either case, decrement D", "T=SIR PC=(D?T:PC+1) D--" },
    { 236, "PAIR", "SJ", "Take SIR as page offset and store it into PC - always", "T=SIR PC=T" },
    { 237, "PAIR", "SH", "Take SIR as page offset and store it into PC - if A is not equal to zero", "T=SIR PC=(A?T:PC+1)" },
    { 238, "PAIR", "SZ", "Take SIR as page offset and store it into PC - if A is equal to zero", "T=SIR PC=(A?PC+1:T)" },
    { 239, "PAIR", "SN", "Take SIR as page offset and store it into PC - if A is negative (has bit 7 set)", "T=SIR PC=(A&80h?T:PC+1)" },
    { 240, "PAIR", "PC", "Take PIR as page-index, load the index into C, set PC to 0. Save return pointer into B:O. Decrement L", "T=PIR B=C C=T O=PC PC=0 L--" },
    { 241, "PAIR", "PM", "Take PIR into M[B:O]", "T=PIR M[B:O]=T PC++" },
    { 242, "PAIR", "PB", "Take PIR into B", "T=PIR B=T PC++" },
    { 243, "PAIR", "PO", "Take PIR into O", "T=PIR O=T PC++" },
    { 244, "PAIR", "PA", "Take PIR into A", "T=PIR A=T PC++" },
    { 245, "PAIR", "PD", "Take PIR into D", "T=PIR D=T PC++" },
    { 246, "PAIR", "PS", "Take PIR into SOR", "T=PIR SOR=T PC++" },
    { 247, "PAIR", "EA", "Copy E to A (done instead of PP!)", "K=B PC++" },
    { 248, "PAIR", "PD", "Take PIR into E, sets device enable signals", "T=PIR E=T PC++ (Device Enable!)" },
    { 249, "PAIR", "PK", "Take PIR into O, load K into B", "T=PIR O=T B=K PC++" },
    { 250, "PAIR", "PU", "Take PIR as 8-bit signed number and add it to 16-bit pointer B:O", "T=PIR (B:O)+=T(signed) PC++" },
    { 251, "PAIR", "PW", "Take PIR as page offset and store it into PC - while register D is not zero. In either case, decrement D", "T=PIR PC=(D?T:PC+1) D--" },
    { 252, "PAIR", "PJ", "Take PIR as page offset and store it into PC - always", "T=PIR PC=T" },
    { 253, "PAIR", "PH", "Take PIR as page offset and store it into PC - if A is not equal to zero", "T=PIR PC=(A?T:PC+1)" },
    { 254, "PAIR", "PZ", "Take PIR as page offset and store it into PC - if A is equal to zero", "T=PIR PC=(A?PC+1:T)" },
    { 255, "PAIR", "PN", "Take PIR as page offset and store it into PC - if A is negative (has bit 7 set)", "T=PIR PC=(A&80h?T:PC+1)" }
};

const int opcode_count = sizeof(opcodes) / sizeof(opcodes[0]);




LabelDef label_table[MAX_LABELS];
size_t label_count;


int handle_numbers(const char* token, int* out_value)
{
        if (!token || !*token) return -1;

        size_t len = strlen(token);

        // Hexadecimal: suffix 'h' or 'H'
        if (len > 1 && (token[len - 1] == 'h' || token[len - 1] == 'H')) {
                return handle_hex(token, out_value);
        }

        // Binary: prefix 'b' or 'B'
        if (token[0] == 'b' || token[0] == 'B') {
                return handle_bin(token, out_value);
        }

        // Decimal fallback
        return handle_dec(token, out_value);
}

int handle_hex(const char* token, int* out_value)
{
        char buf[32];
        size_t len = strlen(token);

        if (len >= sizeof(buf)) return -1;

        strncpy(buf, token, len - 1);    // Strip 'h' suffix
        buf[len - 1] = '\0';

        for (size_t i = 0; i < strlen(buf); ++i) {
                if (!isxdigit((unsigned char)buf[i])) return -1;
        }

        *out_value = (int)strtol(buf, NULL, 16);
        return 0;
}

int handle_dec(const char* token, int* out_value)
{
        char* endptr;
        *out_value = (int)strtol(token, &endptr, 10);

        if (*endptr != '\0') return -1;
        return 0;
}

int handle_bin(const char* token, int* out_value)
{
        if (token[0] != 'b' && token[0] != 'B') return -1;

        int value = 0;

        for (const char* p = token + 1; *p; ++p) {
                if (*p == '_') continue;
                if (*p != '0' && *p != '1') return -1;
                value = (value << 1) | (*p - '0');
        }

        *out_value = value;
        return 0;
}

void handle_labeldef(const char* label_raw, uint16_t* pc, uint8_t pass)
{
        if (label_count >= MAX_LABELS) {
                fprintf(stderr, "Label table overflow: too many labels\n");
                return;
        }

        // Copy the label to work on a modifiable buffer
        char working_token[256];
        strncpy(working_token, label_raw, sizeof(working_token) - 1);
        working_token[sizeof(working_token) - 1] = '\0';

        // Check for trailing colon
        size_t len = strlen(working_token);
        uint8_t isglobal = 0;

        if (len > 0 && working_token[len - 1] == ':') {
                working_token[len - 1] = '\0';
                isglobal = 1;
        }

        // Parse number prefix before @
        int prefix_num = 0;
        const char* label = working_token;

        if (isdigit(label[0])) {
                char num_prefix[16];
                size_t num_len = 0;

                while (isdigit(label[num_len]) && num_len < sizeof(num_prefix) - 1) {
                        num_prefix[num_len] = label[num_len];
                        num_len++;
                }

                num_prefix[num_len] = '\0';
                prefix_num = atoi(num_prefix);

                if (label[num_len] != '@') {
                        fprintf(stderr, "Invalid label: missing '@' after number\n");
                        return;
                }

                label = label + num_len + 1;
        } else if (label[0] == '@') {
                label = label + 1;
        }

        if (strlen(label) > 16) {
                fprintf(stderr, "Label too long (max 16 chars): %s\n", label);
                return;
        }

        // Determine if all uppercase
        int is_upper = 1;
        for (size_t i = 0; i < strlen(label); ++i) {
                if (!isupper((unsigned char)label[i]) && !isdigit((unsigned char)label[i]) ) {
                        is_upper = 0;
                        break;
                }
        }

        // Rebase PC if a prefix number exists
        if (prefix_num > 0) {
                //uint16_t prev_pc = *pc;  // Save the old PC before changing

                if (is_upper) {
                        *pc = ((uint16_t)(prefix_num & 0xFF)) << 8;
                        
                } else {
                        *pc = (*pc & 0xFF00) | (prefix_num & 0xFF);
                }
                // Fill skipped bytes with zero if moving forward
                // if (*pc > prev_pc) {
                //         for (uint16_t addr = prev_pc; addr < *pc; addr++) {
                //                 ram[addr] = 0x00;
                //         }
                // }
        } else if (is_upper) {
                uint8_t pg = (*pc & 0xFF00) >> 8;
                *pc = (pg+1)*256;
        }

        // Allow redefinition in pass 1 only if the label is a single lowercase letter
        if (!(pass==2) && !(strlen(label) == 1 && islower((unsigned char)label[0]))) {
                for (size_t i = 0; i < label_count; ++i) {
                        if (strcmp(label_table[i].name, label) == 0) {
                                fprintf(stderr, "Duplicate label definition: %s\n", label);
                                return;
                        }
                }
        }

        // Add to label table
        strncpy(label_table[label_count].name, label, MAX_LABEL_LEN - 1);

        label_table[label_count].name[MAX_LABEL_LEN - 1] = '\0';
        label_table[label_count].address = is_upper ? *pc >> 8 : *pc;
        label_table[label_count].isglobal = isglobal;
        label_count++;

        //printf("Label defined: %s -> 0x%04X (%s)\n", label, *pc, isglobal ? "global" : "local");
}


void handle_labelref(const char* label, char direction, uint16_t pc)
{
        // if (pass == 1) {
        //         // In pass 1, we just note the reference but don't emit code yet
        //         printf("Pass 1: Deferring resolution of label reference %c%s at 0x%04X\n",
        //                direction, label, pc);
        //         return;
        // }

        int found = 0;
        uint16_t resolved_address = 0;

        if (direction == '<') {
                // Search backwards in label_table for matching label
                for (ssize_t i = (ssize_t)label_count - 1; i >= 0; --i) {
                        if (strcmp(label_table[i].name, label) == 0) {
                                resolved_address = label_table[i].address;
                                found = 1;
                                break;
                        }
                }
        }
        else if (direction == '>') {
                // Search forwards in label_table for matching label
                for (size_t i = 0; i < label_count; ++i) {
                        if (strcmp(label_table[i].name, label) == 0) {
                                resolved_address = label_table[i].address;
                                found = 1;
                                break;
                        }
                }
        }

        if (found) {
                // Write low byte of label address into RAM
                ram[pc] = (uint8_t)(resolved_address & 0xFF);
                //printf("Resolved %c%s to 0x%04X -> emitted 0x%02X at 0x%04X\n",
                //       direction, label, resolved_address, ram[pc], pc);
        } else {
                fprintf(stderr, "Error: Could not resolve label reference %c%s at 0x%04X\n",
                        direction, label, pc);
        }
}

void strip_trailing_punctuation(char* token)
{
        size_t len = strlen(token);
        while (len > 0 && (token[len - 1] == ',' || token[len - 1] == '.')) {
                token[--len] = '\0';
        }
}

// Helper function to convert a string to uppercase
void to_upper(const char* src, char* dest, size_t size) {
    if (!src || !dest || size == 0) {
        return; // Prevent segmentation faults due to NULL pointers or zero size
    }

    size_t i;
    for (i = 0; i < size - 1 && src[i] != '\0'; ++i) {
        dest[i] = (char)toupper((unsigned char)src[i]);
    }

    dest[i] = '\0'; // Always null-terminate
}


int find_opcode(const char* token, uint8_t* opcode)
{
    char token_upper[64];  // Adjust size as needed
    char mnemonic_upper[64];
    to_upper(token, token_upper, sizeof(token_upper));

    for (int i = 0; i < opcode_count; ++i) {
        if (opcodes[i].mnemonic) {
            to_upper(opcodes[i].mnemonic, mnemonic_upper, sizeof(mnemonic_upper));
            if (strcmp(mnemonic_upper, token_upper) == 0) {
                *opcode = (uint8_t)opcodes[i].opcode;
                return 0;
            }
        }
    }
    return -1;
}

void strip_comment(char* str)
{
        char* semi = strchr(str, ';');
        if (semi) {
                *semi = '\0';
        }
}


// Parses an escape sequence starting at token[pos] (where token[pos] == '\\')
// Returns the parsed char in *out_char, and the number of characters consumed in *consumed
// Returns 0 on success, -1 on failure
int parse_escape_sequence(const char* token, size_t pos, char* out_char, size_t* consumed) {
    *consumed = 0;
    if (token[pos] != '\\') {
        return -1; // Not an escape sequence
    }

    char c = token[pos + 1];
    if (c == '\0') {
        return -1; // Incomplete escape
    }

    switch (c) {
        case 'n': *out_char = '\n'; *consumed = 2; return 0;
        case 't': *out_char = '\t'; *consumed = 2; return 0;
        case 'r': *out_char = '\r'; *consumed = 2; return 0;
        case 'b': *out_char = '\b'; *consumed = 2; return 0;
        case 'f': *out_char = '\f'; *consumed = 2; return 0;
        case '\\': *out_char = '\\'; *consumed = 2; return 0;
        case '\'': *out_char = '\''; *consumed = 2; return 0;
        case '"': *out_char = '\"'; *consumed = 2; return 0;

        // Octal escape: \ooo (1 to 3 octal digits)
        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7': {
            int val = 0;
            size_t i = 1;
            while (i <= 3 && token[pos + i] >= '0' && token[pos + i] <= '7') {
                val = val * 8 + (token[pos + i] - '0');
                i++;
            }
            *out_char = (char)val;
            *consumed = i;
            return 0;
        }

        // Hex escape: \xHH (exactly two hex digits)
        case 'x': {
            if (isxdigit((unsigned char)token[pos + 2]) && isxdigit((unsigned char)token[pos + 3])) {
                char hex_str[3] = {token[pos + 2], token[pos + 3], '\0'};
                *out_char = (char)strtol(hex_str, NULL, 16);
                *consumed = 4;
                return 0;
            } else {
                return -1; // Invalid hex escape
            }
        }

        default:
            // Unknown escape, treat as literal char (e.g. \z -> z)
            *out_char = c;
            *consumed = 2;
            return 0;
    }
}

// Handles a single character literal, including escaped characters like '\n'
int handle_singlechar(const char* token, int* value) {
    size_t len = strlen(token);

    if (len < 3 || token[0] != '\'' || token[len - 1] != '\'') {
        return -1; // Not a valid character literal format
    }

    // Extract the content between the quotes
    size_t pos = 1;

    if (token[pos] == '\\') {
        char c;
        size_t consumed;
        if (parse_escape_sequence(token, pos, &c, &consumed) != 0) {
            fprintf(stderr, "Invalid escape sequence in character literal: %s\n", token);
            return -1;
        }
        // Make sure consumed characters fit inside the literal length (len-2 between quotes)
        if (consumed != len - 2) {
            fprintf(stderr, "Character literal too long or malformed: %s\n", token);
            return -1;
        }
        *value = (int)c;
        return 0;
    } else {
        // Regular single char
        if (len == 3) {
            *value = (int)token[pos];
            return 0;
        } else {
            fprintf(stderr, "Invalid character literal length: %s\n", token);
            return -1;
        }
    }
}

// Handles a string literal with escape sequences, outputting to output_buffer
int handle_string_literal(const char* token, uint8_t* output_buffer, size_t* output_size) {
    size_t len = strlen(token);

    if (len < 2 || token[0] != '"' || token[len - 1] != '"') {
        fprintf(stderr, "Invalid string literal (missing quotes): %s\n", token);
        return -1;
    }

    size_t j = 0; // output_buffer index
    for (size_t i = 1; i < len - 1; ) {
        if (token[i] == '\\') {
            char c;
            size_t consumed;
            if (parse_escape_sequence(token, i, &c, &consumed) != 0) {
                fprintf(stderr, "Invalid escape sequence in string literal: %s\n", token);
                return -1;
            }
            output_buffer[j++] = (uint8_t)c;
            i += consumed;
        } else {
            output_buffer[j++] = (uint8_t)token[i];
            i++;
        }
    }

    *output_size = j;
    return 0;
}



int add_symbol_entry(uint8_t typeid, const char* name, const uint8_t* data,
                     uint8_t itemcount, uint16_t* offset)
{
    if (typeid > 15) {
        fprintf(stderr, "Invalid typeid: %u\n", typeid);
        return -1;
    }

    size_t namelen = strlen(name);
    if (namelen > 15) {
        fprintf(stderr, "Symbol name too long: %s\n", name);
        return -1;
    }

    uint16_t start = *offset;
    uint16_t pos = *offset;

    // (a) Placeholder for offset to next entry
    uint16_t offset_pos = pos;
    ram[pos++] = 0x00;

    // (b) Type/length byte
    uint8_t typelen = ((typeid & 0x0F) << 4) | ((uint8_t)namelen & 0x0F);
    ram[pos++] = typelen;

    // (c) Symbol name (null-terminated)
    for (size_t i = 0; i < namelen; ++i) {
        ram[pos++] = (uint8_t)name[i];
    }
    ram[pos++] = 0x00;

    // (d) Data bytes
    for (uint8_t i = 0; i < itemcount; ++i) {
        ram[pos++] = data[i];
    }

    // (e) Compute and patch relative offset
    uint16_t entry_size = pos - start;
    if (entry_size > 255) {
        fprintf(stderr, "Symbol entry too large: %s (size %u bytes)\n", name, entry_size);
        return -1;
    }

    ram[offset_pos] = (uint8_t)entry_size;

    // Update offset pointer
    *offset = pos;
    return 0;
}



/* Create a symbol table inside ram[] with the mnemonics
 *
 */
void write_globals(void)
{
    uint16_t symaddr = SYMTAB_OFFS;


    // Create symbols for mnemonics
    for (int i = 0; i < 256; ++i) {
        const char* name = opcodes[i].mnemonic;

        if (!name || strlen(name) == 0) {
            continue; // skip empty mnemonics
        }

        uint8_t data[1] = { (uint8_t)opcodes[i].opcode };

        if (add_symbol_entry(2, name, data, 1, &symaddr) != 0) {
            fprintf(stderr, "Failed to write mnemonic symbol: %s\n", name);
        }
    }

    for (size_t i = 0; i < label_count; ++i) {
        if (!label_table[i].isglobal) {
            continue;
        }

        const char* name = label_table[i].name;

        // No extra data associated with label symbols in this case
        uint8_t empty_data[0] = {};

        // Type ID 0x1 = label symbol
        if (add_symbol_entry(0x1, name, empty_data, 0, &symaddr) != 0) {
            fprintf(stderr, "Failed to write global symbol: %s\n", name);
        }
    }

    // Mark end of symbol table
    ram[symaddr++] = 0x00;
}




void handle_constdef(const char* token)
{
        if (label_count >= MAX_LABELS) {
                fprintf(stderr, "Label table full\n");
                return;
        }

        const char* equal_sign = strchr(token, '=');
        if (!equal_sign) {
                fprintf(stderr, "Invalid constant definition: %s\n", token);
                return;
        }

        size_t name_len = equal_sign - token;
        if (name_len == 0 || name_len > 16) {
                fprintf(stderr, "Invalid constant name length\n");
                return;
        }

        char name[MAX_LABEL_LEN];
        strncpy(name, token, name_len);
        name[name_len] = '\0';

        // Check if name already exists
        for (size_t i = 0; i < label_count; ++i) {
                if (strcmp(label_table[i].name, name) == 0) {
                        fprintf(stderr, "Duplicate label or constant: %s\n", name);
                        return;
                }
        }

        // Parse numeric literal
        const char* value_token = equal_sign + 1;
        int value = 0;
        if (handle_numbers(value_token, &value) != 0) {
                fprintf(stderr, "Invalid constant value: %s\n", value_token);
                return;
        }

        // Add constant to label table (address set to 0)
        strncpy(label_table[label_count].name, name, MAX_LABEL_LEN - 1);
        label_table[label_count].name[MAX_LABEL_LEN - 1] = '\0';
        label_table[label_count].address = 0;
        label_table[label_count].data = (uint8_t)(value & 0xFF);
        label_table[label_count].isglobal = 0;
        label_count++;

        //printf("Constant defined: %s = 0x%02X\n", name, value & 0xFF);
}


int lookup_constant(const char* token, uint8_t* out)
{
        for (size_t i = 0; i < label_count; ++i) {
                if (strcmp(label_table[i].name, token) == 0) {
                        *out = label_table[i].data;
                        return 0;
                }
        }
        return -1;
}

void remove_parentheses_comment(char* line) {
    char* start = strchr(line, '(');  // Find the opening parenthesis
    while (start != NULL) {
        char* end = strchr(start, ')');  // Find the closing parenthesis
        if (end != NULL) {
            // Shift everything after the closing parenthesis to overwrite the comment
            memmove(start, end + 1, strlen(end));  // Remove the comment part
        }
        // Look for the next occurrence of a parenthesis
        start = strchr(line, '(');
    }
}

void emit(uint8_t byte, uint16_t* emit_at, uint16_t* bytes_emitted) {
    ram[(*emit_at)++] = byte;
    (*bytes_emitted)++;
}

void assemble(Line** line_ptr_array, size_t line_count)
{
    uint16_t emit_at;
    static int pass;

    for (pass = 1; pass <= 2; ++pass) {
        emit_at = 0;

        for (size_t i = 0; i < line_count; ++i) {
            Line* line = line_ptr_array[i];
            uint16_t bytes_emitted = 0;
            line->objcode_offset = emit_at;

            char line_copy[256];
            strncpy(line_copy, line->text, sizeof(line_copy) - 1);
            line_copy[sizeof(line_copy) - 1] = '\0';

            strip_comment(line_copy);
            remove_parentheses_comment(line_copy);

            if (line_copy[0] == '\0') continue;

            char* token = strtok(line_copy, " \t,");
            while (token != NULL) {
                strip_trailing_punctuation(token);

                if (strcmp(token, "-") == 0) {
                    token = strtok(NULL, " \t,");
                    continue;
                }

                if (strcmp(token, "PAGE") == 0) {
                    uint8_t page_value = (uint8_t)(emit_at >> 8);
                    if (pass == 1) {
                        line->objcode_offset = emit_at;
                    }
                    emit(page_value, &emit_at, &bytes_emitted);
                    token = strtok(NULL, " \t,");
                    continue;
                }

                if (strcmp(token, "OFFSET") == 0) {
                    uint8_t offset_value = (uint8_t)(emit_at & 0xFF);
                    if (pass == 1) {
                        line->objcode_offset = emit_at;
                    }
                    emit(offset_value, &emit_at, &bytes_emitted);
                    token = strtok(NULL, " \t,");
                    continue;
                }

                if (strchr(token, '=') != NULL) {
                    if (pass == 1) {
                        handle_constdef(token);
                    }
                    token = strtok(NULL, " \t,");
                    continue;
                }

                char* at_sign = strchr(token, '@');
                if (at_sign != NULL) {
                    int valid_prefix = 1;
                    for (char* p = token; p < at_sign; ++p) {
                        if (!isdigit((unsigned char)*p)) {
                            valid_prefix = 0;
                            break;
                        }
                    }

                    if (valid_prefix) {
                        handle_labeldef(token, &emit_at, pass);
                        token = strtok(NULL, " \t,");
                        continue;
                    }
                }

                if (token[0] == '<' || token[0] == '>') {
                    if (pass == 1) {
                        emit(0x00, &emit_at, &bytes_emitted); // Reserve space
                    } else {
                        handle_labelref(token + 1, token[0], emit_at++);
                    }
                    token = strtok(NULL, " \t,");
                    continue;
                }

                uint8_t opcode = 0;

                if (find_opcode(token, &opcode) == 0) {
                    if (pass == 1) {
                        line->objcode_offset = emit_at;
                    }
                    emit(opcode, &emit_at, &bytes_emitted);
                } else {
                    int value = 0;

                    if (handle_singlechar(token, &value) == 0) {
                        if (pass == 1) {
                            line->objcode_offset = emit_at;
                        }
                        emit((uint8_t)(value & 0xFF), &emit_at, &bytes_emitted);
                    }
                    else if (token[0] == '"') {
                        uint8_t string_buffer[256];
                        size_t string_len = 0;

                        if (handle_string_literal(token, string_buffer, &string_len) == 0) {
                            if (pass == 1) {
                                line->objcode_offset = emit_at;
                            }
                            for (size_t j = 0; j < string_len; ++j) {
                                emit(string_buffer[j], &emit_at, &bytes_emitted);
                            }
                        }
                    }
                    else if (handle_numbers(token, &value) == 0) {
                        if (pass == 1) {
                            line->objcode_offset = emit_at;
                        }
                        emit((uint8_t)(value & 0xFF), &emit_at, &bytes_emitted);
                    }
                    else {
                        uint8_t const_val = 0;
                        if (lookup_constant(token, &const_val) == 0) {
                            if (pass == 1) {
                                line->objcode_offset = emit_at;
                            }
                            emit(const_val, &emit_at, &bytes_emitted);
                            token = strtok(NULL, " \t,");
                            continue;
                        }

                        if (pass == 2) {
                            fprintf(stderr, "Unrecognized token on line %zu: %s\n", i + 1, token);
                        }
                        if (pass == 1) {
                            line->objcode_offset = 0xFFFF;
                        }
                    }
                }

                token = strtok(NULL, " \t,");
            }

            line->bytes_emitted = bytes_emitted;
            final_offset = emit_at;
        }
    }

    write_globals();
}




void
write_listing(FILE* out, Line** line_ptr_array, size_t line_count)
{
    int use_color = (out == stdout);
    uint16_t len, pos;

    // Print aligned header
    fprintf(out, "ADDR:  OBJCODE:                  LIN:  SOURCE:\n");

    for(size_t i = 0; i < line_count; i++){

        Line* line = line_ptr_array[i];
        uint16_t acurr = line->objcode_offset;

        len = line->bytes_emitted;

        if(len == 0){
            fprintf(out, "                              ");
            if (use_color) fprintf(out, COLOR_LINE);
            fprintf(out, "   %04zu  ", i + 1);
            if (use_color) fprintf(out, COLOR_RESET);
            fprintf(out, "%s\n", line->text);
            continue;
        }

        pos = acurr;
        while(len > 0){
            uint8_t chunk_len = len > 8 ? 8 : len;
            if (use_color) fprintf(out, COLOR_ADDR);
            fprintf(out, "%04X:  ", pos);
            if (use_color) fprintf(out, COLOR_RESET);

            if (use_color) fprintf(out, COLOR_CODE);
            for (int j = 0; j < 8; ++j) {
                if (j < chunk_len) {
                    fprintf(out, "%02X", ram[pos + j]);
                } else {
                    fprintf(out, "  ");
                }
                if (j < 7) fprintf(out, " ");
            }
            if (use_color) fprintf(out, COLOR_RESET);

            if (use_color) fprintf(out, COLOR_LINE);
            fprintf(out, "   %04zu  ", i + 1);
            if (use_color) fprintf(out, COLOR_RESET);

            fprintf(out, "%s", line->text);
            fprintf(out, "\n");

            pos += chunk_len;
            len -= chunk_len;
        }
    }
}






