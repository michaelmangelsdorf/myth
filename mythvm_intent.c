
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
uint8_t busy; /* Interrupts disabled flag (output) */

uint8_t sclk; /* Serial clock (output) */
uint8_t miso;
uint8_t mosi;
uint8_t sir;
uint8_t sor;

uint8_t pir;
uint8_t por;

uint8_t a;
uint8_t a0;
uint8_t t;
uint8_t t0;

uint8_t o;
uint8_t d;
uint8_t pc;

uint8_t c;
uint8_t b;
uint8_t l;

uint8_t p1_b;
uint8_t p2_b;
uint8_t p3_b;
uint8_t sp_b;

uint8_t p1_o;
uint8_t p2_o;
uint8_t p3_o;
uint8_t sp_o;


void reset();
void myth_step();
static uint8_t fetch_opcode();
static uint8_t srcval( uint8_t srcreg);
static void scrounge( uint8_t opcode);
static void pair( uint8_t opcode);
static void getput( uint8_t opcode);
static void trap( uint8_t opcode);
static void alu( uint8_t opcode);
static void bops( uint8_t opcode);
static void sys( uint8_t opcode);
static void call( uint8_t dstpage);



/* The 'REGx' notation means: REG into (something)
   i.e. REG is a source
*/

#define Fx 0 /*from FETCH literal*/
#define Mx 1 /*from MEMORY @ BASE:OFFSET*/
#define Bx 2 /*from BASE register*/
#define Ox 3 /*from OFFSET register*/
#define Ax 4 /*from ACCUMULATOR register*/
#define Ex 5 /*from ENABLE register*/
#define Sx 6 /*from SERIAL input*/
#define Px 7 /*from PARALLEL input*/


/* The 'xREG' notation means: (something) into REG
   i.e. REG is a destination
*/

#define xL 0  /*to LOCAL register*/
#define xM 1  /*to MEMORY @ BASE:OFFSET*/
#define xB 2  /*to BASE register*/
#define xO 3  /*to OFFSET register*/
#define xA 4  /*to ACCUMULATOR register*/
#define xE 5  /*to ENABLE register*/
#define xS 6  /*to SERIAL output*/
#define xP 7  /*to PARALLEL output*/

#define xD 8  /*to DOWN-COUNTER register*/
#define xU 9  /* UPDATE - add signed byte to 16-bit pair B:O)*/
#define xG 10 /* GO - write to C, jumps to C:O, store return pointer in B:O*/
#define xJ 11 /* JUMP - write to pc*/
#define xW 12 /* WHILE JUMP - write to PC WHILE D not zero, decrement D*/
#define xN 13 /* NOT-ZERO JUMP - write to PC if A not zero*/
#define xZ 14 /* ZERO JUMP- write to PC if A zero*/
#define xC 15 /* CALL - write to C, calls C:O, store return pointer in B:O*/


/*ALU Instructions
*/

#define COA 0 /*Copy A*/
#define COT 1 /*Copy T*/
#define OCA 2 /*Ones' complement of A*/
#define OCT 3 /*Ones' complement of T*/
#define SLA 4 /*Shift left A*/
#define SLT 5 /*Shift left T*/
#define SRA 6 /*Shift right A*/
#define SRT 7 /*Shift right T*/
#define AND 8 /*A AND T*/
#define IOR 9 /*A OR T*/
#define EOR 10 /*A XOR T*/
#define ADD 11 /*A + T*/
#define CAR 12 /*Carry of: A + T (0 or 1)*/
#define ALT 13 /*255 if A<T else 0*/
#define AET 14 /*255 if A=T else 0*/
#define AGT 15 /*255 if A>T else 0*/


/*SYS Instructions
*/

#define NOP 0 /* No Operation */
#define SSI 1 /* Serial Shift In */
#define SSO 2 /* Serial Shift Out */
#define SCL 3 /* Set serial Clock Low */
#define SCH 4 /* Set serial Clock High */
#define RTS 5 /* Return from Subroutine */
#define RTI 6 /* Return from Interrupt */
#define KEY 7 /* NOP for now */


/*BOP Instructions
*/

#define P1BO 0 /* Store P1 into B:O */
#define BOP1 1 /* Store B:O into P1 */
#define P2BO 2
#define BOP2 3
#define P3BO 4
#define BOP3 5
#define SPBO 6 /* P4 => SP Stack Pointer */
#define BOSP 7

#define GETPUT_OFFSET 0xF8 /*Local-page offset used by GETPUT instructions*/



void
reset() {

     //   memset(ram[256][256], 0, 256*256); /* Clear RAM */

         e_old = 0; /* LO Nybble 0: NOP, 1: POE, HI Nybble: 0: NOP, 1: PLE */
         e_new = 0;

         irq = 0; /* Interrupt Request flag (input) */
         busy = 0; /* Interrupts disabled flag (output) */

         sclk = 0; /* Serial clock (output) */
         miso = 0;
         mosi = 0;
         sir = 0;
         sor = 0;

         pir = 0;
         por = 0;

         a = 0;  /* ALU registers */
         a0 = 0;
         t = 0;
         t0 = 0;


         o = 0;
         d = 0;
         pc = 0;

         c = 0;
         b = 0;
         l = 0;
         t = 0;

         p1_b = 0;
         p2_b = 0;
         p3_b = 0;
         sp_b = 0;

         p1_o = 0;
         p2_o = 0;
         p3_o = 0;
         sp_o = 0;
}


