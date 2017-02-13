
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "txproto.h"
#include "connection.h"

struct connection
{
    enum connection_state state;
    int                   fd;
    unsigned char        *rcvbuf;
    size_t                rcvbuflen;
};

static ssize_t
read_buffer(struct connection *conn, void *buffer, size_t length)
{
    assert(conn);

    if (!conn->rcvbuflen) {
        return 0;
    }

    if (length < conn->rcvbuflen) {
        memcpy(buffer, conn->rcvbuf, length);
        conn->rcvbuflen -= length;
    } else {
        memcpy(buffer, conn->rcvbuf, conn->rcvbuflen);
        length = conn->rcvbuflen;
        conn->rcvbuflen = 0;
    }

    return length;
}

static ssize_t
add_data(struct connection *conn, const struct packet *packet)
{
    assert(conn);
    assert(packet);

    void *tmp = realloc(conn->rcvbuf, conn->rcvbuflen+packet->data.len);

    if (!tmp) {
        perror("realloc");
        return -1;
    }

    conn->rcvbuf = tmp;

    memcpy(conn->rcvbuf+conn->rcvbuflen, packet->data.msg, packet->data.len);
    conn->rcvbuflen += packet->data.len;

    return packet->data.len;
}

struct connection *
connection_create(int fd)
{
    struct connection *conn = malloc(sizeof(*conn));

    if (!conn) {
        perror("malloc");
        return NULL;
    }

    if (connection_init(conn, fd) < 0) {
        free(conn);
        return NULL;
    }

    return conn;
}

int
connection_init(struct connection *conn, int fd)
{
    assert(conn);

    conn->state = CONNSTATE_UNKNOWN;
    conn->fd = fd;
    conn->rcvbuf = NULL;
    conn->rcvbuflen = 0;

    /*int nodelay = 1;
    if (setsockopt(conn->fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay)) < 0) {
        perror("setsockopt");
        return ERR_SYSTEM;
    }*/

    return 0;
}

void
connection_uninit(struct connection *conn)
{
    assert(conn);
}

void
connection_destroy(struct connection *conn)
{
    connection_uninit(conn);
    free(conn);
}

enum state
{
    STATE_UNKNOWN = 0,
    STATE_NOUNDO,
    STATE_REVOCABLE,
    STATE_WANTPLAIN,
    STATE_PEEREOTX,
    STATE_PEERCOMMIT,
    STATE_PEERABORT,
    STATE_WAITRESP,
    STATE_WAITEOTX,
    STATE_WAITCMMT,
    STATE_WAITABRT,
    STATE_WAITNNDO,
    STATE_RETURN
};

static const char* statenametab[] = {
    "unknown",
    "noundo",
    "revocable",
    "wantplain",
    "peereotx",
    "peercommit",
    "peerabort",
    "waitresp",
    "waiteotx",
    "waitcmmt",
    "waitabrt",
    "waitnndo",
    "return" };

static const enum state initstatetab[] = {
    STATE_UNKNOWN,
    STATE_NOUNDO,
    STATE_REVOCABLE,
    STATE_WANTPLAIN,
    STATE_PEEREOTX,
    STATE_PEERCOMMIT,
    STATE_PEERABORT };

static enum state
any_invalid(void)
{
    abort();
    return STATE_RETURN;
}

/*
 * Send
 */

static enum state
send_unknown(struct connection *conn, const void *buffer, size_t len, ssize_t *res)
{
    if (txproto_send_txchal(conn->fd) < 0) {
        *res = -1;
        return STATE_RETURN;
    }

    return STATE_WAITRESP;
}

static enum state
send_noundo(struct connection *conn, const void *buffer, size_t len, ssize_t *res)
{
    return any_invalid();

    /* *res = send(conn->fd, buffer, len, 0);
    return STATE_RETURN;*/
}

