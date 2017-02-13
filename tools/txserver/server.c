
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "connection.h"

static int
exec_cmd(void)
{
    return rand()%5;
}

int
run_server(const struct sockaddr_in *address)
{
	int lisn;
    int sock;

    /* Socket setup */

	if ((lisn = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return -1;
	}

	if (bind(lisn, (const struct sockaddr*)address, sizeof(*address)) < 0) {
		perror("bind");
		return -1;
	}

	if (listen(lisn, SOMAXCONN) < 0) {
		perror("listen");
		return -1;
	}

    /* Serving connections */

    do {
        struct sockaddr conn_addr;
        socklen_t conn_addrlen;
        char buffer[32];
        size_t size;

        conn_addrlen = 0;

        if ((sock = accept(lisn, &conn_addr, &conn_addrlen)) < 0) {
            perror("accept");
            return -1;
        }

        struct connection *conn = connection_create(sock);

        if (!conn) {
            return -1;
        }

        do {
            memset(buffer, 0, sizeof(buffer));
            size = 0;

            if (exec_cmd()) {
                do {
                    size = connection_recv(conn, buffer, sizeof(buffer));

                    if (size == ERR_NOUNDO) {
                        connection_noundo(conn);
                    }
                } while (size == ERR_NOUNDO);

                switch (size) {
                    case ERR_PEERABORT:
                        if (connection_abort(conn) < 0) {
                            return -1;
                        }
                        goto cleanup;
                    case ERR_SYSTEM:
                        return -1;
                    default:
                        break;
                }
            } else {
                if (connection_abort(conn) < 0) {
                    return -1;
                }
                goto cleanup;
            }

            /*if ((size = recv(sock, buffer, sizeof(buffer), 0)) < 0) {
                perror("recv");
                return -1;
            }*/

            fprintf(stdout, "size=%d ", (int)size);
            fflush(stdout);
            fprintf(stdout, "<%.*s>\n", (int)size, buffer);
            fflush(stdout);
        } while (size);

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

        /*if (shutdown(sock, SHUT_RDWR) < 0) {
            perror("shutdown");
            return -1;
        }*/

        if (close(sock) < 0) {
            perror("close");
            return -1;
        }

        fprintf(stdout, "\n");

    } while (1);

    if (close(lisn) < 0) {
        perror("close");
        return -1;
    }

    return 0;
}

