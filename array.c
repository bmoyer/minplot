#include <stdlib.h>
#include "array.h"

void init_array(array* a, size_t initial_size) {
    a->array = malloc(initial_size * sizeof(data_point*));
    a->size = 0;
    a->capacity = initial_size;
}

void insert_array(array* a, data_point* data) {
    if(a->size == a->capacity) {
        a->capacity *= 2;
        a->array = realloc(a->array, a->capacity * sizeof(data_point*));
    }
    a->array[a->size++] = data;
}

void free_array(array* a) {
    free(a->array);
    a->array = NULL;
    a->size = a->capacity = 0;
}