static enum state
send_revocable(struct connection *conn, const void *buffer, size_t len, ssize_t *res)
{
    enum state next = STATE_UNKNOWN;

    /* Read packets to see if we're going noundo or so */

    struct packet packet;

    if (txproto_wait_data(conn->fd, 0)) {
        txproto_recv_packet(conn->fd, &packet, 0);
    } else {
        packet.cmd = CMD_EMPTY;
    }

    switch (packet.cmd) {
        case CMD_EMPTY:
            /* Nothing to read, so send data and return */
            *res = txproto_send_txdata(conn->fd, buffer, len);
            next = STATE_RETURN;
            break;
        case CMD_TXABRT:
            conn->state = CONNSTATE_PEERABORT;
            next = STATE_PEERABORT;
            break;
        case CMD_TXDATA:
            *res = add_data(conn, &packet);
            next = STATE_REVOCABLE;
            break;
        case CMD_TXEOTX:
            conn->state = CONNSTATE_PEERENDOFTX;
            next = STATE_PEEREOTX;
            break;
        case CMD_TXNNDO:
            *res = ERR_NOUNDO;
            next = STATE_RETURN;
            break;
        case CMD_PLAIN:
        case CMD_TXCHAL:
        case CMD_TXRESP:
        case CMD_TXCMMT:
        case CMD_TXNACK:
            /* Cannot receive these commands here */
            abort();
    }

    return next;
}

static enum state
send_plain(struct connection *conn, const void *buffer, size_t len, ssize_t *res)
{
    *res = send(conn->fd, buffer, len, 0);
    return STATE_RETURN;
}

static enum state
send_peereotx(struct connection *conn, const void *buffer, size_t len, ssize_t *res)
{
    *res = txproto_send_txdata(conn->fd, buffer, len);
    return STATE_RETURN;
}

static enum state
send_peercommit(struct connection *conn, const void *buffer, size_t len, ssize_t *res)
{
    /* Cannot send after commit phase has begun */
    return any_invalid();
}

static enum state
send_peerabort(struct connection *conn, const void *buffer, size_t len, ssize_t *res)
{
    *res = ERR_PEERABORT;
    return STATE_RETURN;
}

static enum state
send_waitresp(struct connection *conn, const void *buffer, size_t len, ssize_t *res)
{
    enum state next;

    /* Read data from socket into connection buffer */

    struct packet packet;

    if (txproto_wait_data(conn->fd, 5)) {
        txproto_recv_packet(conn->fd, &packet, 0);
    } else {
        packet.cmd = CMD_PLAIN;
        packet.data.msg = NULL;
        packet.data.len = 0;
    }

    switch (packet.cmd) {
        case CMD_EMPTY:
            next = STATE_WAITRESP;
            break;
        case CMD_PLAIN:
            add_data(conn, &packet);
            *res = ERR_NOUNDO;
            conn->state = CONNSTATE_WANTPLAIN;
            next = STATE_RETURN;
            break;
        case CMD_TXCHAL:
            conn->state = CONNSTATE_REVOCABLE;
            next = STATE_REVOCABLE;
            break;
        case CMD_TXRESP:
            conn->state = CONNSTATE_REVOCABLE;
            next = STATE_REVOCABLE;
            break;
        case CMD_TXNNDO:
        case CMD_TXABRT:
        case CMD_TXCMMT:
        case CMD_TXDATA:
        case CMD_TXEOTX:
        case CMD_TXNACK:
            /* Cannot receive these commands here */
            abort();
    }

    return next;
}

static enum state
send_waiteotx(struct connection *conn, const void *buffer, size_t length, ssize_t *res)
{
    return any_invalid();
}

static enum state
send_waitcmmt(struct connection *conn, const void *buffer, size_t length, ssize_t *res)
{
    return any_invalid();
}

static enum state
send_waitabrt(struct connection *conn, const void *buffer, size_t length, ssize_t *res)
{
    return any_invalid();
}

static enum state
send_waitnndo(struct connection *conn, const void *buffer, size_t length, ssize_t *res)
{
    return any_invalid();
}

