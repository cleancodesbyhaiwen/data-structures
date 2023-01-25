#ifndef structure_
#define structure_

typedef struct node_{
    int wid;
    int hei;
    int x;
    int y;
    int tag;
}Node;

typedef struct binary_tree_{
    Node *item;
    char type; //h/v
    struct binary_tree_ *left;
    struct binary_tree_ *right;
}Tnode;

typedef struct list_tree_pot_{
    Tnode * troot;
    struct list_tree_pot_ *next;
}Tpot;

#endif
