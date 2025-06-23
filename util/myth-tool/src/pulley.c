

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <stdbool.h>
#include <getopt.h>


#include "pulley.h"
#include "cpu.h"

// Pulley stuff


// Example Pulley:
// C:22 PC:00          E:00(0000_0000) E_OLD:00(0000_0000) | PULLEY
// SCLK:0 MISO:0 MOSI:0      SIR:00 SOR:00   PIR:00 POR:00 |
// A:83(-125,1000_0011)  X:7A(+122,0111_1010)   D:00  L:00 |
// BO:0000  P1:0000 P2:0000 P3:0000 P4:0000   KEY:00 L0:00 |
// L1:02(+002) L2:00(+000) L3:00(+000) L4:00(+000)   IRQ:0 |
// L5:03(+003) L6:00(+000) L7:00(+000) L8:00(+000)  BUSY:0 |


bool parse_packed_hex4(const char *token, uint8_t *hi, uint8_t *lo) {
    if (!token || strlen(token) < 4) return false;
    char high[3] = {token[0], token[1], '\0'};
    char low[3]  = {token[2], token[3], '\0'};
    *hi = (uint8_t)strtol(high, NULL, 16);
    *lo = (uint8_t)strtol(low, NULL, 16);
    return true;
}


// Requires `out` to be at least 10 bytes: 8 bits + 1 underscore + 1 null terminator
void binary_literal(uint8_t val, char *out) {
    for (int i = 7; i >= 0; --i) {
        *out++ = ((val >> i) & 1) ? '1' : '0';
        if (i == 4) *out++ = '_';  // Add underscore after 4 bits
    }
    *out = '\0';
}

void format_signed_3d(int8_t val, char *out) {
    snprintf(out, 6, "%+04d", val);  // Format like +012, -005
}


void emit_pulley_block(FILE *fp) {
    const char *label_color = "";
    const char *reset_color = "";

    if (fp == stdout) {
        label_color = "\033[97m";  // Bright white
        reset_color = "\033[0m";   // Reset
    }

    char bin_e[10], bin_e_old[10];
    char dec_e[6], dec_e_old[6];

    binary_literal(e, bin_e);
    binary_literal(e_old, bin_e_old);

    format_signed_3d((int8_t)e, dec_e);
    format_signed_3d((int8_t)e_old, dec_e_old);

    fprintf(fp, "\n");

    // Line 1
    fprintf(fp, "%sC:%s%02X %sPC:%s%02X          %sE:%s%02X(%s) %sE_OLD:%s%02X(%s)\n",
            label_color, reset_color, c,
            label_color, reset_color, pc,
            label_color, reset_color, e, bin_e,
            label_color, reset_color, e_old, bin_e_old);

    // Line 2
    fprintf(fp, "%sSCLK:%s%u %sMISO:%s%u %sMOSI:%s%u      %sSIR:%s%02X %sSOR:%s%02X   %sPIR:%s%02X %sPOR:%s%02X\n",
            label_color, reset_color, sclk,
            label_color, reset_color, miso,
            label_color, reset_color, mosi,
            label_color, reset_color, sir,
            label_color, reset_color, sor,
            label_color, reset_color, pir,
            label_color, reset_color, por);

    // Line 3
    char bin_a[10], bin_x[10];
    char dec_a[6], dec_x[6];

    binary_literal(a, bin_a);
    format_signed_3d((int8_t)a, dec_a);

    binary_literal(x, bin_x);
    format_signed_3d((int8_t)x, dec_x);

    fprintf(fp, "%sA:%s%02X(%s,%s)  %sX:%s%02X(%s,%s)   %sD:%s%02X  %sL:%s%02X\n",
            label_color, reset_color, a, dec_a, bin_a,
            label_color, reset_color, x, dec_x, bin_x,
            label_color, reset_color, d,
            label_color, reset_color, l);

    // Line 4
    fprintf(fp, "%sBO:%s%02X%02X  %sP1:%s%02X%02X %sP2:%s%02X%02X %sP3:%s%02X%02X %sP4:%s%02X%02X   %sKEY:%s%02X %sL0:%s%02X\n",
            label_color, reset_color, b, o,
            label_color, reset_color, p1b, p1o,
            label_color, reset_color, p2b, p2o,
            label_color, reset_color, p3b, p3o,
            label_color, reset_color, p4b, p4o,
            label_color, reset_color, k,
            label_color, reset_color, ram[l * 256 + 0xF7]);

    // Line 5
    fprintf(fp, "%sL1:%s%02X(%+04d) %sL2:%s%02X(%+04d) %sL3:%s%02X(%+04d) %sL4:%s%02X(%+04d)   %sIRQ:%s%u\n",
            label_color, reset_color, ram[l * 256 + 0xF8], (int8_t)ram[l * 256 + 0xF8],
            label_color, reset_color, ram[l * 256 + 0xF9], (int8_t)ram[l * 256 + 0xF9],
            label_color, reset_color, ram[l * 256 + 0xFA], (int8_t)ram[l * 256 + 0xFA],
            label_color, reset_color, ram[l * 256 + 0xFB], (int8_t)ram[l * 256 + 0xFB],
            label_color, reset_color, irq);

    // Line 6
    fprintf(fp, "%sL5:%s%02X(%+04d) %sL6:%s%02X(%+04d) %sL7:%s%02X(%+04d) %sL8:%s%02X(%+04d)  %sBUSY:%s%u\n",
            label_color, reset_color, ram[l * 256 + 0xFC], (int8_t)ram[l * 256 + 0xFC],
            label_color, reset_color, ram[l * 256 + 0xFD], (int8_t)ram[l * 256 + 0xFD],
            label_color, reset_color, ram[l * 256 + 0xFE], (int8_t)ram[l * 256 + 0xFE],
            label_color, reset_color, ram[l * 256 + 0xFF], (int8_t)ram[l * 256 + 0xFF],
            label_color, reset_color, busy);

    fprintf(fp, "\n");
}


