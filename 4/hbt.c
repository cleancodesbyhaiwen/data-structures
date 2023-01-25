#include "hbt.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "headers.h"

// counter clock rotate
static Tnode* hbt_ccr(Tnode* old_root);

// clock rotate
static Tnode* hbt_cr(Tnode* old_root);

// helper function for hbt delete
static Tnode* delete(Tnode* node, Tnode* parent, bool is_left, int key);

// helper function for delete, delete the immediate predecessor
static Tnode* delete_predecessor(Tnode* node, Tnode* parent, bool is_left, int* key);

// balance the node
static Tnode* balance(Tnode* node, Tnode* parent, bool is_left);

// update parent' balance
static void update_parent(Tnode* parent, bool is_left);

Tnode* node_create(int key) {
    // malloc a node
    Tnode* node = calloc(1, sizeof(Tnode));
    if (!node) {
        perror("malloc failed");
        return NULL;
    }
    // set key
    node->key = key;
    // return node
    return node;
}

void hbt_insert(Tnode** root, int key) {
    Tnode* ya;      // youngest ancestor with balance 1/-1
    Tnode* pya;     // parent of youngest ancestor
    Tnode* curr;    // current node
    Tnode* pcurr;   // parent of current node
    Tnode* q;       // the created node for key

    ya = curr = *root;
    pya = pcurr = NULL;
    while (curr) {
        // allow duplicate keys, insert to left
        if (key <= curr->key) {
            q = curr->left;
        } else {
            q = curr->right;
        }

        // keep track of youngest ancestor and its parent
        if (q && q->balance != 0) {
            pya = curr;
            ya = q;
        }

        // check next level node
        pcurr = curr;
        curr = q;
    }

    // create a new node
    q = node_create(key);
    // create node failed, something is wrong.
    if (!q) {
        return;
    }

    // root is NULL, return node
    if (!pcurr) {
        *root = q;
        return;
    }

    // insert node to tree
    if (key <= pcurr->key) {
        pcurr->left = q;
    } else {
        pcurr->right = q;
    }
    // insertion done, The following code is for balancing

    // update the balance from youngest ancestor to
    // parent of the new node
    curr = ya;
    while (curr != q) {
        if (key <= curr->key) {  // insert on left
            ++curr->balance;
            curr = curr->left;
        } else {  // insert on right
            --curr->balance;
            curr = curr->right;
        }
    }

    // check if balancing is required
    // balance of ya = -1, 0, or 1, no balancing needed
    if ((ya->balance < 2) && (ya->balance > -2)) {
        return;
    }

    // find the child into which q is inserted
    Tnode* child;
    if (key <= ya->key) {
        child = ya->left;
    } else {
        child = ya->right;
    }
    // the subtree rooted at ya needs balancing
    // curr points to the new root of the subtree
    // pya has to point to curr after re-balancing
    // both ya and child are unbalanced in the same
    // direction
    if ((ya->balance == 2) && (child->balance == 1)) {
        // case 1: both left
        curr = hbt_cr(ya);  // clockwise rotation
        ya->balance = 0;
        child->balance = 0;
    } else if ((ya->balance == -2) && (child->balance == -1)) {
        // case 3: both right
        curr = hbt_ccr(ya);  // counter-clockwise rotation
        ya->balance = 0;
        child->balance = 0;
    } else if ((ya->balance == 2) && (child->balance == -1)) {
        // case 2: ya and child are unbalanced in opp. directions
        ya->left = hbt_ccr(child);  // counter-clockwise rotation
        curr = hbt_cr(ya);  // clockwise rotation

        // update balances
        if (curr->balance == 0) {
            // case 2a: curr is inserted node q
            ya->balance = 0;
            child->balance = 0;
        } else {
            if (curr->balance == 1) {
                // case 2b: left subtree of curr contains q
                ya->balance = -1;
                child->balance = 0;
            } else {
                // case 2c: right subtree of curr contains q
                ya->balance = 0;
                child->balance = 1;
            }
            // new root is balanced
            curr->balance = 0;
        }
    } else if ((ya->balance == -2) && (child->balance == 1)) {
        // case 4: ya and child are unbalanced in opp. directions
        ya->right = hbt_cr(child);  // clockwise rotation
        curr = hbt_ccr(ya);  // counter-clockwise rotation
        // update balances
        if (curr->balance == 0) {
            // case 4a: curr is inserted node q
            ya->balance = 0;
            child->balance = 0;
        } else {
            if (curr->balance == -1) {
                // case 4b: right subtree of curr contains q
                ya->balance = 1;
                child->balance = 0;
            } else {
                // case 4c: left subtree of curr contains q
                ya->balance = 0;
                child->balance = -1;
            }

            curr->balance = 0;  // new root is balanced
        }
    }

    // no pya, curr is new root
    if (!pya) {
        *root = curr;
        return;
    }

    // set curr to pya
    if (key <= pya->key) {
        pya->left = curr;
    } else {
        pya->right = curr;
    }
}

Tnode* hbt_delete(Tnode* root, int key) {
    // delete from root, the parent of root is NULL.
    // the is_left means whether the node is left of parent,
    // it has no effect when parent is NULL
    return delete(root, NULL, true, key);
}


