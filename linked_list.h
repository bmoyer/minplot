#ifndef LINKED_LIST_H
#define LINKED_LIST_H 

typedef struct node {
    void* val;
    struct node* next;
} node;

typedef struct linked_list {
    node* head;
    node* tail;
} linked_list;

node* create_node(void* val);
void append(linked_list* list, void* val);
void delete_list(linked_list* list);
void print_list(linked_list* list);
int list_length(linked_list* list);

#endif /* LINKED_LIST_H */