ssize_t
connection_send(struct connection *conn, const void *buffer, size_t length)
{
    static enum state (* const state[])(struct connection*, const void*, size_t, ssize_t*) = {
        send_unknown,
        send_noundo,
        send_revocable,
        send_plain,
        send_peereotx,
        send_peercommit,
        send_peerabort,
        send_waitresp,
        send_waiteotx,
        send_waitcmmt,
        send_waitabrt,
        send_waitnndo,
        NULL };

    assert(conn);

    ssize_t res;

    enum state next = initstatetab[conn->state];

    while (state[next]) {

        fprintf(stderr, "send_%s\n", statenametab[next]);
        next = state[next](conn, buffer, length, &res);
    }

    return res;
}

/*
 * Receive
 */

static enum state
recv_unknown(struct connection *conn, void *buffer, size_t length, ssize_t *res)
{
    if (txproto_send_txchal(conn->fd) < 0) {
        abort();
    }

    return STATE_WAITRESP;
}

static enum state
recv_noundo(struct connection *conn, void *buffer, size_t length, ssize_t *res)
{
    return any_invalid();

    /*if (conn->rcvbuflen) {
        *res = read_buffer(conn, buffer, length);
    } else {
        *res = recv(conn->fd, buffer, length, 0);
    }
    return STATE_RETURN;*/
}

static enum state
recv_revocable(struct connection *conn, void *buffer, size_t length, ssize_t *res)
{
    /* Read from local buffer */

    *res = read_buffer(conn, buffer, length);

    if (*res) {
        return STATE_RETURN;
    }

    enum state next = STATE_RETURN;

    /* Read packets from socket, if buffer is empty */

    struct packet packet;

    txproto_recv_packet(conn->fd, &packet, 0);

    switch (packet.cmd) {
        case CMD_EMPTY:
            *res = 0;
            next = STATE_RETURN;
            break;
        case CMD_TXABRT:
            conn->state = CONNSTATE_PEERABORT;
            next = STATE_PEERABORT;
            break;
        case CMD_TXDATA:
            *res = add_data(conn, &packet);
            *res = read_buffer(conn, buffer, length);
            next = STATE_RETURN;
            break;
        case CMD_TXEOTX:
            conn->state = CONNSTATE_PEERENDOFTX;
            next = STATE_PEEREOTX;
            break;
        case CMD_TXNNDO:
            *res = ERR_NOUNDO;
            next = STATE_RETURN;
            break;
        case CMD_TXCMMT:
        case CMD_TXCHAL:
        case CMD_TXRESP:
        case CMD_PLAIN:
        case CMD_TXNACK:
            /* Cannot receive these commands here */
            abort();
    }

    return next;
}

static enum state
recv_plain(struct connection *conn, void *buffer, size_t length, ssize_t *res)
{
    *res = recv(conn->fd, buffer, length, 0);
    return STATE_RETURN;
}

static enum state
recv_peereotx(struct connection *conn, void *buffer, size_t length, ssize_t *res)
{
    /* Peer is in eotx mode, so no new data can arive, therefore simply
       read buffer */
    *res = read_buffer(conn, buffer, length);
    return STATE_RETURN;
}

static enum state
recv_peercommit(struct connection *conn, void *buffer, size_t length, ssize_t *res)
{
    /* Peer is already in commit, so it is an error to still
      read from socket */
    return any_invalid();
    /*abort();
    return STATE_UNKNOWN;*/
}

static enum state
recv_peerabort(struct connection *conn, void *buffer, size_t length, ssize_t *res)
{
    *res = ERR_PEERABORT;
    return STATE_RETURN;
}

