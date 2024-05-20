#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CRC32_POLYNOMIAL 0xEDB88320
#define BUFFER_SIZE 1024

unsigned int crc32_table[256];

void generate_crc32_table() {
    unsigned int crc, i, j;

    for (i = 0; i < 256; i++) {
        crc = i;
        for (j = 0; j < 8; j++) {
            crc = (crc & 1) ? (CRC32_POLYNOMIAL ^ (crc >> 1)) : (crc >> 1);
        }
        crc32_table[i] = crc;
    }
}

unsigned int swap_endian(unsigned int num) {
    return ((num & 0xFF) << 24) |
           ((num & 0xFF00) << 8) |
           ((num & 0xFF0000) >> 8) |
           ((num & 0xFF000000) >> 24);
}

unsigned int getCrc32(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Cannot open file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    unsigned int crc32 = 0xFFFFFFFF;
    unsigned char buffer[BUFFER_SIZE];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        for (size_t i = 0; i < bytesRead; i++) {
            crc32 = crc32_table[(crc32 ^ buffer[i]) & 0xFF] ^ (crc32 >> 8);
        }
    }

    crc32 ^= 0xFFFFFFFF;
    fclose(file);
    return crc32;
}

unsigned int filesize(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("Cannot open file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    unsigned int size = ftell(file);
    fclose(file);
    return size;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <output_file> <input_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    generate_crc32_table();
    unsigned int crc = getCrc32(argv[2]);
    //crc = swap_endian(crc);
    printf("%X\n", crc);

    FILE *file = fopen(argv[1], "r+b");
    if (file == NULL) {
        printf("Cannot open file: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    // write lpfw crc
    fseek(file, -64, SEEK_END);
    fwrite(&crc, sizeof(unsigned int), 1, file);

    // write LPFW ,lpfw size, and padding
    fseek(file, -32, SEEK_END);
    char magic[32];
    fread(magic, sizeof(char), 32, file);
    fseek(file, -32, SEEK_END);
    if (memcmp(magic, "LPFW", 4) != 0) {
        printf("%s maybe is a merged image, please clean and retry\n", argv[1]);
        return EXIT_FAILURE;
    }
    *(unsigned int *)&magic[4] = filesize(argv[2]);
    fwrite(magic, sizeof(magic), 1, file);

    // write lpfw bin
    FILE *inputFile = fopen(argv[2], "rb");
    if (inputFile == NULL) {
        printf("Cannot open input file: %s\n", argv[2]);
        return EXIT_FAILURE;
    }
    char buffer[BUFFER_SIZE];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, inputFile)) > 0) {
        fwrite(buffer, 1, bytesRead, file);
    }
    fclose(inputFile);

    fclose(file);
    return EXIT_SUCCESS;
}
