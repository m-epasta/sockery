#ifndef RETRIEVER_H
#define RETRIEVER_H

#include <stdint.h>

typedef enum {
    ESTABLISHED,
    UNCONN,
    LISTEN,
    SYN_SENT,
    SYN_RECV,
    FIN_WAIT1,
    FIN_WAIT2,
    CLOSE_WAIT,
    LAST_ACK,
    TIME_WAIT,
    CLOSING,
    CLOSED
} State;

inline const char *display_state(State s) {
    switch (s) {
    case ESTABLISHED:
        return "ESTABLISHED";
    case UNCONN:
        return "UNCONN";
    case LISTEN:
        return "LISTEN";
    case SYN_SENT:
        return "SYN-SENT";
    case SYN_RECV:
        return "SYN-RECV";
    case FIN_WAIT1:
        return "FIN-WAIT-1";
    case FIN_WAIT2:
        return "FIN-WAIT-2";
    case CLOSE_WAIT:
        return "CLOSE-WAIT";
    case LAST_ACK:
        return "LAST-ACK";
    case TIME_WAIT:
        return "TIME-WAIT";
    case CLOSING:
        return "CLOSING";
    case CLOSED:
        return "CLOSED";
    default:
        return "UNKNOWN";
    }
}

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