static enum state
recv_waitresp(struct connection *conn, void *buffer, size_t length, ssize_t *res)
{
    enum state next;

    /* Read data from socket into connection buffer */

    struct packet packet;

    if (txproto_wait_data(conn->fd, 5)) {
        txproto_recv_packet(conn->fd, &packet, 0);
    } else {
        packet.cmd = CMD_PLAIN;
        packet.data.msg = NULL;
        packet.data.len = 0;
    }

    switch (packet.cmd) {
        case CMD_EMPTY:
            next = STATE_WAITRESP;
            break;
        case CMD_PLAIN:
            add_data(conn, &packet);
            *res = ERR_NOUNDO;
            conn->state = CONNSTATE_WANTPLAIN;
            next = STATE_RETURN;
            break;
        case CMD_TXCHAL:
            conn->state = CONNSTATE_REVOCABLE;
            next = STATE_REVOCABLE;
            break;
        case CMD_TXRESP:
            conn->state = CONNSTATE_REVOCABLE;
            next = STATE_REVOCABLE;
            break;
        case CMD_TXCMMT:
        case CMD_TXABRT:
        case CMD_TXDATA:
        case CMD_TXNACK:
        case CMD_TXNNDO:
        case CMD_TXEOTX:
            /* Cannot receive these commands here */
            abort();
    }

    return next;
}

static enum state
recv_waiteotx(struct connection *conn, void *buffer, size_t length, ssize_t *res)
{
    return any_invalid();

    /*abort();
    return STATE_RETURN;*/
}

static enum state
recv_waitcmmt(struct connection *conn, void *buffer, size_t length, ssize_t *res)
{
    return any_invalid();

    /*abort();
    return STATE_RETURN;*/
}

static enum state
recv_waitabrt(struct connection *conn, void *buffer, size_t length, ssize_t *res)
{
    return any_invalid();
    /*abort();
    return STATE_RETURN;*/
}

static enum state
recv_waitnndo(struct connection *conn, void *buffer, size_t length, ssize_t *res)
{
    return any_invalid();
    /*abort();
    return STATE_RETURN;*/
}

ssize_t
connection_recv(struct connection *conn, void *buffer, size_t length)
{
    static enum state (* const state[])(struct connection*, void*, size_t, ssize_t*) = {
        recv_unknown,
        recv_noundo,
        recv_revocable,
        recv_plain,
        recv_peereotx,
        recv_peercommit,
        recv_peerabort,
        recv_waitresp,
        recv_waiteotx,
        recv_waitcmmt,
        recv_waitabrt,
        recv_waitnndo,
        NULL };

    assert(conn);

    ssize_t res;

    enum state next = initstatetab[conn->state];

    while (state[next]) {

        fprintf(stderr, "recv_%s\n", statenametab[next]);
        next = state[next](conn, buffer, length, &res);
    }

    return res;
}

/*
 * End of transaction
 */

static enum state
eotx_unknown(struct connection *conn, int *res)
{
    if (txproto_send_txchal(conn->fd) < 0) {
        abort();
    }

    return STATE_WAITRESP;
}

static enum state
eotx_noundo(struct connection *conn, int *res)
{
    return any_invalid();

    /* Peer is in noundo, so eotx always succeeds */
    /*conn->state = CONNSTATE_PEERENDOFTX;
    *res = 0;
    return STATE_RETURN;*/
}

static enum state
eotx_revocable(struct connection *conn, int *res)
{
    /* Signal commit to peer, wait for answer */

    if (txproto_send_txeotx(conn->fd) < 0) {
        abort();
    }
    *res = 0;

    return STATE_WAITEOTX;
}

static enum state
eotx_plain(struct connection *conn, int *res)
{
    /* Force abort in case that plain was selected, this triggers noundo */
    *res = ERR_PEERABORT;
    return STATE_RETURN;
}

static enum state
eotx_peereotx(struct connection *conn, int *res)
{
    /* Peer signalled commit, so commit succeeds immediately */

    if (txproto_send_txeotx(conn->fd) < 0) {
        abort();
    }
    *res = 0;

    return STATE_RETURN;
}

static enum state
eotx_peercommit(struct connection *conn, int *res)
{
    /* Peer cannot have signalled commit */
    return any_invalid();
}

static enum state
eotx_peerabort(struct connection *conn, int *res)
{
    /* Peer signalled abort, so eotx failes immediately */
    *res = ERR_PEERABORT;
    return STATE_RETURN;
}

