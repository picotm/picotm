/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include "connection.h"

static const char msg[] = "Hello world!";

static int
exec_cmd(void)
{
    return rand()%5;
}

int
run_client(const struct sockaddr_in *address)
{
    int i;

    for (i = 0; i < 10; /*++i*/) {

//        sleep(1);

        int sock;

        /* Socket setup */

    	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    		perror("socket");
    		return -1;
    	}

        /* Connecting to server */

        int err;

        do {
            err = connect(sock, (const struct sockaddr*)address, sizeof(*address));
        } while ((err < 0) && (errno == ECONNREFUSED || errno == ECONNRESET));

        if (err < 0) {
            fprintf(stderr, "errno=%d ", errno);
            perror("connect");
            return -1;
        }

        struct connection *conn = connection_create(sock);

        if (!conn) {
            return -1;
        }

        fprintf (stderr, "*** Client: Beginning tx ***\n");

        if (exec_cmd()) {

            ssize_t res;

            do {
                res = connection_send(conn, msg, strlen(msg));

                if (res == ERR_NOUNDO) {
                    connection_noundo(conn);
                }
            } while (res == ERR_NOUNDO);

            switch (res) {
                case ERR_PEERABORT:
                    if (connection_abort(conn) < 0) {
                        return -1;
                    }
                    goto cleanup;
                case ERR_SYSTEM:
                    return res;
                default:
                    break;
            }
        } else {
            if (connection_abort(conn) < 0) {
                return -1;
            }
            goto cleanup;
        }

        if (exec_cmd()) {
            ssize_t res = connection_send(conn, msg, strlen(msg));

            switch (res) {
                case ERR_PEERABORT:
                    if (connection_abort(conn) < 0) {
                        return -1;
                    }
                    goto cleanup;
                case ERR_SYSTEM:
                    return res;
                default:
                    break;
            }
        } else {
            if (connection_abort(conn) < 0) {
                return -1;
            }
            goto cleanup;
        }

        if (exec_cmd()) {
            int res;

            do {
                res = connection_eotx(conn);

                if (res == ERR_NOUNDO) {
                    connection_noundo(conn);
                }
            } while (res == ERR_NOUNDO);

            switch (res) {
                case ERR_PEERABORT:
                    if (connection_abort(conn) < 0) {
                        return -1;
                    }
                    goto cleanup;
                case ERR_SYSTEM:
                    return res;
                default:
                    break;
            }
        } else {
            if (connection_abort(conn) < 0) {
                return -1;
            }
            goto cleanup;
        }

        if (exec_cmd()) {
            int res = connection_commit(conn);

            switch (res) {
                case ERR_PEERABORT:
                    if (connection_abort(conn) < 0) {
                        return -1;
                    }
                    goto cleanup;
                case ERR_SYSTEM:
                    return res;
                default:
                    break;
            }
        } else {
            if (connection_abort(conn) < 0) {
                return -1;
            }
            goto cleanup;
        }

        cleanup:

        connection_destroy(conn);

        if (shutdown(sock, SHUT_RDWR) < 0) {
            perror("shutdown");
            //return -1;
        }

        if (close(sock) < 0) {
            perror("close");
            return -1;
        }
    }

    return 0;
}

