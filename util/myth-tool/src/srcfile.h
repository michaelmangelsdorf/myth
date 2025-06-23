#ifndef FILE_H
#define FILE_H

#include <stdint.h>
#include <stddef.h>

#include "asm.h"

#define MAX_LINE_LENGTH 80


Line **read_lines_from_file(const char *filename, size_t *out_count);


void free_lines(Line **lines, size_t count);

#endif // FILE_H