static enum state
eotx_waitresp(struct connection *conn, int *res)
{
    enum state next;

    /* Read data from socket into connection buffer */

    struct packet packet;

    txproto_recv_packet(conn->fd, &packet, 0);

    switch (packet.cmd) {
        case CMD_EMPTY:
            next = STATE_WAITRESP;
            break;
        case CMD_PLAIN:
            add_data(conn, &packet);
            *res = ERR_NOUNDO;
            conn->state = CONNSTATE_WANTPLAIN;
            next = STATE_RETURN;
            break;
        case CMD_TXCHAL:
            conn->state = CONNSTATE_REVOCABLE;
            next = STATE_REVOCABLE;
            break;
        case CMD_TXRESP:
            conn->state = CONNSTATE_REVOCABLE;
            next = STATE_REVOCABLE;
            break;
        case CMD_TXABRT:
        case CMD_TXCMMT:
        case CMD_TXDATA:
        case CMD_TXEOTX:
        case CMD_TXNACK:
        case CMD_TXNNDO:
            /* Cannot receive these commands here */
            abort();
    }

    return next;
}

static enum state
eotx_waiteotx(struct connection *conn, int *res)
{
    enum state next;

    /* Read data from socket into connection buffer */

    struct packet packet;

    txproto_recv_packet(conn->fd, &packet, 0);

    switch (packet.cmd) {
        case CMD_EMPTY:
            next = STATE_WAITEOTX;
            break;
        case CMD_TXEOTX:
            conn->state = CONNSTATE_PEERENDOFTX;
            *res = 0;
            next = STATE_RETURN;
            break;
        case CMD_TXNACK:
            conn->state = CONNSTATE_PEERABORT;
            next = STATE_PEERABORT;
            break;
        case CMD_TXDATA:
            if ( (*res = add_data(conn, &packet)) < 0 ) {
                return STATE_RETURN;
            }
            break;
        case CMD_TXABRT:
            conn->state = CONNSTATE_PEERABORT;
            next = STATE_PEERABORT;
            break;
        case CMD_TXCHAL:
        case CMD_TXRESP:
        case CMD_TXCMMT:
        case CMD_PLAIN:
        case CMD_TXNNDO:
            /* Cannot receive these commands here */
            abort();
    }

    return next;
}

static enum state
eotx_waitcmmt(struct connection *conn, int *res)
{
    return any_invalid();
}

static enum state
eotx_waitabrt(struct connection *conn, int *res)
{
    return any_invalid();
}

static enum state
eotx_waitnndo(struct connection *conn, int *res)
{
    return any_invalid();
}

int
connection_eotx(struct connection *conn)
{
    static enum state (* const state[])(struct connection*, int*) = {
        eotx_unknown,
        eotx_noundo,
        eotx_revocable,
        eotx_plain,
        eotx_peereotx,
        eotx_peercommit,
        eotx_peerabort,
        eotx_waitresp,
        eotx_waiteotx,
        eotx_waitcmmt,
        eotx_waitabrt,
        eotx_waitnndo,
        NULL };

    assert(conn);

    int res;

    enum state next = initstatetab[conn->state];

    while (state[next]) {

        fprintf(stderr, "eotx_%s\n", statenametab[next]);
        next = state[next](conn, &res);
    }

    return res;
}

/*
 * Commit
 */

static enum state
commit_unknown(struct connection *conn, int *res)
{
    return any_invalid();
}

static enum state
commit_noundo(struct connection *conn, int *res)
{
    /* Signal to abort transaction, because noundo was requested, the
       transaction will be restarted in exclusive noundo mode */
    *res = ERR_PEERABORT;

    /* Peer is in noundo, so commit always succeeds */
    /*conn->state = CONNSTATE_PEERCOMMIT;
    *res = 0;*/
    return STATE_RETURN;
}

static enum state
commit_revocable(struct connection *conn, int *res)
{
    /* Commit before eotx, abort */
    return any_invalid();
}

static enum state
commit_plain(struct connection *conn, int *res)
{
    *res = 0;
    return STATE_RETURN;
}

static enum state
commit_peereotx(struct connection *conn, int *res)
{
    /* Signal commit to peer, wait for answer */

    if (txproto_send_txcmmt(conn->fd) < 0) {
        abort();
    }

    return STATE_WAITCMMT;
}

