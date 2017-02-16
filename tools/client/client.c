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

static const char msg[] = "Hello world!";

int
run_client(const struct sockaddr_in *address)
{
    int i;

//    srand(2);

//    sleep(1);

    for (i = 0; i < 10; /*++i*/) {

        int sock;

        /* Socket setup */

    	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    		perror("socket");
    		return -1;
    	}

        /* Connecting to server */

        if (connect(sock, (const struct sockaddr*)address, sizeof(*address)) < 0) {
            perror("connect");
            return -1;
        }

//        sleep(2);

        /*send(sock, msg, strlen(msg), 0);
        send(sock, msg, strlen(msg), 0);*/

        ssize_t res;

        do {
            res = send(sock, msg, strlen(msg), 0);
        } while ((res < 0) && (errno == EINTR));

        do {
            res = send(sock, msg, strlen(msg), 0);
        } while ((res < 0) && (errno == EINTR));

        if (shutdown(sock, SHUT_RDWR) < 0) {
            perror("shutdown");
            return -1;
        }

        if (close(sock) < 0) {
            perror("close");
            return -1;
        }
    }

    return 0;
}

