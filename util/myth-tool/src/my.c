#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include <stdbool.h>
#include <getopt.h>


#include "my.h"
#include "srcfile.h"
#include "binfile.h"
#include "cpu.h"
#include "pulley.h"
#include "asm.h"


// This tool, excluding cpu.c, was coauthored with an AI, as I wanted to try.
// It worked surprisingly well not only for boilerplate code.
// I had written a very similar program (assembler-parser) for my 16-bit cpu
// Project (see GitHub) by hand, and wanted to see how the "AI version" looked.
// I've learned that there is a boolean type in C!
// I respect the C99 standard for other good things... but boolean! come on...

#define OFFSET_INPUT 0x0200
#define OFFSET_OUTPUT 0x0280
#define MAX_LENGTH 127

uint8_t print_listing = 0;
uint8_t dump_base = 16;

void print_usage() {
    // ANSI escape sequences for white text
    const char *white = "\033[0;37m";
    const char *reset = "\033[0m";

    // Print the title in white
    printf("\n%sMYth Firmware Development Tool%s\n", white, reset);

    // Print description
    printf("Use without options (first arg does not start with -)\n");
    printf("for dialog mode via buffers.\n");
    printf("Command line is stored at 0x0200, VM responds via\n");
    printf("zero-terminated string at 0x0280, printed to terminal.\n\n");

    // Print "Options:" in white
    printf("%sOptions:%s\n", white, reset);

    // Example options table with left-hand side in white
    printf("  %s-h, --help%s             Display this help message\n", white, reset);
    printf("  %s-u, --usage%s            Show usage information\n", white, reset);
    printf("  %s-N, --name <filename>%s  Create new binary\n", white, reset);
    printf("  %s-p%s                     Print Pulley\n", white, reset);
    printf("  %s-a <filename>%s          Assemble\n", white, reset);
    printf("  %s-l%s                     Optional assembly listing\n", white, reset);
    printf("  %s-b <base 2, 4, 16, 26>%s Set number base for dump\n", white, reset);
    printf("  %s-d <addr dec or XXh>%s   Dump 16 bytes at addr\n", white, reset);
    printf("  %s-s%s                     Single step CPU\n", white, reset);
    printf("  %s-r%s                     Run n CPU cycles\n\n", white, reset);
    printf("  %s-m%s                     Additional cycles for dialog mode query\n\n", white, reset);
    printf("  %s-w%s reg=num             Write into CPU registers\n\n", white, reset);
    printf("  %s-o%s mnemonic            Execute instruction opcode\n\n", white, reset);
}



void handle_N_option(const char *filename) {
    create_new_ram_file(filename);
    printf("New RAM file '%s' created and zeroed.\n", filename);
}


void handle_unknown_option(int opt) {
    fprintf(stderr, "Unknown option: %c\n", opt);
}

int
handle_assemble_option( const char *fname)
{
        Line **lines;
        size_t line_count = 0;

        lines = read_lines_from_file( fname, &line_count);
        if( lines == 0){
            fprintf( stderr, "Failed to read lines from file.\n");
            return -1;
        }

       //Process the lines for opcodes, labels, and literals
       assemble( lines, line_count);
       if (print_listing) write_listing( stdout, lines, line_count);

       free_lines( lines, line_count);
       return 0;
}



// Function to print the stored output from RAM at offset 0x0280
void terminal_output() {
    // Print zero-terminated string up to 127 chars
    int i = 0;
    while (i < MAX_LENGTH && ram[OFFSET_OUTPUT + i] != '\0') {
        putchar(ram[OFFSET_OUTPUT + i]);
        i++;
    }
    putchar('\n');  // Ensure output ends with a newline
}


void
try_dialog()
{
        c=0; pc=0;
        for (unsigned u=0xFFFF; u>0; u--) {
                myth_step();
                if (ram[OFFSET_OUTPUT] != '\0') break;
        }
        if (ram[OFFSET_OUTPUT] != '\0') terminal_output();
        else {
                printf("No response after 64k cycles. ");
                printf("To continue trying, run 'my -m'.\n");
        }
}


