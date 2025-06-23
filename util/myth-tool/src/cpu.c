
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "pulley.h"
#include "cpu.h"


/*
** Minimal C program as reference on how Myth CPU is intended to work.
** Use in conjunction with Verilog file and KiCad schematics.
** Project files see: https://github.com/michaelmangelsdorf/myth
** Author: 2025 Michael Mangelsdorf <mim@ok-schalter.de>
*/

/* Memory */

uint8_t ram[256*256]  = {0}; /* Organised as [PAGE-INDEX][BYTE-OFFSET] */

/* Interrupts */

uint8_t irq;   /* Interrupt Request Flag (input) */
uint8_t busy;  /* BUSY / Interrupts-Disabled flag (output) */

/* IO Device Selection */

uint8_t e_old; /* Device Enable Register */
uint8_t e;     /* _old is for persisting edge information in VM file */

/* Serial Interface */

uint8_t sclk;  /* SPI Serial Clock signal (output)  */
uint8_t miso;  /* SPI Master-in Slave-out bit (input) */
uint8_t mosi;  /* SPI Master-out Slave-in bit (output) */
uint8_t sir;   /* Serial Input Register */
uint8_t sor;   /* Serial Output Register */

/* GPIO bus */

uint8_t pir;   /* Parallel Input Register */
uint8_t por;   /* Parallel Output Register */

/* Accumulator and ALU*/

uint8_t a;     /* Primary Result (ALU input operand A) */
uint8_t x;     /* Secondary Result (ALU input operand X) */

/* Execution Control */

uint8_t c;     /* CODE Program Current Page-Index Register */
uint8_t pc;    /* Program-Counter Current Page-Offset Register */

/* 16-bit Base Pointer */

uint8_t b;     /* Base Page-Index Register */
uint8_t o;     /* Base Page-Offset Register */

/* 16-bit Amenity Pointers - Transfer to/from Base Pointer (B:O) */

uint8_t p1b, p1o; /* Page-index, page-offset parts */
uint8_t p2b, p2o;
uint8_t p3b, p3o;
uint8_t p4b, p4o;

/* Amenity Page-Index */

uint8_t k;

/* Local Stack Frame-Pointer */

uint8_t l;     /* Local Page Index Register */

/* Hardware Loop Counter */

uint8_t d;     /* Down-Counter / Auto-Decrement Register */


/* Execute an opcode
 *
 */
void exec_opcode(uint8_t opcode)
{
        /*Decode priority encoded opcode
         and execute decoded instruction*/
        if      (opcode & 0x80) pair(   opcode);
        else if (opcode & 0x40) getput( opcode);
        else if (opcode & 0x20) trap(   opcode);
        else if (opcode & 0x10) alu(    opcode);
        else if (opcode & 0x08) bop(    opcode);
        else                    sys(    opcode);
}


/* Single-step 1 instruction cycle
*/
void
myth_step()
{

/*  Decoding Schema:  -- Opcode Bits --
                         MSB     LSB
    all 0: OPC_SYS       00000   xxx    b0-2: Index of SYS Instruction
     else: OPC_BOP       00001   xxx    b0-2: Index of BOP Instruction
           OPC_ALU       0001   xxxx    b0-3: Index of ALU Instruction
           OPC_TRAP      001   xxxxx    b0-4: DESTPAGE
           OPC_GETPUT    01 xx x xxx    b0-2: OFFS, b3: GET/PUT, b4-5: REG
           OPC_PAIR      1  xxx xxxx    b0-3: DST, b4-6: SRC
*/

        uint8_t opcode;

        /* Before each instruction, Myth checks if an interrupt is
           requested. Such requests are ignored unless the BUSY flag
           is disabled AND the program is not executing in page 0.
           This latter condition assures that the system is not
           forced to interrupt while initialising upon RESET.

           If an interrupt is not ignored, the way it is handled
           is by injecting a "fake" opcode: A trap subroutine call to
           a service handler at page 0.
           During RESET, the local-page index register L is reset to 0,
           and this setting can be used to distinguish a RESET from
           an interrupt (instruction LOCAL).

           Executing a TRAP0 call from your program is indistinguishable
           from accepting an interrupt.
           Execute RTI to disable the busy flag and unnest the call. */

        if (irq && c!=0 && !busy) opcode = 32; /* Inject TRAP0 */
        else opcode = fetch();

        exec_opcode(opcode);
}


