#include <stdlib.h>
#include <stdio.h>
#include "linked_list.h"

node* create_node(void* val) {
    node* n = malloc(sizeof(node));
    n->val = val;
    n->next = NULL;
    return n;
}

void append(node** head, node** tail, void* val) {
    node* new_tail = create_node(val);

    if(*tail != NULL) {
        (*tail)->next = new_tail;
    }

    *tail = new_tail;

    if(*head == NULL) {
        *head = *tail;
    }
}

void delete_list(node** head, node** tail) {
    node* pn = *head;
    while(pn != NULL) {
        node* tmp = pn->next;
        free(pn->val);
        free(pn);
        pn = tmp;
    }
    *head = NULL;
    *tail = NULL;
}

void print_list(node* n) {
    while(n != NULL) {
        printf("Node val: %d\n", *((int*)n->val));
        n = n->next;
    }
}

int list_length(node* n) {
    int len = 0;
    while(n != NULL) {
        len++;
        n = n->next;
    }
    return len;
}

