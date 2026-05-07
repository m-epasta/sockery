#include "sockset.h"
#include "retriever.h"
#include <stdint.h>
#include <stdlib.h>

#define SSE_INITIAL_CAP 16

void sset_init(SSet* set) {
    set->cap = SSE_INITIAL_CAP;
    set->count = 0;
    set->tombstones = 0;
    set->entries = calloc(set->cap, sizeof(SEntry));
}

Socket** sset_lookup(const SSet* set, uint64_t inode) {
    if (set->count == 0)
        return NULL;
    size_t idx = hash_inode(inode) & (set->cap - 1);
    while (set->entries[idx].inode != SSE_INODE_EMPTY) {
        if (set->entries[idx].inode != SSE_INODE_TOMB &&
            set->entries[idx].inode == inode)
            return &set->entries[idx].sock;
        idx = (idx + 1) & (set->cap - 1);
    }

    return NULL;
}

int sset_insert(SSet* set, Socket* sock) {
    if (sock == NULL)
        return -1;
    uint64_t inode = sock->inode;

    if ((set->count + set->tombstones) * 10 >= set->cap * 6)
        sset_resize(set, set->cap * 2);

    size_t idx = hash_inode(inode) & (set->cap - 1);
    while (set->entries[idx].inode != SSE_INODE_EMPTY &&
           set->entries[idx].inode != SSE_INODE_TOMB) {
        if (set->entries[idx].inode == inode)
            return -1;
        idx = (idx + 1) & (set->cap - 1);
    }

    if (set->entries[idx].inode == SSE_INODE_TOMB)
        set->tombstones--;
    set->entries[idx].inode = inode;
    set->entries[idx].sock = sock;
    set->count++;
    return 0;
}

Socket* sset_remove(SSet* set, uint64_t inode) {
    size_t idx = hash_inode(inode) & (set->cap - 1);
    while (set->entries[idx].inode != SSE_INODE_EMPTY) {
        if (set->entries[idx].inode != SSE_INODE_TOMB &&
            set->entries[idx].inode == inode) {
            Socket* removed = set->entries[idx].sock;
            set->entries[idx].inode = SSE_INODE_TOMB;
            set->entries[idx].sock = NULL;
            set->count--;
            set->tombstones++;
            return removed;
        }
        idx = (idx + 1) & (set->cap - 1);
    }

    return NULL;
}

static void sset_resize(SSet* set, size_t new_cap) {
    size_t old_cap = set->cap;
    SEntry* old_entries = set->entries;

    set->cap = new_cap;
    set->count = 0;
    set->tombstones = 0;
    set->entries = calloc(new_cap, sizeof(SEntry));

    for (size_t i = 0; i < old_cap; i++) {
        if (old_entries[i].inode != SSE_INODE_EMPTY &&
            old_entries[i].inode != SSE_INODE_TOMB) {
            size_t idx = hash_inode(old_entries[i].inode) & (new_cap - 1);
            while (set->entries[idx].inode != SSE_INODE_EMPTY)
                idx = (idx + 1) & (new_cap - 1);
            set->entries[idx] = old_entries[i];
            set->count++;
        }
    }
    free(old_entries);
}

void destroy_sset(SSet* set) {
    for (size_t i = 0; i < set->cap; i++) {
        if (set->entries[i].inode != SSE_INODE_EMPTY &&
            set->entries[i].inode != SSE_INODE_TOMB)
            free(set->entries[i].sock);
    }
    free(set->entries);
    set->entries = NULL;
    set->cap = set->count = set->tombstones = 0;
}

void sset_foreach(const SSet* set, sset_foreach_func func, void* userdata) {
    for (size_t i = 0; i < set->cap; i++) {
        if (set->entries[i].inode != SSE_INODE_EMPTY &&
            set->entries[i].inode != SSE_INODE_TOMB) {
            func(set->entries[i].sock, userdata);
        }
    }
}
