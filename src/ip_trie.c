#include "ip_trie.h"
#include "retriever.h"
#include "smarray.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define IP_MAX_DEPTH ((family == AF_INET) ? 32 : 128)

void trie_insert(IpIndex* idx, sa_family_t family, const void* addr,
                 Socket* sock) {
    TrieNode** root = (family == AF_INET) ? &idx->v4_root : &idx->v6_root;

    int max_depth = IP_MAX_DEPTH;
    const uint8_t* ip6 = NULL;
    uint32_t ip4 = 0;

    if (family == AF_INET)
        memcpy(&ip4, addr, sizeof(ip4));
    else
        ip6 = (const uint8_t*)addr;

    TrieNode* node = *root;
    if (!node) {
        node = calloc(1, sizeof(TrieNode));
        if (!node)
            abort();
        node->family = family;
        node->prefix_len = 0;
        if (family == AF_INET)
            node->addr.ip4 = ip4;
        else
            memcpy(node->addr.ip6, ip6, 16);
        *root = node;
    }

    for (int depth = 0; depth < max_depth; depth++) {
        int bit = key_bit(addr, family, depth);

        if (!node->child[bit]) {
            TrieNode* child = calloc(1, sizeof(TrieNode));
            if (!child)
                abort();
            child->family = family;
            child->prefix_len = depth + 1;

            if (family == AF_INET)
                child->addr.ip4 = ip4;
            else
                memcpy(child->addr.ip6, ip6, 16);
            node->child[bit] = child;
        }

        node = node->child[bit];
    }

    if (!node->sockets) {
        node->sockets = malloc(sizeof(SMarray));
        if (!node->sockets)
            abort();
        smarray_init(node->sockets, 0);
    }
    smarray_push(node->sockets, sock);
}

static void collect_sockets(TrieNode* node, SMarray* out) {
    if (!node)
        return;
    if (node->sockets)
        for (size_t i = 0; i < node->sockets->size; i++)
            smarray_push(out, node->sockets->data[i]);
    for (int i = 0; i < N_OF_CHILD; i++)
        collect_sockets(node->child[i], out);
}

SMarray* trie_prefix_lookup(IpIndex* idx, sa_family_t family,
                            const void* prefix_bytes, int prefix_len) {
    TrieNode* root = (family == AF_INET) ? idx->v4_root : idx->v6_root;
    if (!root)
        return NULL;

    TrieNode* node = root;
    int max_depth = IP_MAX_DEPTH;
    if (prefix_len > max_depth)
        prefix_len = max_depth;

    for (int depth = 0; depth < prefix_len; depth++) {
        int bit = key_bit(prefix_bytes, family, depth);
        if (!node->child[bit])
            return NULL;
        node = node->child[bit];
    }

    SMarray* result = malloc(sizeof(SMarray));
    if (!result)
        abort();
    smarray_init(result, 0);
    collect_sockets(node, result);
    return result;
}

void trie_remove(IpIndex* idx, sa_family_t family, const void* addr,
                 Socket* sock) {
    TrieNode* root = (family == AF_INET) ? idx->v4_root : idx->v6_root;
    if (!root)
        return;
    TrieNode* node = root;
    int max_depth = IP_MAX_DEPTH;

    for (int depth = 0; depth < max_depth; depth++) {
        int bit = key_bit(addr, family, depth);
        if (!node->child[bit])
            return;
        node = node->child[bit];
    }

    if (!node->sockets)
        return;
    SMarray* arr = node->sockets;
    for (size_t i = 0; i < arr->size; i++)
        if (arr->data[i] == sock) {
            smarray_remove(arr, sock);
            break;
        }
}

static void trie_node_destroy(TrieNode* node) {
    if (!node)
        return;
    for (int i = 0; i < N_OF_CHILD; i++)
        trie_node_destroy(node->child[i]);
    if (node->sockets) {
        smarray_free(node->sockets);
        free(node->sockets);
    }
    free(node);
}

void ip_index_destroy(IpIndex* idx) {
    trie_node_destroy(idx->v4_root);
    trie_node_destroy(idx->v6_root);
    idx->v4_root = NULL;
    idx->v6_root = NULL;
}