Tnode* delete(Tnode* node, Tnode* parent, bool is_left, int key) {
    // nothing to do for NULL
    if (!node) {
        return NULL;
    }

    // find the key
    if (key < node->key) {  // delete in left
        // record old balance
        int balance = node->balance;
        // delete in left
        node->left = delete(node->left, node, true, key);

        // node is shorter, update parent
        if (balance != 0 && node->balance == 0) {
            update_parent(parent, is_left);
        }
    } else if (key > node->key) {  // delete in right
        // record old balance
        int balance = node->balance;
        // delete in right
        node->right = delete(node->right, node, false, key);

        // node is shorter, update parent
        if (balance != 0 && node->balance == 0) {
            update_parent(parent, is_left);
        }
    } else {  // found the key
        // check left and right of node
        if (!node->left && !node->right) {  // node is a leaf
            // free node
            free(node);
            // set node to NULL
            node = NULL;

            // height decrease, update parent
            update_parent(parent, is_left);
        } else if (node->left && !node->right) {  // node has only left leaf
            // node is balance now
            node->balance = 0;
            // use the left key
            node->key = node->left->key;
            // free left node
            free(node->left);
            node->left = NULL;

            // height decrease, update parent
            update_parent(parent, is_left);
        } else if (!node->left && node->right) {  // node has only right leaf
            // node is balance now
            node->balance = 0;
            // use the right key
            node->key = node->right->key;
            // free right node
            free(node->right);
            node->right = NULL;

            // height decrease, update parent
            update_parent(parent, is_left);
        } else {  // node has two subtree
            // record old balance
            int balance = node->balance;

            // delete immediate predecessor from left
            int new_key;
            node->left = delete_predecessor(node->left, node, true, &new_key);
            // use the predecessor's key
            node->key = new_key;

            // node is shorter, update parent
            if (balance != 0 && node->balance == 0) {
                update_parent(parent, is_left);
            }
        }
    }

    // nothing to balance is node is NULL
    if (!node) {
        return NULL;
    }

    // balance node
    node = balance(node, parent, is_left);

    // return node
    return node;
}

Tnode* hbt_ccr(Tnode* old_root) {
    Tnode* new_root = old_root->right;
    old_root->right = new_root->left;
    new_root->left = old_root;
    return new_root;
}

Tnode* hbt_cr(Tnode* old_root) {
    Tnode* new_root = old_root->left;
    old_root->left = new_root->right;
    new_root->right = old_root;
    return new_root;
}

void hbt_destroy(Tnode* root) {
    // nothing to do for NULL
    if (!root) {
        return;
    }

    // destroy left and right
    hbt_destroy(root->left);
    hbt_destroy(root->right);

    // free node
    free(root);
}

void update_parent(Tnode* parent, bool is_left) {
    // nothing to do when parent is NULL
    if (!parent) {
        return;
    }

    // node is parent's left, so decrease balance
    if (is_left) {
        // left subtree of parent is shorter
        --parent->balance;
    } else {  // node is parent's right, so increase balance
        // right subtree of parent is shorter
        ++parent->balance;
    }
}

Tnode* delete_predecessor(Tnode* node, Tnode* parent,
                          bool is_left, int* key) {
    // find the rightmost node
    if (node->right) {
        // record old balance
        int balance = node->balance;
        // delete rightmost in right
        node->right = delete_predecessor(node->right, node, false, key);

        // node is shorter, update parent
        if (balance != 0 && node->balance == 0) {
            update_parent(parent, is_left);
        }
    } else {  // found the node
        // record node's key
        *key = node->key;

        // check whether node has left
        if (node->left) {
            // node is balance now
            node->balance = 0;
            // use the left key
            node->key = node->left->key;
            // free left node
            free(node->left);
            node->left = NULL;
        } else {  // no left
            // node is NULL now
            free(node);
            node = NULL;
        }

        // height decrease, update parent
        update_parent(parent, is_left);
    }

    // nothing to balance for NULL
    if (!node) {
        return NULL;
    }

    // balance node
    node = balance(node, parent, is_left);

    // return node
    return node;
}

static Tnode* balance(Tnode* node, Tnode* parent, bool is_left) {
    // check node balance;
    int balance = node->balance;
    // check whether the node is already balanced
    if ((balance < 2) && (balance > -2)) {
        return node;
    }

    // case 1: left left
    Tnode* child = node->left;
    if (balance == 2 && child->balance == 1) {
        node->balance = 0;
        child->balance = 0;
        // right rotate
        node = hbt_cr(node);
        // update parent's balance
        update_parent(parent, is_left);
    }

    // case 2: left right
    child = node->left;
    if (balance == 2 && child->balance == -1) {
        node->balance = 0;
        child->balance = 0;
        // left rotate
        node->left = hbt_ccr(node->left);
        // right rotate
        node = hbt_cr(node);
        // update parent's balance
        update_parent(parent, is_left);
    }

    // case 3: right right
    child = node->right;
    if (balance == -2 && child->balance == -1) {
        node->balance = 0;
        child->balance = 0;
        // right rotate
        node = hbt_ccr(node);
        // update parent's balance
        update_parent(parent, is_left);
    }

    // case 4: right left
    child = node->right;
    if (balance == -2 && child->balance == 1) {
        node->balance = 0;
        child->balance = 0;
        // right rotate
        node->right = hbt_cr(node->right);
        // left rotate
        node = hbt_ccr(node);
        // update parent's balance
        update_parent(parent, is_left);
    }

    return node;
}
