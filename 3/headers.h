#ifndef headers_
#define headers_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "struct.h"

//void file_to_tnode (FILE *fptr1, Tnode ** tree_node);
Tnode * makeTree (char c, Node * item, Tnode* left, Tnode* right);
void plantTr (Tnode *tr, Tpot **pot);

int Hwid (int left,int right);
int Hhei (int left, int right);
int Vwid (int up, int down);
int Vhei (int up, int down);

void findDimension (Tnode * tr);
void findCoordinate(Tnode * tr, int x, int y);

void printDimension(Tnode *tr, FILE *fptr3);
void printCoordinate(Tnode *tr, FILE *fptr4);

void wildFire(Tnode *tr);
void file_to_tnode (FILE *fptr1, Tnode ** tree_node);

#endif