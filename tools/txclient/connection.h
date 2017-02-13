
#ifndef CONNECTION_H
#define CONNECTION_H

enum error_code
{
    ERR_SYSTEM = -1,
    ERR_NOUNDO = -2,
    /*ERR_PLAIN = -3,*/
    ERR_PEERABORT = -4
};

enum connection_state
{
    CONNSTATE_UNKNOWN = 0,
    CONNSTATE_NOUNDO,
    CONNSTATE_REVOCABLE,
    CONNSTATE_WANTPLAIN,
    CONNSTATE_PEERENDOFTX,
    CONNSTATE_PEERCOMMIT,
    CONNSTATE_PEERABORT
};

struct connection;

struct connection *
connection_create(int fd);

void
connection_destroy(struct connection *conn);

int
connection_init(struct connection *conn, int fd);

void
connection_uninit(struct connection *conn);

ssize_t
connection_send(struct connection *conn, const void *buffer, size_t size);

ssize_t
connection_recv(struct connection *conn, void *buffer, size_t size);

int
connection_eotx(struct connection *conn);

int
connection_commit(struct connection *conn);

int
connection_abort(struct connection *conn);

int
connection_noundo(struct connection *conn);

#endif

