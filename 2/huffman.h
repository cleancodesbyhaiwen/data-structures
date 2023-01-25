#ifndef HUFFMAN_H
#define HUFFMAN_H 

#include "bits.h"

#include <stdio.h>

typedef struct HuffmanNode {
    unsigned char value;
    struct HuffmanNode* left;
    struct HuffmanNode* right;
} HuffmanNode;

char* binary_to_string_huffman(bit_reader* reader, int num_tree_bytes, int *size);

void pre_order_huffman_tree(HuffmanNode* root, FILE* fp);
HuffmanNode *string_to_huffman_tree(char* tree_str, int size);
unsigned char decode_huffman_tree(HuffmanNode *root, bit_reader* reader, long* num_bits);
HuffmanNode *build_huffman_with_frequency(int freqs[], unsigned char chs[], int size);

void get_num_encoded_bits(HuffmanNode* root, long count[]);

#endif /* ifndef HUFFMAN_H */
