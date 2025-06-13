
#include <stdio.h>
#include <stdlib.h>


/*
** C program as reference on how Myth CPU is supposed to work.
** Schematics see https://github.com/michaelmangelsdorf/myth
** Author: Michael Mangelsdorf <mim@ok-schalter.de>
**/


uint8_t ram[256][256];

uint8_t e_old; /* LO Nybble 0: NOP, 1: POE, HI Nybble: 0: NOP, 1: PLE */
uint8_t e_new;

uint8_t irq; /* Interrupt Request flag (input) */
uint8_t busy; /* Interrupts-Disabled flag (output) */

uint8_t sclk; /* Serial Clock (output) */
uint8_t miso; /* SPI Master-in Slave-out */
uint8_t mosi; /* SPI Master-out Slave-in */
uint8_t sir; /* Serial Input Register */
uint8_t sor; /* Serial Output Register */

uint8_t pir; /* Parallel Input Register */
uint8_t por; /* Parallel Output Register */

uint8_t a; /* Accumulator Register */
uint8_t x; /* ALU 2nd Operand */

uint8_t o; /* Offset Register */
uint8_t d; /* Down-Counter / Decrement Register */
uint8_t pc; /* Program Counter Register */

uint8_t c; /* Code-Page Index register */
uint8_t r; /* Resident-Page Index register */
uint8_t b; /* Base Page-Index Register */
uint8_t l; /* Local-Page Index Register */

uint8_t rp_b; /* Pointer Page-Index */
uint8_t ip_b;
uint8_t tp_b;
uint8_t sp_b;

uint8_t rp_o; /* Pointer Offset */
uint8_t ip_o;
uint8_t tp_o;
uint8_t sp_o;


void myth_step();
static uint8_t fetch_opcode();
static uint8_t srcval( uint8_t srcreg);
static void scrounge( uint8_t opcode);
static void pair( uint8_t opcode);
static void getput( uint8_t opcode);
static void trap( uint8_t opcode);
static void alu( uint8_t opcode);
static void bop( uint8_t opcode);
static void sys( uint8_t opcode);
static void call( uint8_t dstpage);



/* PAIR Sources
   The 'REGx' notation means: REG into (something)
   i.e. REG is a source
*/
#define Fx 0 /*from FETCH literal*/
#define Mx 1 /*from MEMORY @ B:O*/
#define Bx 2 /*from BASE register*/
#define Ox 3 /*from OFFSET register*/
#define Ax 4 /*from ACCUMULATOR register*/
#define Ex 5 /*from ENABLE register*/
#define Sx 6 /*from SERIAL input*/
#define Px 7 /*from PARALLEL input*/


/* PAIR Destinations
   The 'xREG' notation means: (something) into REG
   i.e. REG is a destination
*/
#define xU 0  /* UPDATE - add signed byte into 16-bit pair B:O) */
#define xM 1  /* into MEMORY @ B:O */
#define xB 2  /* into BASE register */
#define xO 3  /* into OFFSET register */
#define xA 4  /* into ACCUMULATOR register */
#define xE 5  /* into ENABLE register */
#define xS 6  /* into SERIAL output */
#define xP 7  /* into PARALLEL output */

#define xD 8  /* into DOWN-COUNTER register */
#define xW 9  /* JUMP WHILE D - write into PC WHILE D not zero, decrement D */
#define xJ 10 /* JUMP - write into pc */
#define xH 11 /* JUMP IF HOT (NOT ZERO) - write into PC if D not zero*/
#define xZ 12 /* JUMP IF ZERO - write into PC if A zero */
#define xN 13 /* JUMP IF NEGATIVE - write into PC if A has bit 7 set */
#define xR 14 /* JUMP to RESIDENT segment - write into R, set PC to 80h */
#define xC 15 /* CALL - write into C, call C:0, store return pointer in B:O */


/*Scrounged Pairs
*/
#define CODE  16*Fx+xM
#define LOCAL 16*Mx+xM
#define LEAVE 16*Bx+xB
#define ENTER 16*Ox+xO
#define INC   16*Ax+xA
#define DEC   16*Ex+xE


/*ALU Instructions
*/
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