static enum state
commit_peercommit(struct connection *conn, int *res)
{
    /* Peer signalled commit, so commit succeeds immediately */

    if (txproto_send_txcmmt(conn->fd) < 0) {
        abort();
    }
    *res = 0;

    return STATE_RETURN;
}

static enum state
commit_peerabort(struct connection *conn, int *res)
{
    /* Peer signalled abort, so commit failes immediately */
    *res = ERR_PEERABORT;
    return STATE_RETURN;
}

static enum state
commit_waitresp(struct connection *conn, int *res)
{
    abort();
    return STATE_RETURN;
}

static enum state
commit_waiteotx(struct connection *conn, int *res)
{
    return any_invalid();
}

static enum state
commit_waitcmmt(struct connection *conn, int *res)
{
    enum state next;

    /* Read data from socket into connection buffer */

    struct packet packet;

    txproto_recv_packet(conn->fd, &packet, 0);

    switch (packet.cmd) {
        case CMD_EMPTY:
            next = STATE_WAITCMMT;
            break;
        case CMD_TXCMMT:
            conn->state = CONNSTATE_PEERCOMMIT;
            *res = 0;
            next = STATE_RETURN;
            break;
        case CMD_TXABRT:
        case CMD_TXNACK:
            conn->state = CONNSTATE_PEERABORT;
            next = STATE_PEERABORT;
            break;
        case CMD_TXDATA:
            if ( (*res = add_data(conn, &packet)) < 0 ) {
                return STATE_RETURN;
            }
            next = STATE_WAITCMMT;
            break;
        case CMD_TXNNDO:
        case CMD_TXCHAL:
        case CMD_TXRESP:
        case CMD_TXEOTX:
        case CMD_PLAIN:
            /* Cannot receive these commands here */
            abort();
    }

    return next;
}

static enum state
commit_waitabrt(struct connection *conn, int *res)
{
    return any_invalid();
}

static enum state
commit_waitnndo(struct connection *conn, int *res)
{
    return any_invalid();
}

int
connection_commit(struct connection *conn)
{
    static enum state (* const state[])(struct connection*, int*) = {
        commit_unknown,
        commit_noundo,
        commit_revocable,
        commit_plain,
        commit_peereotx,
        commit_peercommit,
        commit_peerabort,
        commit_waitresp,
        commit_waiteotx,
        commit_waitcmmt,
        commit_waitabrt,
        commit_waitnndo,
        NULL };

    assert(conn);

    int res;

    enum state next = initstatetab[conn->state];

    while (state[next]) {

        fprintf(stderr, "commit_%s\n", statenametab[next]);
        next = state[next](conn, &res);
    }

    return res;
}

/*
 * Abort
 */

static enum state
abort_unknown(struct connection *conn, int *res)
{
    /* No transmissions yet, so abort succeeds */
    /**res = 0;
    return STATE_RETURN;*/

    if (txproto_send_txchal(conn->fd) < 0) {
        abort();
    }
    return STATE_WAITRESP;
}

static enum state
abort_noundo(struct connection *conn, int *res)
{
    /* Peer is in noundo, so abort always fails */
    return any_invalid();
}

static enum state
abort_revocable(struct connection *conn, int *res)
{
    /* Signal abort to peer, wait for answer */

    if (txproto_send_txabrt(conn->fd) < 0) {
        abort();
    }

    return STATE_WAITABRT;
}

static enum state
abort_plain(struct connection *conn, int *res)
{
    *res = 0;
    return STATE_WANTPLAIN;
    /* Peer is running plain (and therefore noundo), so abort always fails */
    //return any_invalid();
}

static enum state
abort_peereotx(struct connection *conn, int *res)
{
    /* Signal abort to peer, wait for answer */

    if (txproto_send_txabrt(conn->fd) < 0) {
        abort();
    }

    return STATE_WAITABRT;
}

static enum state
abort_peercommit(struct connection *conn, int *res)
{
    /* Peer signalled commit, so abort fails immediately */
    return any_invalid();
}