/* Single-step 1 instruction cycle
*/
void
myth_step()
{
        uint8_t opcode = fetch_opcode();

        /*Decode priority encoded opcode
         and execute decoded instruction*/
        if (opcode & 0x80) pair( opcode);
        else if (opcode & 0x40) getput( opcode);
        else if (opcode & 0x20) trap( opcode);
        else if (opcode & 0x10) alu( opcode);
        else if (opcode & 0x08) bops( opcode);
        else sys( opcode);
}


/* Fetch next byte in CODE stream, then increment PC.
   Fetches either an instruction, or an instruction literal (Nx)
*/
uint8_t
fetch_opcode()
{
        if (irq && c>0) {
                busy = 1;
                return 32; /* TRAP0 */
        } else
                return ram[c][pc++];
}



void
go( uint8_t dstpage)
{
        uint8_t temp = o;
        o = pc;
        pc = temp;

        b = c;
        c = dstpage;

        call( dstpage);
}

void
call( uint8_t dstpage)
{
        go( dstpage);
        /*Create stack frame*/
        l--;
}


void
trap( uint8_t opcode)
{
        uint8_t dstpage = opcode & 31; /*Zero except low order 5 bits*/

        o = pc;

        /*Branch to page head*/
        pc = 0;

        b = c;
        c = dstpage;

        /*Create stack frame*/
        l--;

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

                case 16*Ax + xA: /*INC*/
                                 a++;
                                 break;

                case 16*Ex + xE: /*DEC*/
                                 a--;
                                 break;
        }
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
                case xL: l = v;         break;
                case xM: ram[b][o] = v; break;
                case xB: b = v;         break;
                case xO: o = v;         break;
                case xA: a = v;         break;
                
                case xE: e_old = e_new;
                         e_new = v;     break;
                
                case xS: sor = v;       break;
                case xP: por = v;       break;
                case xD: d = v;         break; 
                
                case xU: ui16 = o + v>127 ? (uint16_t) v|0x00FF : v;
                         o = ui16 & 0xFF;
                         b = ui16 >> 8;
                         break;

                case xG: go(v);         break;
                case xJ: pc = v;        break;
                
                case xW: if (d) pc = v;
                         d--; /*Post decrement, either case!*/
                         break;

                case xN: if (a) pc = v;  break;
                case xZ: if (!a) pc = v; break;
                case xC: call(v);        break;
        }
}


void
getput( uint8_t opcode) /*Execute GETPUT instruction*/
{
        /* OPCODE
            BITS 0-2 encode byte address offset in local page (from F8)
            BIT 3 encodes GET/PUT mode
            BITS 4-5 encode register index (BOAD)
        */

        #define BIT3 8
        #define BITS45 (opcode >> 4) & 3
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
                case 0: b = *mptr; break;
                case 1: o = *mptr; break;
                case 2: a = *mptr; break;
                case 3: d = *mptr; break;
        }
}


void
alu( uint8_t opcode)
{
        int i;
        uint8_t u, t = a;

        switch(opcode & 15){
                case COA:             break;
                case COT: a=t0;        break;
                case OCA: a = ~a0;     break;
                case OCT: a = ~t0;     break;
                case SLA: a = a0 << 1; break;
                case SLT: a = t0 << 1; break;
                case SRA: a = a0 >> 1; break;
                case SRT: a = t0 >> 1; break;
                
                case AND: a = a0 & t0;  break;
                case IOR: a = a0 | t0;  break;
                case EOR: a = a0 ^ t0;  break;
                case ADD: a = a0 + t0;  break;
                
                case CAR: i = a0 + t0;
                          a =  (i > 255) ? 1 : 0;
                          break;

                case ALT: a = (a0<t0)  ? 255 : 0; break;
                case AET: a = (a0==t0) ? 255 : 0; break;
                case AGT: a = (a0>t0)  ? 255 : 0; break;
        }
        t0=t;
}


void
bops( uint8_t opcode)
{
        switch(opcode & 7){
                caseP1BO: b=p1_b; o=p1_o; break;
                caseBOP1: p1_b=b; p1_o=o; break;
        
                caseP2BO: b=p2_b; o=p2_o; break;
                caseBOP2: p2_b=b; p2_o=o; break;
        
                caseP3BO: b=p3_b; o=p3_o; break;
                caseBOP3: p3_b=b; p3_o=o; break;
        
                caseSPBO: b=sp_b; o=sp_o; break;
                caseBOSP: sp_b=b; sp_o=o; break;
                default: break;
        }
}


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
                        pc = o;
                        l++;
                        break;
                case KEY: /* Fall through */
                default: break;
        }
}


int
main(int argc, char *argv[])
{       
        irq = 0;
        reset();
        // Run fetch n times
        exit(0);
}


