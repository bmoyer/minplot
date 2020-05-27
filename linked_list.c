#include <stdlib.h>
#include <stdio.h>
#include "linked_list.h"

node* create_node(void* val) {
    node* n = malloc(sizeof(node));
    n->val = val;
    n->next = NULL;
    return n;
}

void append(linked_list* list, void* val) {
    node* new_tail = create_node(val);

    if(list->tail != NULL) {
        list->tail->next = new_tail;
    }

    list->tail = new_tail;

    if(list->head == NULL) {
        list->head = list->tail;
    }
}

void delete_list(linked_list* list) {
    node* pn = list->head;
    while(pn != NULL) {
        node* tmp = pn->next;
        free(pn->val);
        free(pn);
        pn = tmp;
    }
    list->head = NULL;
    list->tail = NULL;
}

void print_list(linked_list* list) {
    node* n = list->head;
    while(n != NULL) {
        printf("Node val: %d\n", *((int*)n->val));
        n = n->next;
    }
}

int list_length(linked_list* list) {
    int len = 0;
    node* n = list->head;
    while(n != NULL) {
        len++;
        n = n->next;
    }
    return len;
}

