#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "struct.h"
#include "headers.h"

void cutTr (Tnode **tr, Tpot **pot)
{
    if(pot == NULL || *pot == NULL)
    {
        return;
    }
    Tpot *temp = *pot;
    *tr = (*pot) ->troot;
    *pot = (*pot)-> next;
    free(temp);
}
void plantTr (Tnode *tr, Tpot **pot)
{
    if(pot == NULL)
    {
        return;
    }
    Tpot *temp = *pot; 
    (*pot) = malloc(sizeof(**pot));
    (*pot)->troot = tr;
    (*pot)->next = temp;
}
Tnode * makeTree (char c, Node * item, Tnode* left, Tnode* right)
{
    Tnode * new_tree = malloc(sizeof(Tnode));
    new_tree -> type = c;
    new_tree -> item = item;
    new_tree -> left = left;
    new_tree -> right = right;
    return new_tree;
}
void file_to_tnode (FILE *fptr1, Tnode ** tree_node)
{   
    char temp [60];
    Tnode * right = NULL;
    Tnode * left = NULL;    
    Tpot *treePot = NULL;

    while (fscanf(fptr1,"%s",temp)!= EOF) // cannot be used with fgetc/s together
    {
        if(strlen(temp) != 1) // leaf node
        {
            Node *item = malloc(sizeof(*item));
            sscanf(temp, "%d(%d,%d)", &(item->tag), &(item->wid) ,&(item->hei));
            Tnode * newTree = makeTree(0, item, NULL, NULL);
            plantTr(newTree,&treePot);
        }
        else // h / v
        {
            cutTr(&right,&treePot);
            cutTr(&left,&treePot);
            Tnode * newTree = makeTree(temp[0],NULL,left,right);
            plantTr(newTree,&treePot);
        }
    }
    cutTr(tree_node, &treePot); // this way we free the last tree pot
}

void printPreorder(FILE *fptr2, Tnode *treeNode)
{   
    if(treeNode == NULL) // 理论上不会发生，因为之前就已经把type赋值为0或者其他的数
    {
        return;
    }
    if(treeNode -> type) // not a leaf node, we print.
    {// and keep recursion
        fprintf(fptr2, "%c\n", treeNode -> type); 
        printPreorder(fptr2, treeNode->left);
        printPreorder(fptr2, treeNode->right);
    }else{ // else it is a leafnode
        fprintf(fptr2, "%d(%d,%d)\n", treeNode->item->tag, treeNode->item->wid, treeNode->item->hei);
    }    
}
int main (int argc, char ** argv)
{
    if(argc != 5){ // there needs to be 4 arguments 
        return EXIT_FAILURE;
    }
    Tnode * tr = NULL;
    FILE *fptr1 = fopen(argv[1], "r");
    file_to_tnode (fptr1, &tr);
    fclose(fptr1);
    //
    FILE *fptr2 = fopen(argv[2], "w+");
    printPreorder(fptr2, tr);
    fclose(fptr2);
    //
    findDimension(tr);
    findCoordinate(tr,0,0); //bottom left corner.

    // By now all the necessary values for Dimen & coord should be filled in

    FILE *fptr3 = fopen(argv[3], "w+");
    printDimension(tr,fptr3);
    fclose(fptr3);

    FILE *fptr4 = fopen(argv[4], "w+");
    printCoordinate(tr,fptr4);
    fclose(fptr4);

    wildFire(tr);

    return EXIT_SUCCESS;
}