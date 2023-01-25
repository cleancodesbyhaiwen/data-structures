#include "shell_list.h"

#include <stdio.h>
#include <stdlib.h>

static Node *new_node(long value) {
    Node *node = malloc(sizeof(Node));
    node->value = value;
    node->next = NULL;
    return node;
}

Node *List_Load_From_File(char *filename) {
    Node* head = NULL;
    Node* tail = NULL;

    FILE* fp = fopen(filename, "r");
    if (!fp) {
        return NULL;
    }

    while (1) {
        long value;
        if (fread(&value, sizeof(long), 1, fp) == 1) {
            Node *node = new_node(value);
            if (head == NULL) {
                head = node;
            } else {
                tail->next = node;
            }
            tail = node;
        } else {
            break;
        }
    }

    fclose(fp);

    return head;
}

int List_Save_To_File(char *filename, Node *list) {
    if (!list) {
        return 0;
    }
    int size = 0;

    FILE *fp = fopen(filename, "w");
    while (list != NULL) {
        /* printf("%ld\n", list->value); */
        int num = fwrite(&list->value, sizeof(long), 1, fp);
        size += num;
        list = list->next;
    }
    fclose(fp);
    return size;
}

static int size_list(Node *list) {
    int size = 0;
    while (list) {
        list = list->next;
        size += 1;
    }
    return size;
}

/* static Node *prev_node(Node *head, Node *node) { */
/*     if (head == node) { */
/*         return NULL; */
/*     } */
/*     while (head && head->next != node) { */
/*         head = head->next; */
/*     } */
/*     return head; */
/* } */

static Node *swap_node(Node *head, Node *first, Node *first_prev, Node *second, Node* second_prev) {
    if (first->next == second) {
        if (first_prev == NULL) {
            head = second;
        } else {
            first_prev->next = second;
        }
        first->next = second->next;
        second->next = first;
    } else {
        if (first_prev == NULL) {
            head = second;
        } else {
            first_prev->next = second;
        }
        Node *first_next = first->next;
        Node *second_next = second->next;
        first->next = second_next;
        second->next = first_next;
        second_prev->next = first;
    }
    return head;
}

static Node *advance(Node *head, int num, Node **prev) {
    *prev = NULL;
    while (num > 0 && head) {
        num -= 1;
        *prev = head;
        head = head->next;
    }
    return head;
}

/* static void debug_print(Node* list) { */
/*     int i = 0; */
/*     while (list) { */
/*         printf("%d: %ld\n", i, list->value); */
/*         list = list->next; */
/*         i ++; */
/*     } */
/*     printf("-----\n"); */
/* } */


Node *List_Shellsort_Bubble(Node *list, int size, int k, long *n_comp) {
    Node *inode_prev = NULL;
    Node *inode = NULL;
    Node *lnode_prev = NULL;
    Node *lnode = NULL;
    Node *head = list;
    Node *head_prev = NULL;
    int i, j, l;
    int len = size / k;
    /* printf("len = %d %d %d\n", len, size, k); */
    /* int left = size % k; */
    for (i = 0; i < k; ++i) {
        for (j = 0; j < len; ++j) {
            inode = head;
            inode_prev = head_prev;
            /* printf("start %d\n", size - (j + 1) * k); */
            for (l = i; l < size - (j + 1) * k; l += k) {
                lnode_prev = NULL;
                lnode = advance(inode, k, &lnode_prev);
                if (inode->value > lnode->value) {
                    *n_comp += 1;
                    if (inode == head) {
                        head = lnode;
                    }

                    list = swap_node(list, inode, inode_prev, lnode, lnode_prev);
                    if (k == 1) {
                        inode_prev = lnode;
                    } else {
                        inode_prev = lnode_prev;
                    }
                } else {
                    inode_prev = lnode_prev;
                    inode = lnode;
                }
                
            }
        }
        inode = advance(head, i, &head_prev);
    }
    return list;
}

Node *List_Shellsort(Node *list, long *n_comp) {
    int size = size_list(list);
    int maxk = 1;
    while ((maxk * 3 + 1) < size) {
        maxk = maxk * 3 + 1;
    }

    *n_comp = 0;

    int k = 0;
    /* debug_print(list); */
    for (k = maxk; k >= 1; k = (k - 1) / 3) {
        list = List_Shellsort_Bubble(list, size, k, n_comp);
        /* printf("%d\n", k); */
        /* debug_print(list); */
    }
    return list;
}
