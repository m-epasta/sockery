#ifndef SMARRAY_H
#define SMARRAY_H

#include "retriever.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
    Socket **data;
    size_t size;
    size_t cap;
} SMarray;

/* if initial_capacity is set to -1, then the default capacity will be used
 * (#define SMARRAY_INITIAL_CAP) */
void smarray_init(SMarray *arr, size_t initial_capacity);

void smarray_free(SMarray *arr);

void smarray_push(SMarray *arr, Socket *sock);

/* Returns 0 when successfull, otherwise retrun -1 */
int smarray_remove(SMarray *arr, Socket *sock);

Socket *smarray_find_by_inode(SMarray *arr, uint64_t inode);

void smarray_reserve(SMarray *arr, size_t new_capacity);

#endif // !SMARRAY_H
