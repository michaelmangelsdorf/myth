# Compiler and flags
CC = clang
CFLAGS = -Wall -Wextra -g
LDFLAGS = 

# Source files
SRC = my.c cpu.c srcfile.c binfile.c asm2.c
OBJ = $(SRC:.c=.o)

# Output binary name
TARGET = ../my

# Default target (build the project)
all: $(TARGET)

# Linking the object files into the final executable
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

# Compile each .c file into a .o file
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up the build (remove object files and binary)
clean:
	rm -f $(OBJ) $(TARGET)

# Make dependencies (you can add more as needed)
depend: $(SRC)
	$(CC) -MM $(SRC) > .depend

# Include the dependencies if they exist
ifneq ($(MAKECMDGOALS),clean)
  -include .depend
endif