static enum state
abort_peerabort(struct connection *conn, int *res)
{
    /* Peer signalled abort, so abort succeeds immediately */
    if (txproto_send_txabrt(conn->fd) < 0) {
        abort();
    }
    *res = 0;
    return STATE_RETURN;
}

static enum state
abort_waitresp(struct connection *conn, int *res)
{
    /* FIXME: Cannot get into this state */

    enum state next;

    /* Read data from socket into connection buffer */

    struct packet packet;

    txproto_recv_packet(conn->fd, &packet, 0);

    switch (packet.cmd) {
        case CMD_EMPTY:
            next = STATE_WAITRESP;
            break;
        case CMD_PLAIN:
            add_data(conn, &packet);
            next = STATE_NOUNDO;
            break;
        case CMD_TXCHAL:
            conn->state = CONNSTATE_REVOCABLE;
            next = STATE_REVOCABLE;
            break;
        case CMD_TXRESP:
            conn->state = CONNSTATE_REVOCABLE;
            next = STATE_REVOCABLE;
            break;
        case CMD_TXABRT:
        case CMD_TXCMMT:
        case CMD_TXDATA:
        case CMD_TXEOTX:
        case CMD_TXNNDO:
        case CMD_TXNACK:
            /* Cannot receive these commands here */
            abort();
    }

    return next;
}

static enum state
abort_waiteotx(struct connection *conn, int *res)
{
    return any_invalid();
}

static enum state
abort_waitcmmt(struct connection *conn, int *res)
{
    return any_invalid();
}

static enum state
abort_waitabrt(struct connection *conn, int *res)
{
    enum state next;

    /* Read data from socket into connection buffer */

    struct packet packet;

    txproto_recv_packet(conn->fd, &packet, 0);

    switch (packet.cmd) {
        case CMD_EMPTY:
            next = STATE_WAITABRT;
            break;
        case CMD_TXABRT:
        case CMD_TXNACK:
            /* Abort received, therefore abort succeeds */
            *res = 0;
            next = STATE_RETURN;
            break;
        case CMD_TXCMMT:
        case CMD_TXDATA:
        case CMD_TXEOTX:
        case CMD_TXNNDO:
            /* Consume these packets */
            next = STATE_WAITABRT;
            break;
        case CMD_TXCHAL:
        case CMD_TXRESP:
        case CMD_PLAIN:
            /* Cannot receive these commands here */
            abort();
    }

    return next;
}

static enum state
abort_waitnndo(struct connection *conn, int *res)
{
    return any_invalid();
}

int
connection_abort(struct connection *conn)
{
    static enum state (* const state[])(struct connection*, int*) = {
        abort_unknown,
        abort_noundo,
        abort_revocable,
        abort_plain,
        abort_peereotx,
        abort_peercommit,
        abort_peerabort,
        abort_waitresp,
        abort_waiteotx,
        abort_waitcmmt,
        abort_waitabrt,
        abort_waitnndo,
        NULL };

    assert(conn);

    int res;

    enum state next = initstatetab[conn->state];

    while (state[next]) {

        fprintf(stderr, "abort_%s\n", statenametab[next]);
        next = state[next](conn, &res);
    }

    return res;
}

/*
 * Noundo
 */

static enum state
noundo_unknown(struct connection *conn, int *res)
{
    /* No transmissions yet, so abort succeeds */
    /**res = 0;
    return STATE_RETURN;*/

    if (txproto_send_txchal(conn->fd) < 0) {
        abort();
    }
    return STATE_WAITRESP;
}

static enum state
noundo_noundo(struct connection *conn, int *res)
{
    /* Peer is waiting in noundo, therefore signal answer and succeeds */

    if (txproto_send_txnndo(conn->fd) < 0) {
        abort();
    }
    *res = 0;

    return STATE_RETURN;
}

static enum state
noundo_revocable(struct connection *conn, int *res)
{
    /* Signal abort to peer, wait for answer */

    if (txproto_send_txnndo(conn->fd) < 0) {
        abort();
    }

    return STATE_WAITNNDO;
}