/* Fetch next byte in CODE stream, then increment PC.
   Fetches either an instruction, or an instruction literal (Fx)
*/

uint8_t
fetch()
{
        return ram[c*256 + pc++];
}


/* Coroutine Call - Similar to a function call but does not
   create a stackframe and allows target offsets other than 0.
   Jump to address in pointer B:O, then save return address in B:O */

void
cor()
{
        uint8_t temp;

        temp = o;
        o = pc;     /* Save page offset of return instruction */
        pc = temp;  /* Set branch target offset */

        temp = b;
        b = c;      /* Save page index of return instruction */
        c = temp;   /* Set branch target page-index */
}

/* Subroutine call (xC or TRAP) */

void
call( uint8_t dstpage)
{
        o = pc;      /* Save page offset of return instruction */
        pc = 0;      /* Always branch to page head: offset 0*/

        b = c;       /* Save page-index of return instruction */
        c = dstpage;

        l--;         /*Create stack frame*/
}


void
trap( uint8_t opcode)
{
        uint8_t dstpage = opcode & 31; /* Keep low order 5 bits only */
        if (dstpage==0) busy = 1;      /* Unique side-effect of TRAP0 */
        call( dstpage);
}


/* First step of handling PAIR instructions,
   derives the source value
*/

uint8_t
srcval( uint8_t srcreg)
{
/* PAIR Instruction - Bits 4-6: SOURCE index
   'SRCx' notation: Transfer SRC into DESTINATION x
   EFFECT means: Transfer has side effects:
   e.g.: M: Read from M, transfers value at B:O in memory to X
*/

#define Fx 0 /* FETCH literal (EFFECT: Get code-stream literal, advance PC) */
#define Mx 1 /* MEMORY @ B:O (EFFECT load a byte from memory as source) */
#define Bx 2 /* BASE Register */
#define Ox 3 /* OFFSET Register */
#define Ax 4 /* ACCUMULATOR Register */
#define Dx 5 /* DECREMENT Register */
#define Sx 6 /* SERIAL input */
#define Px 7 /* PARALLEL input */

        switch(srcreg){
                case Fx: return fetch();
                case Mx: return ram[b*256 + o];
                case Bx: return b;
                case Ox: return o;
                case Ax: return a;
                case Dx: return d;
                case Sx: return sir;
                case Px: return pir;
                default: return 0;
        }
}


/* Second part of handling PAIR instructions,
   stores source value into destination
*/
void
pair( uint8_t opcode)
{
        if (scrounge( opcode)) return;

/* PAIR Instruction - Bits 0-3: DESTINATION index
   'xREG' notation: Transfer SOURCE x into DEST
   EFFECT means: Transfer has side effects:
   e.g.: CALL: Write into C, calls C:0, stores return pointer in B:O
*/

#define xC 0  /* CALL (EFFECT: subroutine call)*/
#define xM 1  /* MEMORY @ B:O (EFFECT: Store source into memory at B:O) */
#define xB 2  /* BASE Register */
#define xO 3  /* OFFSET Register */
#define xA 4  /* ACCUMULATOR Register (EFFECT: Saves old value of A in X) */
#define xD 5  /* DECREMENT Register */
#define xS 6  /* SERIAL output */
#define xP 7  /* PARALLEL output */
#define xE 8  /* ENABLE Register (EFFECT: Sets hardware select signals) */
#define xK 9  /* KEY Register */
#define xU 10 /* UPDATE (Effect: Adds signed byte to 16-bit pair B:O) */

/* The following are all EFFECT instructions (JUMPS) */

#define xW 11 /* WHILE D - Write into PC WHILE D is not zero, decrement D */
#define xJ 12 /* JUMP - Writes into pc */
#define xH 13 /* JUMP IF HOT (NOT ZERO) - Write into PC if A is not zero*/
#define xZ 14 /* JUMP IF ZERO - Write into PC if A is zero */
#define xN 15 /* JUMP IF NEGATIVE - Write into PC if A has bit 7 set */

        uint8_t src = (opcode >> 4) & 7; /* Zero except bits 4-6 at LSB */
        uint8_t dst = opcode & 15;       /* Zero except bits 0-3 at LSB */
        uint8_t v = srcval(src);
        uint16_t ui16;

        switch(dst){
                case xC: call(v);           break;
                case xM: ram[b*256 + o] = v;     break;
                case xB: b = v;             break;
                case xO: o = v;             break;
                case xA: push_acc(v);       break;
                case xD: d = v;             break;
                case xS: sor = v;           break;
                case xP: por = v;           break;
                case xE: e_old = e;
                         e = v;             
                         // My-Tool specific!
                         switch(e) {
                                default: emit_pulley_block(stdout);
                         }
                         break;
                case xK: b = k; o = v;      break;

                 /* Add signed 8-bit v to 16-bit B:O */

                case xU: ui16 = (o+v)>127? /* Sign extend to 16-bit */
                                 (uint16_t)v | 0xFF00 : v;
                         o = ui16 & 0xFF;
                         b = ui16 >> 8;
                         break;

                case xW: if (d) pc = v;
                         d--; /*Post decrement, either case!*/
                         break;
                case xJ: pc = v;            break;
                case xH: if (a) pc = v;     break;
                case xZ: if (!a) pc = v;    break;
                case xN: if (a&128) pc = v; break;
        }
}


