#ifndef IP_TRIE_H
#define IP_TRIE_H

#include "smarray.h"
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#define N_OF_CHILD 2

typedef struct TrieNode {
    struct TrieNode *child[N_OF_CHILD];
    sa_family_t family;
    int prefix_len;
    union {
        uint32_t ip4;
        uint8_t ip6[16];
    } addr;
    SMarray *sockets;
} TrieNode;

typedef struct {
    TrieNode *v4_root;
    TrieNode *v6_root;
} IpIndex;

static inline int tget_bit(const TrieNode *node, int depth) {
    if (node->family == AF_INET)
        return (node->addr.ip4 >> (31 - depth)) & 1;
    int byte = depth / 8;
    int bit = 7 - (depth % 8);
    return (node->addr.ip6[byte] >> bit) & 1;
}

static inline int key_bit(const void *addr, sa_family_t family, int depth) {
    if (family == AF_INET) {
        uint32_t ip4;
        memcpy(&ip4, addr, 4);
        return (ip4 >> (31 - depth)) & 1;
    }
    const uint8_t *ip6 = (const uint8_t *)addr;
    int byte = depth / 8;
    int bit = 7 - (depth % 8);
    return (ip6[byte] >> bit) & 1;
}

void trie_insert(IpIndex *idx, sa_family_t family, const void *addr,
                 Socket *sock);
SMarray *trie_prefix_lookup(IpIndex *idx, sa_family_t family,
                            const void *prefix_bytes, int prefix_len);
void trie_remove(IpIndex *idx, sa_family_t family, const void *addr,
                 Socket *sock);
void ip_index_destroy(IpIndex *idx);
#endif // !IP_TRIE_H
