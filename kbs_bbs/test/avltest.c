/*
 * Test Code for AVL Binary Search Tree - atppp
 * To compile: gcc -g -o avl avltest.c -lm
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifndef _cplusplus

#ifndef byte
typedef unsigned char byte;


#endif                          /*  */
#ifndef HAVE_BOOL
typedef enum { false = 0, true = 1 } bool;


#else                           /*  */
#ifndef true
#define true 1
#endif                          /*  */
#ifndef false
#define false 0
#endif                          /*  */
#endif                          /*  */
#endif                          /*  */



#define AVL_DEBUG 1

// balance is LH (left heavy), EH (even), RH (right heavy)
enum balance { LH, EH, RH };
typedef struct _treenode {
    int info;
    int count;
    int height; //this is for test
    enum balance bf;            // bf is the balance factor
    struct _treenode *Lchild;
    struct _treenode *Rchild;
} treenode;


#ifdef AVL_DEBUG
void Error(int err)
{
    printf("faint %d\n", err);
}
#endif
treenode *AVL_RotateLeft(treenode * p)
{
    treenode *temp;

#ifdef AVL_DEBUG
    if (p == NULL)
        Error(1);
    else if (p->Rchild == NULL)
        Error(2);
    else
#endif
    {
        temp = p->Rchild;
        p->Rchild = temp->Lchild;
        temp->Lchild = p;
        p = temp;
    }
    return p;
}

treenode *AVL_RotateRight(treenode * p)
{
    treenode *temp;

#ifdef AVL_DEBUG
    if (p == NULL)
        Error(3);
    else if (p->Lchild == NULL)
        Error(4);
    else
#endif
    {
        temp = p->Lchild;
        p->Lchild = temp->Rchild;
        temp->Rchild = p;
        p = temp;
    }
    return p;
}

treenode *AVL_RightBalance(treenode * r)
{
    treenode *x;
    treenode *w;

    x = r->Rchild;
    switch (x->bf) {
    case RH:
        r->bf = EH;
        x->bf = EH;
        r = AVL_RotateLeft(r);
        break;

#ifdef AVL_DEBUG
    case EH:
        Error(5);
        break;

#endif
    case LH:
        w = x->Lchild;
        switch (w->bf) {
        case EH:
            r->bf = EH;
            x->bf = EH;
            break;
        case LH:
            r->bf = EH;
            x->bf = RH;
            break;
        case RH:
            r->bf = LH;
            x->bf = EH;
            break;
        }
        w->bf = EH;
        x = AVL_RotateRight(x);
        r->Rchild = x;
        r = AVL_RotateLeft(r);
    }
    return r;
}

treenode *AVL_LeftBalance(treenode * r)
{
    treenode *x;
    treenode *w;

    x = r->Lchild;
    switch (x->bf) {
    case LH:
        r->bf = EH;
        x->bf = EH;
        r = AVL_RotateRight(r);
        break;

#ifdef AVL_DEBUG
    case EH:
        Error(6);
        break;

#endif
    case RH:
        w = x->Rchild;
        switch (w->bf) {
        case EH:
            r->bf = EH;
            x->bf = EH;
            break;
        case RH:
            r->bf = EH;
            x->bf = LH;
            break;
        case LH:
            r->bf = RH;
            x->bf = EH;
            break;
        }
        w->bf = EH;
        x = AVL_RotateLeft(x);
        r->Lchild = x;
        r = AVL_RotateRight(r);
    }
    return r;
}

bool AVL_Insert(treenode ** proot, int newinfo)
{
    bool tallersubtree;
    bool taller;
    treenode *root;

    root = *proot;
    if (root == NULL) {
        root = (treenode *) malloc(sizeof(treenode));
        root->info = newinfo;
        root->count = 1;
        root->Lchild = NULL;
        root->Rchild = NULL;
        root->bf = EH;
        taller = true;
    }

    else if (newinfo < root->info) {
        tallersubtree = AVL_Insert(&(root->Lchild), newinfo);
        if (tallersubtree)
            switch (root->bf) {
            case LH:
                root = AVL_LeftBalance(root);
                taller = false;
                break;
            case EH:
                root->bf = LH;
                taller = true;
                break;
            case RH:
                root->bf = EH;
                taller = false;
                break;
            }

        else
            taller = false;
    }

    else if (newinfo > root->info) {
        tallersubtree = AVL_Insert(&(root->Rchild), newinfo);
        if (tallersubtree)
            switch (root->bf) {
            case LH:
                root->bf = EH;
                taller = false;
                break;
            case EH:
                root->bf = RH;
                taller = true;
                break;
            case RH:
                root = AVL_RightBalance(root);
                taller = false;
            }

        else
            taller = false;
    }

    else {                      // newinfo == root->info
        root->count++;
        taller = false;
    }
    *proot = root;
    return taller;
}                               // end insert

void AVL_InTW(treenode * root, void (*fptr) (treenode *, void *), void *arg)
{                               /* inorder tree walk, note root must not be null */
    if (root->Lchild != NULL)
        AVL_InTW(root->Lchild, fptr, arg);
    if (fptr != NULL)
        fptr(root, arg);
    if (root->Rchild != NULL)
        AVL_InTW(root->Rchild, fptr, arg);
}

void AVL_PostTW(treenode * root, void (*fptr) (treenode *, void *), void *arg)
{                               /* postorder tree walk, note root must not be null */
    if (root->Lchild != NULL)
        AVL_PostTW(root->Lchild, fptr, arg);
    if (root->Rchild != NULL)
        AVL_PostTW(root->Rchild, fptr, arg);
    if (fptr != NULL)
        fptr(root, arg);
}

#define MAX 1000000
#define ROUND 10
int round = 0;

void compare(treenode * node, void *arg)
{
    int **p = (int **)arg;
    **p = node->info;
    if (node->count != round + 1) printf("faint, counter error!!!\n");
    (*p)++;
}

void measureheight(treenode *node, void *arg)
{
	int lh = 0;
	int rh = 0;
	if (node->Lchild != NULL) lh = node->Lchild->height;
	if (node->Rchild != NULL) rh = node->Rchild->height;
	if (abs(lh - rh) > 1) printf("faint, height diff > 1!!!\n");
	if (lh > rh) node->height = lh + 1;
	else node->height = rh + 1;
}

void freenode(treenode * node, void *arg)
{
    free(node);
}

int main(int argc, char **argv)
{
    treenode *root = NULL;
    int i;
    int pp[MAX];
    int *p;
    int height_max = (int)(log(MAX) * 2.07 - 0.33);
    
    srand(time(NULL));
	for (round = 0; round < ROUND; round++) {
	    for (i = 0; i < MAX; i++) {
	    	pp[i] = i;
	    }
	    for (i = 0; i < MAX; i++) {
	    	int j, k, temp;
	    	j = rand() % MAX;
	    	k = rand() % MAX;
	    	temp = pp[j];
	    	pp[j] = pp[k];
	    	pp[k] = temp;
	    }
	
	    for (i = 0; i < MAX; i++) {
	        AVL_Insert(&root, pp[i]);
	    }
	    p = pp;
	    AVL_InTW(root, compare, &p);
	    for (i = 0; i < MAX; i++) {
	    	if (i != pp[i]) printf("faint!!\n");
	    }
	}
    AVL_PostTW(root, measureheight, NULL);
    printf("total height: real: %d, max: %d\n", root->height, height_max);

    AVL_PostTW(root, freenode, NULL);
    return 0;
}