int parse_and_set_variable(const char *arg) {
    char varname[16];
    char *equal_sign = strchr(arg, '=');
    if (!equal_sign || equal_sign == arg || *(equal_sign + 1) == '\0') {
        fprintf(stderr, "Invalid format: expected name=value\n");
        return -1;
    }

    size_t name_len = equal_sign - arg;
    if (name_len >= sizeof(varname)) {
        fprintf(stderr, "Variable name too long\n");
        return -1;
    }

    strncpy(varname, arg, name_len);
    varname[name_len] = '\0';

    const char *valstr = equal_sign + 1;
    char *endptr = NULL;
    unsigned long value;

    // Parse value: hex if ends with 'h', else decimal
    size_t len = strlen(valstr);
    if (len > 1 && valstr[len - 1] == 'h') {
        char hexstr[16];
        if (len >= sizeof(hexstr)) {
            fprintf(stderr, "Hex value too long\n");
            return -1;
        }
        strncpy(hexstr, valstr, len - 1);
        hexstr[len - 1] = '\0';
        value = strtoul(hexstr, &endptr, 16);
    } else {
        value = strtoul(valstr, &endptr, 10);
    }

    if (*endptr != '\0' || value > 0xFF) {
        fprintf(stderr, "Invalid or out-of-range value: %s\n", valstr);
        return -1;
    }

    uint8_t byte = (uint8_t)value;

    // "Switch" on variable name
    if (0) {}
    else if (strcmp(varname, "irq") == 0) irq = byte;
    else if (strcmp(varname, "busy") == 0) busy = byte;
    else if (strcmp(varname, "e_old") == 0) e_old = byte;
    else if (strcmp(varname, "e") == 0) e = byte;
    else if (strcmp(varname, "sclk") == 0) sclk = byte;
    else if (strcmp(varname, "miso") == 0) miso = byte;
    else if (strcmp(varname, "mosi") == 0) mosi = byte;
    else if (strcmp(varname, "sir") == 0) sir = byte;
    else if (strcmp(varname, "sor") == 0) sor = byte;
    else if (strcmp(varname, "pir") == 0) pir = byte;
    else if (strcmp(varname, "por") == 0) por = byte;
    else if (strcmp(varname, "a") == 0) a = byte;
    else if (strcmp(varname, "x") == 0) x = byte;
    else if (strcmp(varname, "c") == 0) c = byte;
    else if (strcmp(varname, "pc") == 0) pc = byte;
    else if (strcmp(varname, "b") == 0) b = byte;
    else if (strcmp(varname, "o") == 0) o = byte;
    else if (strcmp(varname, "p1b") == 0) p1b = byte;
    else if (strcmp(varname, "p1o") == 0) p1o = byte;
    else if (strcmp(varname, "p2b") == 0) p2b = byte;
    else if (strcmp(varname, "p2o") == 0) p2o = byte;
    else if (strcmp(varname, "p3b") == 0) p3b = byte;
    else if (strcmp(varname, "p3o") == 0) p3o = byte;
    else if (strcmp(varname, "p4b") == 0) p4b = byte;
    else if (strcmp(varname, "p4o") == 0) p4o = byte;
    else if (strcmp(varname, "k") == 0) k = byte;
    else if (strcmp(varname, "l") == 0) l = byte;
    else if (strcmp(varname, "d") == 0) d = byte;
    else {
        fprintf(stderr, "Unknown variable: %s\n", varname);
        return -1;
    }

    return 0;
}


