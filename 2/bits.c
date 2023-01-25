#include "bits.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bit_reader* open_bit_reader(char const* filename) {
    FILE* inputFile = fopen(filename, "rb");
    if (inputFile == NULL) {
        perror(filename);
        exit(EXIT_FAILURE);
    }
    bit_reader* reader = (bit_reader*)malloc(sizeof(bit_reader));
    reader->buffer = 0;
    reader->index = 8;
    reader->file = inputFile;
    reader->consume_bytes = 0;
    return reader;
}

void closebit_reader(bit_reader* reader) {
    fclose(reader->file);
    free(reader);
}

int get_bit(bit_reader* reader) {
    if (reader->index == 8) {
        if (fread(&reader->buffer, 1, 1, reader->file) != 1) {
            return EOF;
        }
        reader->index = 0;
    }
    int ret = (reader->buffer & (1 << reader->index));
    reader->index += 1;
    if (reader->index == 8) {
        /* printf("P%0xP\n", reader->buffer); */
        reader->consume_bytes += 1;
    }
    return ret > 0;
}

unsigned char get_byte(bit_reader* reader) {
    int i = 0;
    unsigned char ret = 0;
    while (i < 8) {
        int v = get_bit(reader);
        if (v == EOF) {
            fprintf(stderr, "reader get_byte error\n");
            exit(EXIT_FAILURE);
        }
        if (v) {
            /* printf("1"); */
            ret |= (1 << i);
        } else {
            /* printf("0"); */
        }
        i += 1;
    }
    return ret;
}

void fflush_reader(bit_reader* reader) {
    reader->index = 8;
    reader->consume_bytes += 1;
}