static enum state
noundo_plain(struct connection *conn, int *res)
{
    *res = 0;
    return STATE_RETURN;
}

static enum state
noundo_peereotx(struct connection *conn, int *res)
{
    /* Signal abort to peer, wait for answer */
    if (txproto_send_txnndo(conn->fd) < 0) {
        abort();
    }

    return STATE_WAITNNDO;
}

static enum state
noundo_peercommit(struct connection *conn, int *res)
{
    /* Peer signalled commit, so abort fails immediately */
    return any_invalid();
}

static enum state
noundo_peerabort(struct connection *conn, int *res)
{
    /* Peer signalled abort, so noundo fails immediately */
    *res = ERR_PEERABORT;
    return STATE_RETURN;
}

static enum state
noundo_waitresp(struct connection *conn, int *res)
{
    /* FIXME: Cannot get into this state */

    enum state next;

    /* Read data from socket into connection buffer */

    struct packet packet;

    txproto_recv_packet(conn->fd, &packet, 0);

    switch (packet.cmd) {
        case CMD_EMPTY:
            next = STATE_WAITRESP;
            break;
        case CMD_PLAIN:
            add_data(conn, &packet);
            *res = ERR_NOUNDO;
            conn->state = CONNSTATE_WANTPLAIN;
            next = STATE_WANTPLAIN;
            break;
        case CMD_TXCHAL:
            conn->state = CONNSTATE_REVOCABLE;
            next = STATE_REVOCABLE;
            break;
        case CMD_TXRESP:
            conn->state = CONNSTATE_REVOCABLE;
            next = STATE_REVOCABLE;
            break;
        case CMD_TXABRT:
        case CMD_TXCMMT:
        case CMD_TXDATA:
        case CMD_TXEOTX:
        case CMD_TXNACK:
        case CMD_TXNNDO:
            /* Cannot receive these commands here */
            abort();
    }

    return next;
}

static enum state
noundo_waiteotx(struct connection *conn, int *res)
{
    return any_invalid();
}

static enum state
noundo_waitcmmt(struct connection *conn, int *res)
{
    return any_invalid();
}

static enum state
noundo_waitabrt(struct connection *conn, int *res)
{
    return any_invalid();
}

static enum state
noundo_waitnndo(struct connection *conn, int *res)
{
    enum state next;

    /* Read data from socket into connection buffer */

    struct packet packet;

    txproto_recv_packet(conn->fd, &packet, 0);

    switch (packet.cmd) {
        case CMD_EMPTY:
            next = STATE_WAITABRT;
            break;
        case CMD_TXABRT:
            /* Abort signalled, therefore noundo fails */
            *res = ERR_PEERABORT;
            next = STATE_RETURN;
            break;
        case CMD_TXDATA:
        case CMD_TXEOTX:
            /* Consume packets */
            next = STATE_WAITNNDO;
            break;
        case CMD_TXNNDO:
            /* Received noundo, therefore return success */
            conn->state = CONNSTATE_PEERENDOFTX;
            next = STATE_RETURN;
            break;
        case CMD_TXCHAL:
        case CMD_TXRESP:
        case CMD_PLAIN:
        case CMD_TXCMMT:
        case CMD_TXNACK:
            /* Cannot receive these commands here */
            abort();
    }

    return next;
}

int
connection_noundo(struct connection *conn)
{
    static enum state (* const state[])(struct connection*, int*) = {
        noundo_unknown,
        noundo_noundo,
        noundo_revocable,
        noundo_plain,
        noundo_peereotx,
        noundo_peercommit,
        noundo_peerabort,
        noundo_waitresp,
        noundo_waiteotx,
        noundo_waitcmmt,
        noundo_waitabrt,
        noundo_waitnndo,
        NULL };

    assert(conn);

    int res;

    enum state next = initstatetab[conn->state];

    while (state[next]) {

        fprintf(stderr, "noundo_%s\n", statenametab[next]);
        next = state[next](conn, &res);
    }

    return res;
}