// Main argument handler
void handle_args(int argc, char *argv[]) {
    int opt;
    int option_index = 0;

    // Define long options
    static struct option long_options[] = {
        {"help",  no_argument,       0, 'h'},
        {0,       0,                 0,  0 }
    };

    unsigned cycs;

    while ((opt = getopt_long(argc, argv, "lhuN:pa:b:d:sr:mw:o:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h':
                print_usage();
                break;
            case 'N':
                handle_N_option(optarg);
                break;
            case 'w':
                parse_and_set_variable(optarg);
                break;
            case 'p':
                emit_pulley_block(stdout);
                break;
            case 'a':
                handle_assemble_option( optarg);
                break;
            case 'l':
                print_listing = 1;
                break;
            case 's':
                myth_step();
                break;
            case 'o': {
                uint8_t opcode;
                if (find_opcode(optarg, &opcode) == -1) {
                    fprintf(stderr, "Error: Unknown mnemonic '%s'\n", optarg);
                    exit(EXIT_FAILURE);
                }
                exec_opcode(opcode);
                break;
            }
            case 'm':
                try_dialog();
                break;
            case 'r':
                cycs = 0;
                if (optarg) {
                    cycs = atoi(optarg);
                } else {
                    fprintf(stderr, "Option -b requires an argument.\n");
                    exit(EXIT_FAILURE);  // Exit with an error if no argument is provided
                }
                while (cycs--) myth_step();
                break;
            case 'b':
                // Check if the argument is provided
                if (optarg) {
                    int base = atoi(optarg);  // Convert the argument to an integer
                    
                    // Check if the base is one of the valid options: 2, 4, 16, or 26
                    if (base == 2 || base == 10 || base == 16 || base == 26) {
                        dump_base = base;  // Set the dump_base variable
                    } else {
                        fprintf(stderr, "Invalid argument for -b: %s. Valid options are 2, 4, 16, or 26.\n", optarg);
                        exit(EXIT_FAILURE);  // Exit with an error if invalid
                    }
                } else {
                    fprintf(stderr, "Option -b requires an argument.\n");
                    exit(EXIT_FAILURE);  // Exit with an error if no argument is provided
                }
                break;

            case 'd':
                // Check if the argument is provided
                if (optarg) {
                    unsigned long long addr = 0;  // Variable to store the address
                    
                    size_t len = strlen(optarg);
                    
                    // Check if the argument ends with 'h' for hex
                    if (len > 1 && optarg[len - 1] == 'h') {
                        // Try to parse the number part before the 'h' as hexadecimal
                        char* hex_part = strndup(optarg, len - 1);  // Copy the string before 'h'
                        addr = strtol(hex_part, NULL, 16);  // Convert to long using base 16
                        free(hex_part);  // Free the duplicated string

                        // Check if the address is within the 16-bit range (0 to 65535)
                        if (addr < 0 || addr > 0xFFFF) {
                            fprintf(stderr, "Invalid hexadecimal value. Must be between 0x0000 and 0xFFFF.\n");
                            exit(EXIT_FAILURE);
                        }
                    } else {
                        // Convert the decimal argument
                        addr = strtoull(optarg, NULL, 10);  // Base 10 for decimal
                    }

                    emit_ram_block(stdout, addr, dump_base);

                } else {
                    fprintf(stderr, "Option -d requires an argument.\n");
                    exit(EXIT_FAILURE);  // Exit with an error if no argument is provided
                }
                break;


            case '?': // unknown option
                //printf("What is this?\n");
                //Fall through
            default:
                handle_unknown_option(opt);
                print_usage();
                exit(EXIT_FAILURE);
        }
    }

    // Handle positional arguments if needed
    if (optind < argc) {
        printf("Non-option arguments:\n");
        while (optind < argc)
            printf("  %s\n", argv[optind++]);
    }
}





// Function to collect  terminal input to RAM at offset 0x0200
int terminal_input(int argc, char *argv[]) {
    // Check if there are command line arguments and if the first one doesn't start with a dash
    if (argc > 1 && argv[1][0] != '-') {
        // Prepare a buffer to hold the command line string
        char cmd_string[MAX_LENGTH + 1] = {0};  // +1 for null terminator
        int i = 0;

        // Iterate over the command line arguments (skip the first one, which is the program name)
        for (int j = 1; j < argc && i < MAX_LENGTH; j++) {
            for (int k = 0; argv[j][k] != '\0' && i < MAX_LENGTH; k++) {
                // Check if the character is a valid ASCII character (0-127)
                // if ((unsigned char)argv[j][k] > 127) {
                //     fprintf(stderr, "Error: Non-ASCII character detected in input.\n");
                //     return 1;
                // }

                cmd_string[i++] = argv[j][k];
            }
        }

        // Store the string in RAM at offset 0x0200
        for (int i = 0; i < MAX_LENGTH; i++) {
            ram[OFFSET_INPUT + i] = cmd_string[i];
        }
        return 0;
    } else
        return -1;
}


int
main( int argc, char *argv[])
{
        char *fname = "rom.bin"; // Default name

        if(read_ram(fname) == -1) handle_N_option(fname);

        ram[OFFSET_OUTPUT] = '\0'; // Terminate string buffer

        if (terminal_input(argc, argv) == -1) handle_args( argc, argv);
        else try_dialog();

        if(write_ram(fname)) exit( EXIT_FAILURE);
        else return EXIT_SUCCESS;
}










