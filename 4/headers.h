#ifndef __Tree__
#define __Tree__

#include "hbt.h"

// create a node
Tnode* node_create(int key);
// insert key to hbt
void hbt_insert(Tnode** root, int key);
// delete key from hbt
Tnode* hbt_delete(Tnode* root, int key);
// destroy hbt
void hbt_destroy(Tnode* root);


#endif
