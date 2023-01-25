#include "bits.h"
#include "huffman.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ASCII_SIZE 256

int main(int argc, char *argv[]) {
    if (argc != 7) {
        fprintf(stderr, "Usage: %s encoded tree_file original_file count_file htree eval\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int i = 0;

    bit_reader* reader = open_bit_reader(argv[1]);

    FILE* tree_file = fopen(argv[2], "w");
    if (!tree_file) {
        fprintf(stderr, "Invalid tree file: %s\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    FILE* original_file = fopen(argv[3], "w");
    if (!original_file) {
        fprintf(stderr, "Invalid original file: %s\n", argv[3]);
        exit(EXIT_FAILURE);
    }

    FILE* count_file = fopen(argv[4], "w");
    if (!count_file) {
        fprintf(stderr, "Invalid count file: %s\n", argv[4]);
        exit(EXIT_FAILURE);
    }

    FILE* htree_file = fopen(argv[5], "w");
    if (!htree_file) {
        fprintf(stderr, "Invalid htree file: %s\n", argv[5]);
        exit(EXIT_FAILURE);
    }

    FILE* eval_file = fopen(argv[6], "w");
    if (!htree_file) {
        fprintf(stderr, "Invalid eval file: %s\n", argv[6]);
        exit(EXIT_FAILURE);
    }

    long num_file, num_huffman_bytes;
    long num_original_bytes;
    fread(&num_file, sizeof(long), 1, reader->file);
    fread(&num_huffman_bytes, sizeof(long), 1, reader->file);
    fread(&num_original_bytes, sizeof(long), 1, reader->file);

    // reconstruct huffman tree from encoded file
    int size = 0;
    char *tree_str = binary_to_string_huffman(reader, num_huffman_bytes, &size);
    HuffmanNode *root = string_to_huffman_tree(tree_str, size);
    fwrite(tree_str, sizeof(char), size, tree_file);
    fclose(tree_file);

    long count[ASCII_SIZE];
    memset(count, 0, ASCII_SIZE * sizeof(long));

    long first_num_bits = 0;
    // decode encoded file with huffman tree
    for (i = 0; i < num_original_bytes; ++i) {
        unsigned char ch = decode_huffman_tree(root, reader, &first_num_bits);
        fwrite(&ch, sizeof(unsigned char), 1, original_file);
        /* printf("%c", ch); */
        count[ch] += 1;
    }
    /* printf("\n"); */
    fclose(original_file);

    // write count to file
    fwrite(count, sizeof(long), ASCII_SIZE, count_file);
    fclose(count_file);

    // get (frequency, char) from original file
    int freq[ASCII_SIZE];
    unsigned char chs[ASCII_SIZE];
    int num_chars = 0;
    for (i = 0; i < ASCII_SIZE; ++i) {
        if (count[i] > 0) {
            freq[num_chars] = count[i];
            chs[num_chars] = i;
            num_chars += 1;
        }
    }
    // for (i = 0; i < num_chars; ++i) {
    //     printf("%c %d\n", chs[i], freq[i]);
    // }

    // use (frequency, count) build huffman tree
    HuffmanNode* nroot = build_huffman_with_frequency(freq, chs, num_chars);
    pre_order_huffman_tree(nroot, htree_file);
    fclose(htree_file);

    /* pre_order_huffman_tree(nroot, stdout); */
    /* printf("\n"); */

    memset(count, 0, ASCII_SIZE * sizeof(long));
    get_num_encoded_bits(nroot, count);
    long second_num_bits = 0;
    for (i = 0; i < num_chars; ++i) {
        second_num_bits += (freq[i] * count[chs[i]]);
    }

    long first_num_bytes = first_num_bits / 8;
    int first_left_bits = first_num_bits % 8;
    long second_num_bytes = second_num_bits / 8;
    int second_left_bits = second_num_bits % 8;
    fwrite(&first_num_bytes, sizeof(long), 1, eval_file);
    fwrite(&first_left_bits, sizeof(int), 1, eval_file);
    fwrite(&second_num_bytes, sizeof(long), 1, eval_file);
    fwrite(&second_left_bits, sizeof(int), 1, eval_file);
    fclose(eval_file);

    return 0;
}
