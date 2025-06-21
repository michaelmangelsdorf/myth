#ifndef BINFILE_H
#define BINFILE_H

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "my.h"
#include "cpu.h"


#define MAX_LINE_LENGTH 80


extern int write_ram(const char *filename);
extern int read_ram(const char *filename);
void create_new_ram_file(const char *filename);


#endif // BINFILE_H



