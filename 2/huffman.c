#include "huffman.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

static bool isleaf(HuffmanNode *node) {
    return !node->left && !node->right;
}

char* binary_to_string_huffman(bit_reader* reader, int num_tree_bytes, int *size) {
    char* tree = malloc(num_tree_bytes * 8 * sizeof(char));
    int n = 0;

    while (reader->consume_bytes < num_tree_bytes) {
        if (get_bit(reader)) {
            tree[n++] = '1';
            tree[n++] = get_byte(reader);
        } else {
            tree[n++] = '0';
        }
    }

    while (n > 0 && tree[n-1] == '0') {
        n -= 1;
    }

    /* tree[n] = '\0'; */
    /* printf("%s\n", tree); */
    
    *size = n;
    return tree;
}

static HuffmanNode *new_huffman_node(const char ch) {
    HuffmanNode *node = malloc(sizeof(HuffmanNode));
    node->value = ch;
    node->left = NULL;
    node->right = NULL;
    return node;
}

static HuffmanNode *string_to_huffman_tree_helper(char* tree_str, int size, int *pos) {
    if (*pos >= size) {
        return NULL;
    }

    HuffmanNode *root = NULL;
    /* printf("%c\n", tree_str[*pos]); */
    if (tree_str[*pos] == '0') {
        *pos += 1;
        root = new_huffman_node('\0');
        root->left = string_to_huffman_tree_helper(tree_str, size, pos);
        root->right = string_to_huffman_tree_helper(tree_str, size, pos);
    } else {
        *pos += 1;
        root = new_huffman_node(tree_str[*pos]);
        *pos += 1;
    }
    return root;
}

void pre_order_huffman_tree(HuffmanNode* root, FILE* fp) {
    if (!root) {
        return;
    }
    if (isleaf(root)) {
        fwrite("1", 1, 1, fp);
        fwrite(&root->value, 1, 1, fp);
        return;
    }

    fwrite("0", 1, 1, fp);
    pre_order_huffman_tree(root->left, fp);
    pre_order_huffman_tree(root->right, fp);
}

HuffmanNode *string_to_huffman_tree(char* tree_str, int size) {
    int pos = 0;
    HuffmanNode *root = string_to_huffman_tree_helper(tree_str, size, &pos);
    return root;
}

unsigned char decode_huffman_tree(
    HuffmanNode *root, bit_reader* reader, long* num_bits) {

    HuffmanNode* curr = root;
    while (curr) {
        int v = get_bit(reader);
        if (v == EOF) {
            fprintf(stderr, "Decode Error");
            exit(EXIT_FAILURE);
        }

        *num_bits += 1;

        if (v == 0) {
            curr = curr->left;
        } else {
            curr = curr->right;
        }
        
        if (isleaf(curr)) {
            return curr->value;
        }
    }

    fprintf(stderr, "Decode Error");
    exit(EXIT_FAILURE);
}

static void get_num_encoded_bits_helper(
        HuffmanNode *root, long count[], int num_bits) {
    if (!root) {
        return;
    }

    if (isleaf(root)) {
        count[root->value] = num_bits;
        return;
    }
    get_num_encoded_bits_helper(root->left, count, num_bits + 1);
    get_num_encoded_bits_helper(root->right, count, num_bits + 1);
}

void get_num_encoded_bits(HuffmanNode* root, long count[]) {
    get_num_encoded_bits_helper(root, count, 0);
}

typedef struct {
    HuffmanNode* node;
    int create_time;
    int freq;
} PriorityNode;

static PriorityNode* new_priority_node(HuffmanNode* hnode, int ct, int freq) {
    PriorityNode* node = malloc(sizeof(PriorityNode));
    node->node = hnode;
    node->create_time = ct;
    node->freq = freq;
    return node;
}

static int find_min_priority_node(
        PriorityNode** pnodes, int size,
        int exclude) {
    PriorityNode* min = NULL;
    int index = -1;
    int i = 0;
    for (i = 0; i < size; ++i) {
        if (i == exclude || pnodes[i] == NULL) {
            continue;
        }
        if (min == NULL) {
            min = pnodes[i];
            index = i;
        } else if (min->freq > pnodes[i]->freq) {
            min = pnodes[i];
            index = i;
        } else if (min->freq == pnodes[i]->freq) {
            bool minleaf = isleaf(min->node);
            bool pnodeleaf = isleaf(pnodes[i]->node);
            if (minleaf && pnodeleaf) {
                if (min->node->value > pnodes[i]->node->value) {
                    min = pnodes[i];
                    index = i;
                }
            } else if (!minleaf && pnodeleaf) {
                min = pnodes[i];
                index = i;
            } else if (!minleaf && !pnodeleaf) {
                if (min->create_time > pnodes[i]->create_time) {
                    min = pnodes[i];
                    index = i;
                }
            }
        }
    }
    return index;
}

HuffmanNode *build_huffman_with_frequency(int freqs[], unsigned char chs[], int size) {
    int create_time = 0;
    PriorityNode** pnodes = malloc(size * sizeof(PriorityNode*));
    int i = 0;
    for (i = 0; i < size; ++i) {
        pnodes[i] = new_priority_node(
                new_huffman_node(chs[i]), create_time, freqs[i]);
        create_time++;
    }

    int num = size;
    PriorityNode* proot = NULL;

    while (num > 1) {
        int i1 = find_min_priority_node(pnodes, size, -1);
        int i2 = find_min_priority_node(pnodes, size, i1);
        PriorityNode *t1 = pnodes[i1];
        PriorityNode *t2 = pnodes[i2];
        HuffmanNode* n = new_huffman_node('\0');
        n->left = t1->node;
        n->right = t2->node;

        pnodes[i2] = NULL;
        pnodes[i1] = new_priority_node(n, create_time, t1->freq + t2->freq);
        create_time++;

        free(t1);
        free(t2);
        num -= 1;
        if (num == 1) {
            proot = pnodes[i1];
        }
    }

    HuffmanNode* root = proot->node;
    free(proot);
    free(pnodes);

    return root;
}