/*SYS Instructions
*/
#define NOP 0 /* No Operation */
#define SSI 1 /* Serial Shift In */
#define SSO 2 /* Serial Shift Out */
#define SCL 3 /* Set serial Clock Low */
#define SCH 4 /* Set serial Clock High */
#define RTS 5 /* Return from Subroutine */
#define RTI 6 /* Return from Interrupt */
#define COR 7 /* Coroutine: Jump to B:O, save return pointer in B:O */


/*BOP Instructions
*/
#define GRP 0 /* Store "READ"-POINTER into B:O */
#define SRP 1 /* Store B:O into RP */
#define GIP 2 /* "INDEX" */
#define SIP 3
#define GTP 4 /* TP Threading Pointer */
#define STP 5
#define GSP 6 /* SP Stack Pointer */
#define SSP 7

#define GETPUT_OFFSET 0xF8 /*Local-page offset used by GETPUT instructions*/



/* Single-step 1 instruction cycle
*/
void
myth_step()
{
        uint8_t opcode = fetch_opcode();

        /*Decode priority encoded opcode
         and execute decoded instruction*/
        if (opcode & 0x80)      pair( opcode);
        else if (opcode & 0x40) getput( opcode);
        else if (opcode & 0x20) trap( opcode);
        else if (opcode & 0x10) alu( opcode);
        else if (opcode & 0x08) bop( opcode);
        else                    sys( opcode);
}


/* Fetch next byte in CODE stream, then increment PC.
   Fetches either an instruction, or an instruction literal (Nx)
*/
uint8_t
fetch_opcode()
{
        uint8_t pc0 = pc;
        if (irq && c>0 && !busy) {
                return 32; /* TRAP0 */
        } else return ram[ pc0 & 0x80 ? r:c ][pc++];
        /* Instruction byte page offset high: map "resident" routine */
}



void
cor()
{
        uint8_t temp;

        temp = o;
        o = pc; /* Save offset of return instruction */
        pc = temp;

        temp = b;
        b = c; /* Save page index of return instruction */
        c = temp;
        r = temp; /* ! */
}


void
call( uint8_t dstpage)
{
        o = pc; /* Save offset of return instruction */
        pc = 0; /*Branch to page head, offset 0*/

        b = c; /* Save page index of return instruction */
        c = dstpage;
        r = dstpage; /* ! */

        /*Create stack frame*/
        l--;
}


void
trap( uint8_t opcode)
{
        uint8_t dstpage = opcode & 31; /*Zero except low order 5 bits*/

         /* Don't move this line inside call() Only TRAP 0 sets BUSY! */
        if (dstpage==0) busy = 1;

       call( dstpage);
}


/* First part of handling PAIR instructions,
   derives the source value
*/
uint8_t
srcval( uint8_t srcreg)
{
        uint8_t pc0 = pc;
        switch(srcreg){
                case Fx: return ram[ pc0 & 0x80 ? r:c ][pc++];
                case Mx: return ram[b][o];
                case Bx: return b;
                case Ox: return o;
                case Ax: return a;
                case Ex: return e_new;
                case Sx: return sir;
                case Px: return pir;
                default: return 0;
        }
}


void
scrounge( uint8_t opcode)
{
        switch(opcode & 0x7F){
                case CODE:       b = pc & 0x80 ? r:c;
                                 o = pc;
                                 break;

                case LOCAL:      b = l;
                                 o = 0xF7; //L0
                                 break;

                case LEAVE:      l++;
                                 break;

                case ENTER:      l--;
                                 break;

                case INC:        a++;
                                 break;

                case DEC:        a--;
                                 break;
        }
}

void push_acc(uint8_t v){
        x = a;
        a = v;
}


