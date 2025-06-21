#ifndef CPU_H
#define CPU_H

extern uint8_t ram[256*256];
extern uint8_t irq;
extern uint8_t busy;
extern uint8_t e_old;
extern uint8_t e;
extern uint8_t sclk;
extern uint8_t miso;
extern uint8_t mosi;
extern uint8_t sir;
extern uint8_t sor;
extern uint8_t pir;
extern uint8_t por;
extern uint8_t a;
extern uint8_t x;
extern uint8_t c;
extern uint8_t pc;
extern uint8_t b;
extern uint8_t o;
extern uint8_t p1b, p1o;
extern uint8_t p2b, p2o;
extern uint8_t p3b, p3o;
extern uint8_t p4b, p4o;
extern uint8_t k;
extern uint8_t l;
extern uint8_t d;


extern void myth_step();
uint8_t fetch();
void push_acc(uint8_t v);
uint8_t srcval(uint8_t srcreg);
uint8_t scrounge(uint8_t opcode);
void pair(uint8_t opcode);
void getput(uint8_t opcode);
void trap(uint8_t opcode);
void alu(uint8_t opcode);
void bop(uint8_t opcode);
void sys(uint8_t opcode);
void call(uint8_t dstpage);

#endif // CPU_H
