
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "srcfile.h"



// This fills the "text" fields of an array of Lines
// with lines from a source text file:

// #define MAX_D_REGS 40 // max object code bytes per line (single digit lits)

// typedef struct {
//     uint8_t index;
//     uint8_t offset;
//     uint8_t data[MAX_D_REGS];
//     uint8_t data_len;
//    char text[81]; // 80 chars + null-terminator
//} Line;

// The idea is to have the source text in "text", and assemble it
// into "data" to be able to match object-code to source lines.


Line **read_lines_from_file(const char *filename, size_t *out_count) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        fprintf( stderr, "Missing file: %s\n", filename);
        return NULL;
    }

    char buffer[1024];
    size_t count = 0;
    size_t capacity = 8;
    Line **lines = malloc(capacity * sizeof(Line *));
    if (!lines) {
        perror("Initial malloc failed");
        fclose(fp);
        return NULL;
    }

    while (fgets(buffer, sizeof(buffer), fp)) {
        size_t len = strlen(buffer);

        if (buffer[len - 1] != '\n' && len >= sizeof(buffer) - 1) {
            int ch;
            while ((ch = fgetc(fp)) != '\n' && ch != EOF);
            continue;
        }

        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }

        if (len > MAX_LINE_LENGTH) {
            continue;
        }

        Line *line = malloc(sizeof(Line));
        if (!line) {
            perror("malloc failed for Line");
            break;
        }

        memset(line, 0, sizeof(Line));
        strncpy(line->text, buffer, sizeof(line->text) - 1);

        if (count >= capacity) {
            size_t new_cap = capacity * 2;
            Line **tmp = realloc(lines, new_cap * sizeof(Line *));
            if (!tmp) {
                perror("realloc failed");
                free(line);
                break;
            }
            lines = tmp;
            capacity = new_cap;
        }

        lines[count++] = line;
    }

    fclose(fp);
    *out_count = count;
    return lines;
}

void free_lines(Line **lines, size_t count) {
    if (!lines) return;
    for (size_t i = 0; i < count; ++i) {
        free(lines[i]);
    }
    free(lines);
}