uint8_t
scrounge( uint8_t opcode)
{

/*Scrounged Pairs - Repurpose/remap "diagonal" NOP instructions */

#define KEY   16*Fx + xM /* Remaps FM instruction to KEY */
#define CODE  16*Mx + xM /* and so forth */
#define LOCAL 16*Bx + xB
#define LEAVE 16*Ox + xO
#define ENTER 16*Ax + xA
#define INC   16*Dx + xD
#define DEC   16*Sx + xS
#define EA    16*Px + xP

        switch(opcode & 0x7F){
                case KEY:      k=b;                return KEY;
                case CODE:     b=c; o=pc;          return CODE;
                case LOCAL:    b=l; o=0xF7; /*L0*/ return LOCAL;
                case LEAVE:    l++;                return LEAVE;
                case ENTER:    l--;                return ENTER;
                case INC:      a++;                return INC;
                case DEC:      a--;                return DEC;
                case EA:       a=e;            return EA;
                default: return 0;
        }
}


void push_acc(uint8_t v){
        x = a;
        a = v;
}


/* Execute GETPUT instruction
*/

void
getput( uint8_t opcode)
{
#define GETPUT_OFFSET 0xF8 /*Local-page offset*/

        /* Opcode BIT 3 encodes GET/PUT mode */
        #define BIT3 8

        /* Opcode BITS 4-5 encode register index (BOAD) */
        #define BITS45 (opcode >> 4) & 3
        
        /* Opcode BITS 0-2 encode address offset in local page (from F8h) */
        #define BITS02 opcode & 7

        uint8_t index = BITS02;
        uint8_t *mptr = &( ram[l*256 + GETPUT_OFFSET + index] );

        if(opcode & BIT3)
                switch(BITS45){
                        case 0: *mptr = b; break;
                        case 1: *mptr = o; break;
                        case 2: *mptr = a; break;
                        case 3: *mptr = d; break;
                }
        else
        switch(BITS45){
                case 0: b = *mptr;       break;
                case 1: o = *mptr;       break;
                case 2: push_acc(*mptr); break;
                case 3: d = *mptr;       break;
        }
}


