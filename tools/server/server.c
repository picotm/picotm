/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

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

        do {
            memset(buffer, 0, sizeof(buffer));
            size = 0;

            if ((size = recv(sock, buffer, sizeof(buffer), 0)) < 0) {
                perror("recv");
                return -1;
            }

            fprintf(stdout, "size=%d ", (int)size);
            fflush(stdout);
            fprintf(stdout, "<%.*s>\n", (int)size, buffer);
            fflush(stdout);
        } while (size);

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

