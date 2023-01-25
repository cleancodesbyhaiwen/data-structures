#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "struct.h"
#include "headers.h"

int Hwid (int left,int right)
{
    if(left > right)
    {
        return left;
    }else{
        return right;
    }    
}
int Hhei (int left,int right)
{
    return (left + right);
}
int Vwid (int up,int down)
{
    return (up + down);
}
int Vhei (int up,int down)
{
    if(up > down)
    {
        return up;
    }else{
        return down;
    }
}

void findDimension (Tnode * tr)
//上一个是下面两个的值的相加，横着相加或者竖着相加取决于h 或者 v
//故此处用到postorder二叉树算法.
{
    if((tr -> type) == 0)
    {
        return;
    }
    //postorder = ←,→,↑
    findDimension (tr -> left);
    findDimension (tr -> right);

    tr -> item = malloc(sizeof(Node)); //item是Tnode里面的一个普通node
    tr -> item -> tag = tr -> type;

    //所有function, 实参都是(左，右);(上，下)
    if (tr -> type == 'H') 
    {
        tr -> item -> wid = Hwid(tr->left->item->wid, tr->right->item->wid);
        tr -> item -> hei = Hhei(tr->left->item->hei, tr->right->item->hei);
    }
    else if (tr -> type == 'V')
    {
        tr -> item -> wid = Vwid(tr->left->item->wid, tr->right->item->wid);
        tr -> item -> hei = Vhei(tr->left->item->hei, tr->right->item->hei);
    }
    else
    {
        perror("\n findDim Bug \n"); 
    }
}

//用标准坐标系 0,0 为原点,x为横向长度，y为正向长度
//x = hori, y vertical

void findCoordinate(Tnode * tr, int x, int y)
{
    //3((3,3)(0,7))
    //1((5,4)(0,0))
    //2((7,7)(5,0))

    //pdf给的例子
    //我们不需要算出非leaf node的坐标
    //yet, we still need the values for none leaf node for potential leaf.
    //故保留

    tr -> item -> x = x;
    tr -> item -> y = y;
    
    /*
    printf("tag =%d\n",tr -> item -> tag);
    printf("x =%d\n",x);
    printf("y =%d\n",y);
    printf("item x =%d\n",tr -> item -> x);
    printf("item y =%d\n",tr -> item -> y);    
    */

    if(!(tr->type)) //not leaf
    {
        return;
    }

    if(tr->type == 'H')
    {
        findCoordinate(tr->left,x,y + (tr->right->item->hei));
        findCoordinate(tr->right,x,y);
    }
    else if(tr->type == 'V')
    {
        findCoordinate(tr->left,x,y);
        findCoordinate(tr->right,x + (tr -> left -> item -> wid),y);
    }
    else
    {
        perror("\n findCoor Bug \n"); 
    }
}
void printDimension(Tnode *tr, FILE *fptr3)
{   //3(3,3)
    //1(5,4)
    //2(7,7)
    //V(12,7)
    //H(12,10)
    // still postorder
    // leaf node 的 type = 0，之前assign了

    if(tr->type != 0) // not leaf
    {
        printDimension(tr->left, fptr3);
        printDimension(tr->right,fptr3);
        fprintf(fptr3, "%c(%d,%d)\n", tr->type, tr->item->wid, tr->item->hei);
    }
    else // leaf node
    {
        fprintf(fptr3, "%d(%d,%d)\n", tr->item->tag, tr->item->wid, tr->item->hei);
    }
}
void printCoordinate(Tnode * tr, FILE *fptr4)
{
    if(tr->type != 0) // not leaf
    {//not leaf node go next recursion
    /*
        printDimension(tr->left, fptr3);
        printDimension(tr->right,fptr3);
        fprintf(fptr3, "%c(%d,%d)\n", tr->type, tr->item->wid, tr->item->hei);
    */
    printCoordinate(tr -> left, fptr4);
    printCoordinate(tr -> right, fptr4);
    }
    else // leaf node
    {
        //fprintf(fptr3, "%d(%d,%d)\n", tr->item->tag, tr->item->wid, tr->item->hei);
        fprintf(fptr4,"%d((%d,%d)(%d,%d))\n",tr->item->tag,tr->item->wid,tr->item->hei,tr->item->x,tr->item->y);    
    }
}
void wildFire(Tnode *tr)
{
    if(tr == NULL)
    {
        return;
    }
    if(tr -> item != NULL)
    {
        free(tr -> item);
    }
    wildFire(tr -> left);
    wildFire(tr -> right);

    free(tr);
}