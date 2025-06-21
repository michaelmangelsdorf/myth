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


// This code, excluding cpu.c, was coauthored with an AI, to try it out
// worked surprisingly well, as this seems the perfect use-case
// for it, generating boilerplate such as this.
// I had written a very similar program (assembler-parser) for my 16-bit cpu
// Project (see GitHub) and wanted to see how the AI did it.
// I've learned that there is a boolean type in C!
// I respect the C99 standard for other good things... but boolean! come on...


// Print usage
void print_usage(const char *prog_name) {
    printf("Usage: %s [OPTIONS]\n", prog_name);
    printf("Options:\n");
    printf("  -h, --help             Display this help message\n");
    printf("  -u, --usage            Show usage information\n");
    printf("  -N, --name <filename>  Create new binary\n");
    printf("  -p                     Print Pulley\n");
    printf("  -a <filename>          Assemble\n");
}

// Stub function implementations
void handle_help_option(const char *arg) {
    (void)arg; // unused
    printf("Help: This program demonstrates getopt() handling.\n");
    printf("Use --help or -h to view this message.\n");
}

void handle_usage_option(const char *arg) {
    (void)arg; // unused
    printf("Usage option invoked.\n");
    // Optionally call print_usage() here
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
       write_listing( stdout, lines, line_count);

       free_lines( lines, line_count);
       return 0;
}




// Main argument handler
void handle_args(int argc, char *argv[]) {
    int opt;
    int option_index = 0;

    // Define long options
    static struct option long_options[] = {
        {"help",  no_argument,       0, 'h'},
        {"usage", no_argument,       0, 'u'},
        {0,       0,                 0,  0 }
    };

    while ((opt = getopt_long(argc, argv, "huN:pa:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h':
                handle_help_option(NULL);
                break;
            case 'u':
                handle_usage_option(NULL);
                break;
            case 'N':
                handle_N_option(optarg);
                break;
            case 'p':
                //emit_pulley_block(stdout);
                break;
            case 'a':
                handle_assemble_option( optarg);
                break;
            case '?': // unknown option
                //printf("What is this?\n");
                //Fall through
            default:
                handle_unknown_option(opt);
                print_usage(argv[0]);
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






int
main( int argc, char *argv[])
{
        char *fname = "rom.bin"; // Default name

        if( read_ram(fname) == -1)
                handle_N_option(fname);

        handle_args( argc, argv);

        if( write_ram(fname)) exit( EXIT_FAILURE);
        return EXIT_SUCCESS;
}










