#ifndef RETRIEVER_H
#define RETRIEVER_H

#include <stdint.h>

typedef enum { ESTABLISHED } State;

typedef struct {
    uint64_t inode;
    union {
        uint32_t local_ip4;
        uint8_t local_ip6[16];
    };
    uint16_t local_port;

    union {
        uint32_t remote_ip4;
        uint8_t remote_ip6[16];
    };
    uint16_t remote_port;

    State state;
} Socket;

#endif // !RETRIEVER_H
