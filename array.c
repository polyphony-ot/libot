#include "array.h"

void array_init(array* arr, size_t size) {
    arr->len = 0;
    arr->cap = 0;
    arr->size = size;
    arr->data = NULL;
}

void array_free(array* arr) {
    free(arr->data);
}

void array_ensure_size(array* arr) {
    if (arr->len == 0) {
        arr->cap = 1;
        arr->data = malloc(arr->size);
    } else if (arr->len >= arr->cap) {
        arr->cap *= 2;
        arr->data = realloc(arr->data, arr->size * arr->cap);
    }
}

void* array_append(array* arr) {
    array_ensure_size(arr);
    size_t temp = arr->len;
    arr->len++;
    
    return ((char*)arr->data) + (arr->size * temp);
}

bool array_equal(const array* arr1, const array* arr2) {
    size_t size1 = arr1->size * arr1->len;
    size_t size2 = arr2->size * arr2->len;
    if (size1 != size2) {
        return false;
    }
    
    return (memcmp(arr1->data, arr2->data, size1) == 0);
}