/* Execute ALU instruction
*/
void
alu( uint8_t opcode)
{

/*ALU Instructions */

#define NOT   0 /* Set A to one's complement of A , X unchanged */
#define ALX   1 /* Flag (A<X) in A (255 if true, 0 if false), X unchanged */
#define AEX   2 /* Flag (A==X) in A (255 if true, 0 if false), X unchanged */
#define AGX   3 /* Flag (A>X) in A (255 if true, 0 if false), X unchanged */
#define AND   4 /* Set A to (A AND X), X unchanged */
#define IOR   5 /* Set A to (A OR X), X unchanged */
#define EOR   6 /* Set A to (A XOR X), X unchanged */
#define XA    7 /* Set A equal to X, X unchanged */
#define AX    8 /* Set X equal to A */
#define SWAP  9 /* Swap A and X */
#define SHL  10 /* Shift A left, result in A, set X to previous MSB of A as LSB (0 or 1) */
#define SHR  11 /* Shift A right logically, result in A, set X to previous LSB of A as MSB (0 or 80h) */
#define ASR  12 /* Shift A right arithmetically, set X to previous LSB of A as MSB (0 or 80h) */
#define ADDC 13 /* Add A to X, result in A, CARRY bit in X (0 or 1) */
#define ADDV 14 /* Add A to X, result in A, OVERFLOW flag in X (255 if OVF, else 0) */
#define SUBB 15 /* Subtract A from X, result in A, BORROW bit in X (0 or 1) */

        uint8_t a0 = a, x0 = x;

        /* Compute signed addition overflow flag */
        int i = x + a;
        uint8_t ovf = ((a&0x80)^(x&0x80))==0    /* Addends have same sign */
                        && ((i&0x80)^(a&0x80)); /* But result has different sign */
        
        switch(opcode & 15){
                case NOT: a = ~a;                                       break;
                case ALX: a = (a<x)  ? 255:0;                           break;
                case AEX: a = (a==x) ? 255:0;                           break;
                case AGX: a = (a>x)  ? 255:0;                           break;
                case AND: a = a & x;                                    break;
                case IOR: a = a | x;                                    break;
                case EOR:  a = a ^ x;                                   break;
                case XA:   a=x;                                         break;
                case AX:   x=a;                                         break;
                case SWAP: a=x; x=a0;                                   break;
                case SHL:  a<<=1; x=(a0 & 0x80)? 1:0;                   break;
                case SHR:  a>>=1; x=(a0 & 1)? 0x80:0;                   break;
                case ASR:  a=(a>>1)+(a0 & 0x80); x=(a0 & 1)? 0x80:0;    break;
                case ADDC: a=i&0xFF; x=(i>255)? 1:0;                    break;
                case ADDV: a=i&0xFF; x=ovf?255:0;                       break;
                case SUBB: i = x - a;
                           a = x - a;
                           x = (i<0) ? 1 : 0;
                 break;
        }
}


/* Execute BOP instruction
*/
void
bop( uint8_t opcode)
{
                /*BOP Instructions */
#define P1BO 0 /* Store POINTER into B:O */
#define BOP1 1 /* Store B:O into POINTER */
#define P2BO 2 
#define BOP2 3
#define P3BO 4 
#define BOP3 5
#define P4BO 6 
#define BOP4 7

        switch(opcode & 7){
                case P1BO: b=p1b; o=p1o; break;
                case BOP1: p1b=b; p1o=o; break;

                case P2BO: b=p2b; o=p2o; break;
                case BOP2: p2b=b; p2o=o; break;
                
                case P3BO: b=p3b; o=p3o; break;
                case BOP3: p3b=b; p3o=o; break;
                
                case P4BO: b=p4b; o=p4o; break;
                case BOP4: p4b=b; p4o=o; break;
        }
}


/* Execute SYS instruction
*/
void
sys( uint8_t opcode)
{
                /*SYS Instructions */
#define NOP 0 /* No Operation */
#define SSI 1 /* Serial Shift In */
#define SSO 2 /* Serial Shift Out */
#define SCL 3 /* Set serial Clock Low */
#define SCH 4 /* Set serial Clock High */
#define RTS 5 /* Return from Subroutine */
#define RTI 6 /* Return from Interrupt */
#define COR 7 /* Coroutine: Jump to B:O, save return pointer in B:O */

        switch(opcode & 7){ /*Zero except low order 3 bits*/
                case NOP: break;
                case SSI:
                        /*Clocks in MISO line bit into LSB*/
                        sir = (sir<<1) + miso;
                        break;
                case SSO:
                        /*Clocks out MOSI MSB first*/
                        mosi = sor & 0x80 ? 1:0;
                        sor <<= 1;
                        break;
                case SCL: sclk = 0; break;
                case SCH: sclk = 1; break;
                case RTI: busy = 0;
                          /* Fall through! */
                case RTS:
                        c = b;
                        pc = o;
                        l++; /* Leave stack frame*/
                        break;
                case COR: cor(); break;
        }
}

