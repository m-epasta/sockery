#ifndef SOCKSET_H
#define SOCKSET_H

#include "retriever.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint64_t inode;
    Socket *sock;
} SEntry;

#define SSE_INODE_EMPTY 0
#define SSE_INODE_TOMB 1

typedef struct {
    SEntry *entries;
    size_t cap;
    size_t count;
    size_t tombstones;
} SSet;

static inline size_t hash_inode(uint64_t inode) {
    return (size_t)(inode * 11400714819323198549ULL);
}

void sset_init(SSet *set);
Socket **sset_lookup(const SSet *set, uint64_t inode);

/* Returns 1 if successful, otherwise returns -1 */
int sset_insert(SSet *set, Socket *sock);

static void sset_resize(SSet *set, size_t new_cap);
Socket *sset_remove(SSet *set, uint64_t inode);
void destroy_sset(SSet *set);

typedef void (*sset_foreach_func)(Socket *sock, void *userdata);
void sset_foreach(const SSet *set, sset_foreach_func func, void *userdata);
#endif // !SOCKSET_H
