#ifndef FILE_H
#define FILE_H

#include <stdint.h>
#include <stddef.h>

#include "my.h"

#define MAX_LINE_LENGTH 80


/**
 * Reads valid lines (<= 80 chars) from a file.
 * Allocates Line* for each valid line and returns a dynamic array of pointers.
 * 
 * @param filename   The path to the input file.
 * @param out_count  Pointer to size_t that will hold the number of valid lines.
 * @return           Pointer to a dynamically allocated array of Line* (caller must free).
 */
Line **read_lines_from_file(const char *filename, size_t *out_count);

/**
 * Frees the array and all Line* inside.
 */
void free_lines(Line **lines, size_t count);

#endif // FILE_H



