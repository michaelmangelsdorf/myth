
#include "binfile.h"


extern int
write_ram(const char *filename)
{
    FILE *file = fopen(filename, "r+b");  // Open file for reading and writing in binary mode
    if (file == NULL) {
        fprintf( stderr, "Error writing file: %s\n", filename);
        return -1;
    }

    // Write the entire RAM array
    fwrite(ram, sizeof(uint8_t), sizeof(ram), file);

    fseek(file, REG_BUFFER_OFFSET, SEEK_SET);

    // Write the state variables to the file
    fwrite(&irq, sizeof(uint8_t), 1, file);
    fwrite(&busy, sizeof(uint8_t), 1, file);
    fwrite(&e_old, sizeof(uint8_t), 1, file);
    fwrite(&e, sizeof(uint8_t), 1, file);
    fwrite(&sclk, sizeof(uint8_t), 1, file);
    fwrite(&miso, sizeof(uint8_t), 1, file);
    fwrite(&mosi, sizeof(uint8_t), 1, file);
    fwrite(&sir, sizeof(uint8_t), 1, file);
    fwrite(&sor, sizeof(uint8_t), 1, file);
    fwrite(&pir, sizeof(uint8_t), 1, file);
    fwrite(&por, sizeof(uint8_t), 1, file);
    fwrite(&a, sizeof(uint8_t), 1, file);
    fwrite(&x, sizeof(uint8_t), 1, file);
    fwrite(&c, sizeof(uint8_t), 1, file);
    fwrite(&pc, sizeof(uint8_t), 1, file);
    fwrite(&b, sizeof(uint8_t), 1, file);
    fwrite(&o, sizeof(uint8_t), 1, file);
    fwrite(&p1b, sizeof(uint8_t), 1, file);
    fwrite(&p1o, sizeof(uint8_t), 1, file);
    fwrite(&p2b, sizeof(uint8_t), 1, file);
    fwrite(&p2o, sizeof(uint8_t), 1, file);
    fwrite(&p3b, sizeof(uint8_t), 1, file);
    fwrite(&p3o, sizeof(uint8_t), 1, file);
    fwrite(&p4b, sizeof(uint8_t), 1, file);
    fwrite(&p4o, sizeof(uint8_t), 1, file);
    fwrite(&k, sizeof(uint8_t), 1, file);
    fwrite(&l, sizeof(uint8_t), 1, file);
    fwrite(&d, sizeof(uint8_t), 1, file);

    fclose(file);
    return 0;
}


// Read the entire RAM and the variables from the file, with regs at 0x100
extern int
read_ram(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf( stderr, "Missing file: %s\n", filename);
        return -1;
    }

    // Read the entire RAM array
    fread(ram, sizeof(uint8_t), sizeof(ram), file);

    // Seek to position 0x100 for variables
    fseek(file, REG_BUFFER_OFFSET, SEEK_SET);

    // Read the state variables from the file
    fread(&irq, sizeof(uint8_t), 1, file);
    fread(&busy, sizeof(uint8_t), 1, file);
    fread(&e_old, sizeof(uint8_t), 1, file);
    fread(&e, sizeof(uint8_t), 1, file);
    fread(&sclk, sizeof(uint8_t), 1, file);
    fread(&miso, sizeof(uint8_t), 1, file);
    fread(&mosi, sizeof(uint8_t), 1, file);
    fread(&sir, sizeof(uint8_t), 1, file);
    fread(&sor, sizeof(uint8_t), 1, file);
    fread(&pir, sizeof(uint8_t), 1, file);
    fread(&por, sizeof(uint8_t), 1, file);
    fread(&a, sizeof(uint8_t), 1, file);
    fread(&x, sizeof(uint8_t), 1, file);
    fread(&c, sizeof(uint8_t), 1, file);
    fread(&pc, sizeof(uint8_t), 1, file);
    fread(&b, sizeof(uint8_t), 1, file);
    fread(&o, sizeof(uint8_t), 1, file);
    fread(&p1b, sizeof(uint8_t), 1, file);
    fread(&p1o, sizeof(uint8_t), 1, file);
    fread(&p2b, sizeof(uint8_t), 1, file);
    fread(&p2o, sizeof(uint8_t), 1, file);
    fread(&p3b, sizeof(uint8_t), 1, file);
    fread(&p3o, sizeof(uint8_t), 1, file);
    fread(&p4b, sizeof(uint8_t), 1, file);
    fread(&p4o, sizeof(uint8_t), 1, file);
    fread(&k, sizeof(uint8_t), 1, file);
    fread(&l, sizeof(uint8_t), 1, file);
    fread(&d, sizeof(uint8_t), 1, file);

    fclose(file);
    return 0;
}



// Function to create a new zeroed RAM binary file
void
create_new_ram_file(const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (file != NULL) {
    // Write zeroed RAM to the file
        memset(ram, 0, sizeof(ram));
        if (fwrite(ram, sizeof(uint8_t), sizeof(ram), file) != sizeof(ram)) {
            perror("Error writing zeroed RAM to file");
            exit(1);
        }
        fprintf( stderr, "Created zeroed file: %s\n", filename);
        exit(1);
    }
    fclose(file);
}


