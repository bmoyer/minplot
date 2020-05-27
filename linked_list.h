#ifndef LINKED_LIST_H
#define LINKED_LIST_H 

typedef struct node {
    void* val;
    struct node* next;
} node;

node* create_node(void* val);
void append(node** head, node** tail, void* val);
void delete_list(node** head, node** tail);
void print_list(node* n);
int list_length(node* n);

#endif /* LINKED_LIST_H */
