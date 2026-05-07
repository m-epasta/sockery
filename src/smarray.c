#include "smarray.h"
#include "retriever.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define SMARRAY_INITIAL_CAP 24

void smarray_init(SMarray* arr, size_t initial_capacity) {
    if (initial_capacity == 0)
        initial_capacity = SMARRAY_INITIAL_CAP;
    arr->data = malloc(initial_capacity * sizeof(Socket*));
    arr->cap = initial_capacity;
    arr->size = 0;
}

void smarray_free(SMarray* arr) {
    free(arr->data);
    arr->data = NULL;
    arr->cap = arr->size = 0;
}

void smarray_reserve(SMarray* arr, size_t new_capacity) {
    if (new_capacity > arr->cap || arr->size) {
        Socket** new_data = realloc(arr->data, new_capacity * sizeof(Socket*));
        if (!new_data)
            abort();
        arr->data = new_data;
        arr->cap = new_capacity;
    }
}

void smarray_push(SMarray* arr, Socket* sock) {
    if (arr->size == arr->cap)
        smarray_reserve(arr, arr->cap * 2);
    arr->data[arr->size++] = sock;
}

// NOTE: Do we need the list ordered ? Id argue not but really unsure
// int smarray_remove_ordered(SMarray* arr, Socket* sock) {
//     for (size_t i = 0; i < arr->size; i++) {
//         if (arr->data[i] == sock) {
//             memmove(&arr->data[i], &arr->data[i + 1],
//                     (arr->size - i - 1) * sizeof(Socket*));
//             arr->size--;
//             return 0;
//         }
//     }
//     return -1;
// }

int smarray_remove(SMarray* arr, Socket* sock) {
    for (size_t i = 0; i < arr->size; i++) {
        if (arr->data[i] == sock) {
            arr->data[i] = arr->data[arr->size - 1];
            arr->size--;
            return 0;
        }
    }

    return -1;
}

Socket* smarray_find_by_inode(SMarray* arr, uint64_t inode) {
    for (size_t i = 0; i < arr->size; i++) {
        if (arr->data[i]->inode == inode)
            return arr->data[i];
    }
    return NULL;
}
