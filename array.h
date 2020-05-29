#ifndef ARRAY_H
#define ARRAY_H 

typedef struct {
    int val;
} data_point;

typedef struct {
    data_point** array;
    size_t used;
    size_t size;
} array;

void init_array(array* a, size_t initial_size);
void insert_array(array* a, data_point* data);
void free_array(array* a);

#endif /* ARRAY_H */