/* Second part of handling PAIR instructions,
   stores source value into destination
*/
void
pair( uint8_t opcode)
{
        uint8_t src = (opcode >> 4) & 7; /*Zero except bits 4-6 at LSB*/
        uint8_t dst = opcode & 15; /* Zero except bits 0-3 at LSB*/

        scrounge( opcode);

        uint8_t v = srcval(src);
        uint16_t ui16;

        switch(dst){
                case xU: ui16 = o + v>127 ? (uint16_t) v|0x00FF : v;
                         o = ui16 & 0xFF;
                         b = ui16 >> 8;
                         break;
                case xM: ram[b][o] = v;     break;
                case xB: b = v;             break;
                case xO: o = v;             break;
                case xA: push_acc(v);       break;
                case xE: e_old = e_new;
                         e_new = v;         break;
                case xS: sor = v;           break;
                case xP: por = v;           break;

                case xD: d = v;             break;
                case xW: if (d) pc = v;
                         d--; /*Post decrement, either case!*/
                         break;
                case xJ: pc = v;            break;
                case xH: if (a) pc = v;     break;
                case xZ: if (!a) pc = v;    break;
                case xN: if (a&128) pc = v; break;
                case xR: r = v; pc=0x80;    break;
                case xC: call(v);           break;
        }
}


/* Execute GETPUT instruction
*/
void
getput( uint8_t opcode)
{
        /* Opcode BIT 3 encodes GET/PUT mode */
        #define BIT3 8

        /* Opcode BITS 4-5 encode register index (BOAD) */
        #define BITS45 (opcode >> 4) & 3
        
        /* Opcode BITS 0-2 encode address offset in local page (from F8) */
        #define BITS02 opcode & 7

        uint8_t index = BITS02;
        uint8_t *mptr = &(ram[l][GETPUT_OFFSET+index]);

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
        int i;
        uint8_t a0 = a;
        uint8_t ovf = ((a&0x80)^(x&0x80))==0    /* Addends have same sign */
                        && ((i&0x80)^(a&0x80)); /* But result has different sign */
        
        switch(opcode & 15){
                case NOT: a = ~a;                                   break;
                case ALX: a = (a<x)  ? 255:0;                       break;
                case AEX: a = (a==x) ? 255:0;                       break;
                case AGX: a = (a>x)  ? 255:0;                       break;
                case AND: a = a & x;                                break;
                case IOR: a = a | x;                                break;
                case EOR: a = a ^ x;                                break;
                case XA: a=x;                                       break;
                case AX: x=a;                                       break;
                case SWAP: a=x; x=a0;                               break;
                case SHL: a<<=1; x=(a0 & 0x80)? 1:0;                break;
                case SHR: a>>=1; x=(a0 & 1)? 0x80:0;                break;
                case ASR: a=(a>>1)+(a0 & 0x80); x=(a0 & 1)? 0x80:0; break;
                case ADDC: i = x+a; a=i&0xFF; x=(i>255)? 1:0;       break;
                case ADDV: i = x+a; a=i&0xFF; x=ovf?255:0;          break;
                case SUBB: i = x-a; x=(i<0)? 0:1;                   break;
        }
}


/* Execute BOPS instruction
*/
void
bop( uint8_t opcode)
{
        switch(opcode & 7){
                case GRP: b=rp_b; o=rp_o; break;
                case SRP: rp_b=b; rp_o=o; break;
        
                case GIP: b=ip_b; o=ip_o; break;
                case SIP: ip_b=b; ip_o=o; break;
        
                case GTP: b=tp_b; o=tp_o; break;
                case STP: tp_b=b; tp_o=o; break;
        
                case GSP: b=sp_b; o=sp_o; break;
                case SSP: sp_b=b; sp_o=o; break;
                default: break;
        }
}


/* Execute SYS instruction
*/
void
sys( uint8_t opcode)
{
        switch(opcode & 7){ /*Zero except low order 3 bits*/
                case NOP: break;
                case SSI:
                        /*Clocks in MISO line bit into LSB*/
                        sir = (sir<<1) + miso;
                        break;
                case SSO:
                        /*Clocks out MSB first*/
                        mosi = sor & 0x80 ? 1:0;
                        sor <<= 1;
                        break;
                case SCL: sclk = 0; break;
                case SCH: sclk = 1; break;
                case RTI: busy = 0;
                          /* Fall through */
                case RTS:
                        c = b;
                        r = b; /* ! */
                        pc = o;
                        l++;
                        break;
                case COR: cor(); break;
                default: break;
        }
}


int
main(int argc, char *argv[])
{       
        // RESET: Set all regs and BUSY to 0 - match schematics
        // Run myth_step() n times
        exit(0);
}


