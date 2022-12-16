//
// Created by jaranzie on 12/4/22.
//
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "../include/protocol.h"
#include <poll.h>
/*
 * Send a packet, which consists of a fixed-size header followed by an
 * optional associated data payload.
 *
 * @param fd  The file descriptor on which packet is to be sent.
 * @param pkt  The fixed-size packet header, with multi-byte fields
 *   in network byte order
 * @param data  The data payload, or NULL, if there is none.
 * @return  0 in case of successful transmission, -1 otherwise.
 *   In the latter case, errno is set to indicate the error.
 *
 * All multi-byte fields in the packet are assumed to be in network byte order.
 */
int proto_send_packet(int fd, BRS_PACKET_HEADER *hdr, void *payload) {
    // Assumes that hdr and payload are in network byte order
    int bytes_to_write = sizeof(BRS_PACKET_HEADER);
    void* cur_byte = hdr;
    while(bytes_to_write > 0) {
        int wrote = write(fd, cur_byte, bytes_to_write);
        if (wrote <= 0) {
            if (errno == EINTR) {
                wrote = 0;
            } else {
                return -1;
            }
        }
        bytes_to_write -= wrote;
        cur_byte += wrote;
    }
    if(hdr->size != 0) {
        bytes_to_write = ntohs(hdr->size);
        cur_byte = payload;
        while(bytes_to_write > 0) {
            int wrote = write(fd, cur_byte, bytes_to_write);
            if (wrote <= 0) {
                if (errno == EINTR) {
                    wrote = 0;
                } else {
                    return -1;
                }
            }
            bytes_to_write -= wrote;
            cur_byte += wrote;
        }
    }
    return 0;
}

/*
 * Receive a packet, blocking until one is available.
 *
 * @param fd  The file descriptor from which the packet is to be received.
 * @param pkt  Pointer to caller-supplied storage for the fixed-size
 *   portion of the packet.
 * @param datap  Pointer to a variable into which to store a pointer to any
 *   payload received.
 * @return  0 in case of successful reception, -1 otherwise.  In the
 *   latter case, errno is set to indicate the error.
 *
 * The returned packet has all multi-byte fields in network byte order.
 * If the returned payload pointer is non-NULL, then the caller has the
 * responsibility of freeing that storage.
 */
int proto_recv_packet(int fd, BRS_PACKET_HEADER *hdr, void **payloadp) {
    struct pollfd poll_fd = {fd, POLLIN, 0};
    poll(&poll_fd, 1, -1);
    if((poll_fd.revents & (POLLHUP | POLLNVAL | POLLERR )) != 0) {
        return -1;
    }
    int bytes_to_read = sizeof(BRS_PACKET_HEADER);
    void* cur_byte = hdr;
    while(bytes_to_read > 0) {
        int readn = read(fd, cur_byte, bytes_to_read);
        if (readn < 0) {
            if (errno == EINTR) {
                readn = 0;
            } else {
                return -1;
            }
        } else if (readn == 0) {
            return -1;
        }
        bytes_to_read -= readn;
        cur_byte += readn;
    }
    if(hdr->size != 0) {
        bytes_to_read = ntohs(hdr->size);
        void* payload = malloc(bytes_to_read);
        *payloadp = payload;
        cur_byte = payload;
        while(bytes_to_read > 0) {
            int readn = read(fd, cur_byte, bytes_to_read);
            if (readn < 0) {
                if (errno == EINTR) {
                    readn = 0;
                } else {
                    return -1;
                }
            } else if (readn == 0) {
                break;
            }
            bytes_to_read -= readn;
            cur_byte += readn;
        }
    }
    return 0;
}


