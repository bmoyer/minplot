#include <stdlib.h>
#include "array.h"

void init_array(array* a, size_t initial_size) {
    a->array = malloc(initial_size * sizeof(data_point*));
    a->used = 0;
    a->size = initial_size;
}

void insert_array(array* a, data_point* data) {
    if(a->used == a->size) {
        a->size *= 2;
        a->array = realloc(a->array, a->size * sizeof(data_point*));
    }
    a->array[a->used++] = data;
}

void free_array(array* a) {
    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
}
