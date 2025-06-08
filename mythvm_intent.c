
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
uint8_t wp_b;
uint8_t ip_b;
uint8_t sp_b;

uint8_t rp_o; /* Pointer Offset */
uint8_t wp_o;
uint8_t ip_o;
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



/* The 'REGx' notation means: REG into (something)
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


/* The 'xREG' notation means: (something) into REG
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


/*ALU Instructions
*/

#define DUP 0 /* Copy of A */
#define SWAP 1 /* Copy of X */
#define NOTA 2 /* Ones' complement of A */
#define NOTX 3 /* Ones' complement of X */
#define SLA 4 /* A shifted left */
#define SLX 5 /* X shifted left */
#define SRA 6 /* A shifted right*/
#define SRX 7 /* X shifted right*/
#define AND 8 /* A AND X */
#define IOR 9 /* A OR X */
#define EOR 10 /* A XOR X */
#define ADD 11 /* Add X to A (low order 8-bits) */
#define OVF 12 /* Overflow bits of: A+B - B6:V, B7:CARRY */
#define ALX 13 /* 255 if A<X else 0 */
#define AEX 14 /* 255 if A=X else 0 */
#define AGX 15 /* 255 if A>X else 0 */


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

#define RBO 0 /* Store RP into B:O */
#define BOR 1 /* Store B:O into RP */
#define WBO 2
#define BOW 3
#define IBO 4
#define BOI 5
#define SBO 6 /* P4 => SP Stack Pointer */
#define BOS 7

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
        if (irq && c>0) {
                busy = 1;
                return 32; /* TRAP0 */
        } else
        /* Instruction byte page offset high: map "resident" routine */
                return ram[ pc0 & 0x80 ? r:c ][pc++];
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
        call(dstpage);
}


/* First part of handling PAIR instructions,
   derives the source value
*/
uint8_t
srcval( uint8_t srcreg)
{
        switch(srcreg){
                case Fx: return ram[c][pc++];
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
        switch(opcode & 0x7F /*0111_1111*/){
                case 16*Fx + xM: /*CODE*/
                                 b = c;
                                 o = pc;
                                 break;

                case 16*Mx + xM: /*LOCAL*/
                                 b = l;
                                 o = 0xF7; //L0
                                 break;

                case 16*Bx + xB: /*LEAVE*/
                                 l++;
                                 break;

                case 16*Ox + xO: /*ENTER*/
                                 l--;
                                 break;

                case 16*Ax + xA: /*INCA*/
                                 a++;
                                 break;

                case 16*Ex + xE: /*DECA*/
                                 a--;
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
        uint8_t a0 = a, carry, overflow;

        switch(opcode & 15){
                case DUP:                break;
                case SWAP: a=x;           break;
                case NOTA: a = ~a;        break;
                case NOTX: a = ~x;        break;
                case SLA: a = a << 1;    break;
                case SLX: a = x << 1;    break;
                case SRA: a = a >> 1;    break;
                case SRX: a = x >> 1;    break;
                
                case AND: a = a & x;  break;
                case IOR: a = a | x;  break;
                case EOR: a = a ^ x;  break;
                case ADD: a = a + x;  break;
                
                case OVF: i = a + x; // Compute signed/unsigned overflow flags
                          carry =  (i > 255) ? 1 : 0;
                          overflow = ((a&0x80) ^ (x&0x80)) == 0 /* Addends have same sign */
                                   && ((i&0x80) != (a&0x80)); /* But result has different sign */
                          a = carry? 0x80:0 + overflow? 0x40:0; /* Bit 7=Carry, Bit 6=Overflow */
                          break;

                case ALX: a = (a<x)  ? 255 : 0; break;
                case AEX: a = (a==x) ? 255 : 0; break;
                case AGX: a = (a>x)  ? 255 : 0; break;
        }
        x = a0;
}


/* Execute BOPS instruction
*/
void
bop( uint8_t opcode)
{
        switch(opcode & 7){
                case RBO: b=rp_b; o=rp_o; break;
                case BOR: rp_b=b; rp_o=o; break;
        
                case WBO: b=wp_b; o=wp_o; break;
                case BOW: wp_b=b; wp_o=o; break;
        
                case IBO: b=ip_b; o=ip_o; break;
                case BOI: ip_b=b; ip_o=o; break;
        
                case SBO: b=sp_b; o=sp_o; break;
                case BOS: sp_b=b; sp_o=o; break;
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
        // Set all regs and BUSY to 0 - match schematics
        // Run fetch() n times
        exit(0);
}