void emit_ram_block(FILE *fp, uint16_t addr, int base) {
    const char *label_color = "";
    const char *reset_color = "";

    if (fp == stdout) {
        label_color = "\033[97m";  // Bright white
        reset_color = "\033[0m";   // Reset
    }

    if (addr > 0xFFFF - 15) {
        fprintf(fp, "%sAddress out of range:%s %04X\n", label_color, reset_color, addr);
        return;
    }

    if (base == 2) {
        // Binary format: address label + first value, then indented rows
        uint8_t val = ram[addr];
        char bin[10], dec[6];
        binary_literal(val, bin);
        format_signed_3d((int8_t)val, dec);
        fprintf(fp, "%s%04X:%s %02X(%s,%s)\n", label_color, addr, reset_color, val, dec, bin);

        for (int i = 1; i < 16; ++i) {
            val = ram[addr + i];
            binary_literal(val, bin);
            format_signed_3d((int8_t)val, dec);
            fprintf(fp, "      %02X(%s,%s)\n", val, dec, bin);  // Indent to align
        }

    } else if (base == 26) {
        // ASCII printable line in double quotes
        fprintf(fp, "%s%04X:%s \"", label_color, addr, reset_color);

        for (int i = 0; i < 16; ++i) {
            uint8_t val = ram[addr + i];
            char ch = (val >= 32 && val <= 126) ? (char)val : '.';
            fputc(ch, fp);
        }

        fprintf(fp, "\"\n");

    } else {
        // Base 10 or 16: print two rows of 8 bytes (grouped by 4)
        for (int row = 0; row < 2; ++row) {
            uint16_t line_addr = addr + row * 8;
            fprintf(fp, "%s%04X:%s", label_color, line_addr, reset_color);

            for (int i = 0; i < 8; ++i) {
                uint8_t val = ram[line_addr + i];

                switch (base) {
                    case 10: {
                        char dec[6];
                        format_signed_3d((int8_t)val, dec);
                        fprintf(fp, " %s", dec);
                        break;
                    }
                    case 16:
                    default:
                        fprintf(fp, " %02X", val);
                        break;
                }

                if ((i + 1) % 4 == 0 && i != 7) {
                    fprintf(fp, " ");
                }
            }

            fprintf(fp, "\n");
        }
    }
}
