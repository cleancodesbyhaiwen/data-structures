#ifndef BITS_H
#define BITS_H 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** For reading */
typedef struct {
    FILE* file;
    unsigned char buffer;
    unsigned int index;
    unsigned int consume_bytes;
} bit_reader;

/** For writing */
typedef struct {
    FILE* file;
    unsigned char buffer;
    unsigned int index;
} bit_writer;

bit_reader* open_bit_reader(char const* filename);
void close_bit_reader(bit_reader* reader);
int get_bit(bit_reader* reader);
unsigned char get_byte(bit_reader* reader);
void fflush_reader(bit_reader* reader);

#endif /* ifndef BITS_H */
