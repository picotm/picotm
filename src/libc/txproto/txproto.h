/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef TXPROTO_H
#define TXPROTO_H

struct block_sequence
{
    size_t         len;
    unsigned char *msg;
};

enum command
{
    CMD_EMPTY = 0,
    CMD_PLAIN,
    CMD_TXCHAL,
    CMD_TXCMMT,
    CMD_TXABRT,
    CMD_TXDATA,
    CMD_TXEOTX,
    CMD_TXNNDO
};

struct packet
{
    enum command          cmd;
    struct block_sequence data;
};

int
txproto_wait_data(int sockfd, int seconds);

int
txproto_recv_packet(int sockfd, struct packet *packet, int flags, int wait);

int
txproto_send_txchal(int sockfd);

ssize_t
txproto_send_txdata(int sockfd, const void *buffer, size_t length);

int
txproto_send_txeotx(int sockfd);

int
txproto_send_txcmmt(int sockfd);

int
txproto_send_txabrt(int sockfd);

int
txproto_send_txnndo(int sockfd);

#endif

