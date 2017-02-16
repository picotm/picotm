/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CONNECTION_H
#define CONNECTION_H

enum connection_state
{
    CONNSTATE_UNKNOWN = 0,
    CONNSTATE_NOUNDO,
    CONNSTATE_REVOCABLE,
    CONNSTATE_WANTPLAIN,
    CONNSTATE_PEERENDOFTX,
    CONNSTATE_PEERCOMMIT,
    CONNSTATE_PEERABORT,
    CONNSTATE_PEERNOUNDO
};

struct connection;

struct connection *
connection_create(int fd);

void
connection_destroy(struct connection *conn);

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

