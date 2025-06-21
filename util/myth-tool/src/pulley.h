
// Example Pulley:
// C:22 PC:00          E:00(0000_0000) E_OLD:00(0000_0000) | PULLEY
// SCLK:0 MISO:0 MOSI:0      SIR:00 SOR:00   PIR:00 POR:00 |
// A:83(-125,1000_0011)  X:7A(+122,0111_1010)   D:00  L:00 |
// BO:0000  P1:0000 P2:0000 P3:0000 P4:0000   KEY:00 L0:00 |
// L1:02(+002) L2:00(+000) L3:00(+000) L4:00(+000)   IRQ:0 |
// L5:03(+003) L6:00(+000) L7:00(+000) L8:00(+000)  BUSY:0 |

#ifndef PULLEY_H
#define PULLEY_H


#include "cpu.h"

// Pulley stuff


// bool parse_packed_hex4(const char *token, uint8_t *hi, uint8_t *lo) {
//     if (!token || strlen(token) < 4) return false;
//     char high[3] = {token[0], token[1], '\0'};
//     char low[3]  = {token[2], token[3], '\0'};
//     *hi = (uint8_t)strtol(high, NULL, 16);
//     *lo = (uint8_t)strtol(low, NULL, 16);
//     return true;
// }

// static inline char *next_line(char *ptr) {
//     char *newline = strchr(ptr, '\n');
//     return newline ? (newline + 1) : NULL;
// }


// // Requires `out` to be at least 10 bytes: 8 bits + 1 underscore + 1 null terminator
// void binary_literal(uint8_t val, char *out) {
//     for (int i = 7; i >= 0; --i) {
//         *out++ = ((val >> i) & 1) ? '1' : '0';
//         if (i == 4) *out++ = '_';  // Add underscore after 4 bits
//     }
//     *out = '\0';
// }

// void format_signed_3d(int8_t val, char *out) {
//     snprintf(out, 6, "%+04d", val);  // Format like +012, -005
// }


// void emit_pulley_block(FILE *fp) {

//     char bin_e[10], bin_e_old[10];
//     char dec_e[6], dec_e_old[6];
    
//     binary_literal(e, bin_e);
//     binary_literal(e_old, bin_e_old);
    
//     format_signed_3d((int8_t)e, dec_e);
//     format_signed_3d((int8_t)e_old, dec_e_old);

//     fprintf(fp,"\n");

//     // Line 1
//     fprintf(fp, "C:%02X PC:%02X          E:%02X(%s) E_OLD:%02X(%s)\n",
//             c, pc, e, bin_e, e_old, bin_e_old);

//     // Line 2
//     fprintf(fp, "SCLK:%u MISO:%u MOSI:%u      SIR:%02X SOR:%02X   PIR:%02X POR:%02X\n",
//             sclk, miso, mosi, sir, sor, pir, por);

//     // Line 3
//     char bin_a[10], bin_x[10]; 
//     char dec_a[6], dec_x[6];
    
//     binary_literal(a, bin_a);
//     format_signed_3d((int8_t)a, dec_a);
    
//     binary_literal(x, bin_x);
//     format_signed_3d((int8_t)x, dec_x);

//     fprintf(fp, "A:%02X(%s,%s)  X:%02X(%s,%s)   D:%02X  L:%02X\n",
//             a, dec_a, bin_a,
//             x, dec_x, bin_x,
//             d, l);

//     // Line 4
//     fprintf(fp, "BO:%02X%02X  P1:%02X%02X P2:%02X%02X P3:%02X%02X P4:%02X%02X   KEY:%02X L0:%02X\n",
//             b, o,
//             p1b, p1o,
//             p2b, p2o,
//             p3b, p3o,
//             p4b, p4o,
//             k,
//             ram[l][0xF7]);

//     // Line 5
//     fprintf(fp, "L1:%02X(%+04d) L2:%02X(%+04d) L3:%02X(%+04d) L4:%02X(%+04d)   IRQ:%u\n",
//             ram[l][0xF8], (int8_t)ram[l][0xF8],
//             ram[l][0xF9], (int8_t)ram[l][0xF9],
//             ram[l][0xFA], (int8_t)ram[l][0xFA],
//             ram[l][0xFB], (int8_t)ram[l][0xFB],
//             irq);

//     // Line 6
//     fprintf(fp, "L5:%02X(%+04d) L6:%02X(%+04d) L7:%02X(%+04d) L8:%02X(%+04d)  BUSY:%u\n",
//             ram[l][0xFC], (int8_t)ram[l][0xFC],
//             ram[l][0xFD], (int8_t)ram[l][0xFD],
//             ram[l][0xFE], (int8_t)ram[l][0xFE],
//             ram[l][0xFF], (int8_t)ram[l][0xFF],
//             busy);

//     fprintf(fp,"\n");
// }

#endif
