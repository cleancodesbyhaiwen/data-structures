#include "hbt.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>


#include "headers.h"
// whether the data file is valid
bool valid_file = true;

// build hbt in file, pre-order output it to out
int build_hbt(FILE* in, FILE* out);

// output hbt to out file
void output_hbt(Tnode* node, FILE* out);

// evaluate hbt
int evaluate_hbt(FILE* in);
// evaluate whether is a BST
bool evaluate_bst(Tnode** node, FILE* in);

// evaluate whether is balanced
int evaluate_balance(Tnode* node);

// for debug
// void preorder(Tnode* node);


int main(int argc, char* argv[]) {
    // check command line argument
    if (argc < 2) {
        fprintf(stderr, "Usage:\n"
                        "  ./pa4 -b operations_input_file tree_output_file\n"
                        "  ./pa4 -e tree_input_file\n");
        return 0;
    }

    bool is_build = false;
    // check flag
    if (strcmp(argv[1], "-b") == 0) {
        // check command line for build
        if (argc != 4) {
            fprintf(stderr, "Usage: ./pa4 -b operations_input_file tree_output_file\n");
            return 0;
        }
        // is build
        is_build = true;
    } else if (strcmp(argv[1], "-e") == 0) {
        // check command line for evaluate
        if (argc != 3) {
            fprintf(stderr, "Usage: ./pa4 -e tree_input_file\n");
            return 0;
        }
    } else {  // invalid tag
        fprintf(stderr, "Usage:\n"
                        "  ./pa4 -b operations_input_file tree_output_file\n"
                        "  ./pa4 -e tree_input_file\n");
        return 0;
    }

    int ret;
    if (is_build) {  // build hbt
        // open data file to read
        FILE* in = fopen(argv[2], "rb");
        if (!in) {
            fprintf(stderr, "No such file: %s\n", argv[2]);
            printf("-1\n");
            return 1;
        }

        // open data file to write
        FILE* out = fopen(argv[3], "wb");
        if (!out) {
            fclose(in);
            fprintf(stderr, "Open file: %s failed\n", argv[3]);
            printf("-1\n");
            return 1;
        }

        // build hbt
        ret = build_hbt(in, out);
        // close file
        fclose(in);
        fclose(out);
    } else {  // evaluate hbt
        // open data file to read
        FILE* in = fopen(argv[2], "rb");
        if (!in) {
            fprintf(stderr, "No such file: %s\n", argv[2]);
            printf("-1,0,0\n");
            return 1;
        }

        // evaluate hbt
        ret = evaluate_hbt(in);
        // close file
        fclose(in);
    }

    return ret;
}

int build_hbt(FILE* in, FILE* out) {
    int key;
    char aux;
    Tnode* hbt = NULL;

    // read data one by one
    while (fread(&key, sizeof(int), 1, in) == 1 &&
           fread(&aux, sizeof(char), 1, in) == 1) {
        // fprintf(stderr, "%d: %c\n", data.key, data.aux);
        switch (aux) {  // check operation
            case 'i':
                // insert
                hbt_insert(&hbt, key);
                break;
            case 'd':
                // delete
                hbt = hbt_delete(hbt, key);
                break;
            default:
                // invalid operation
                fprintf(stderr, "Invalid operation: %d: %c\n",
                        key, aux);
        }
    }

    // can't build hbt
    if (!hbt) {
        return 1;
    }

    // output hbt to file
    output_hbt(hbt, out);

    // for debug
    // preorder(hbt);

    // destroy hbt
    hbt_destroy(hbt);

    return 0;
}

void output_hbt(Tnode* node, FILE* out) {
    // get key
    int key = node->key;
    // calculate aux
    char aux = 0;
    if (node->left) {  // has left child
        aux += 2;
    }
    if (node->right) {  // has right child
        aux += 1;
    }

    // write self data
    fwrite(&key, sizeof(key), 1, out);
    fwrite(&aux, sizeof(char), 1, out);

    // travel left
    if (node->left) {
        output_hbt(node->left, out);
    }

    // travel right
    if (node->right) {
        output_hbt(node->right, out);
    }
}

/*
void preorder(Tnode* node) {
    if (!node) {
        return;
    }

    int v = 0;
    if (node->left)
        v += 2;
    if (node->right)
        v += 1;

    printf("%d %d\n", node->key, v);
    preorder(node->left);
    preorder(node->right);
}
*/


int evaluate_hbt(FILE* in) {
    Tnode* root = NULL;
    bool is_bst;
    bool is_balanced;

    // check whether is a BST
    is_bst = evaluate_bst(&root, in);
    // check whether is balanced
    is_balanced = evaluate_balance(root) > 0;

    // print out result
    printf("%d,%d,%d\n", valid_file, is_bst, is_balanced);

    // preorder(root);
    // destroy tree
    hbt_destroy(root);

    // return whether is valid file
    return valid_file ? 0 : 1;
}

bool evaluate_bst(Tnode** node, FILE* in) {
    int key;
    char aux;

    if (fread(&key, sizeof(int), 1, in) != 1 ||
        fread(&aux, sizeof(char), 1, in) != 1) {
        // invalid file
        valid_file = false;
        return false;
    }

    // create a node
    *node = node_create(key);
    // create node failed, something is wrong.
    if (!*node) {
        return false;
    }

    switch (aux) {
        case 0:
            // no left or right, success
            return true;
        case 1:
            // has right, build right and compare key
            return evaluate_bst(&(*node)->right, in) &&
                   ((*node)->key <= (*node)->right->key);
        case 2:
            // has left, build left and compare key
            return evaluate_bst(&(*node)->left, in) &&
                   ((*node)->left->key <= (*node)->key);
        case 3: {
            // has both, build left and right, compare key
            bool left = evaluate_bst(&(*node)->left, in);
            bool right = evaluate_bst(&(*node)->right, in);

            // check build result, then compare key
            return left && right &&
                   ((*node)->left->key <= (*node)->key) &&
                   ((*node)->key <= (*node)->right->key);
        }
        default:
            // invalid aux field
            fprintf(stderr, "Invalid aux in data: %d\n", (int) aux);
            return false;
    }
}

int evaluate_balance(Tnode* node) {
    // null, height is 0
    if (!node) {
        return 0;
    }

    // check left
    int left_height = evaluate_balance(node->left);
    if (left_height < 0) {  // -1 indicate left is not balance
        return -1;
    }

    // check right
    int right_height = evaluate_balance(node->right);
    if (right_height < 0) { // -1 indicate right is not balance
        return -1;
    }

    // now left and right are balance, check root
    int diff = left_height - right_height;
    // check whether root is balance
    if (diff > 1 || diff < -1) {
        // root is not balance
        return -1;
    }
    int max = left_height > right_height ? left_height : right_height; 
    // return tree height
    return max + 1;
}
