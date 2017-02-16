/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
/*#include <errno.h>*/
#include <netinet/in.h>
/*#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>*/


#include <tanger-stm.h>

#include <tanger-stm-internal.h>
#include <tanger-stm-internal-extact.h>

/*static tanger_stm_tx_t* tanger_wrapperpure_tanger_stm_get_tx(void)                __attribute__ ((weakref("tanger_stm_get_tx")));
static int              tanger_wrapperpure_tanger_stm_go_noundo(tanger_stm_tx_t*) __attribute__ ((weakref("tanger_stm_go_noundo")));
static void             tanger_wrapperpure_tanger_stm_abort_self(tanger_stm_tx_t*) __attribute__ ((weakref("tanger_stm_abort_self")));*/

#include <tanger-stm-std-errno.h>
#include <tanger-stm-std-stdio.h>
#include <tanger-stm-std-stdlib.h>
#include <tanger-stm-std-string.h>
#include <tanger-stm-std-sys-socket.h>
#include <tanger-stm-std-unistd.h>
#include "socket_test.h"

static const char msg[] = "Hello world!";

void
getaddress(struct sockaddr_in *address)
{
    assert(address);

    memset(address, 0, sizeof(*address));
	address->sin_family = AF_INET;
	address->sin_addr.s_addr = INADDR_ANY;
	address->sin_port = IPPORT_USERRESERVED;
}

void
tanger_stm_socket_test_1(unsigned int tid)
{
    struct sockaddr_in address;
    int sock;

    getaddress(&address);

    sleep(1);

    tanger_begin();

    /* Socket setup */

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		abort();
	}

    /* Connecting to server */

    if (connect(sock, (const struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("connect");
        abort();
    }

    /* Sending data */

    ssize_t res;

    do {
        res = send(sock, msg, strlen(msg), 0);
    } while ((res < 0) && (errno == EINTR));

    if (res < 0) {
        perror("send");
        abort();
    }

    do {
        res = send(sock, msg, strlen(msg), 0);
    } while ((res < 0) && (errno == EINTR));

    if (res < 0) {
        perror("send");
        abort();
    }

    /* Closing connection */

    if (shutdown(sock, SHUT_RDWR) < 0) {
        perror("shutdown");
        abort();
    }

    if (close(sock) < 0) {
        perror("close");
        abort();
    }

    tanger_commit();
}

void
tanger_stm_socket_test_2(unsigned int tid)
{
    struct sockaddr_in address;
	int lisn;
    int sock;

    getaddress(&address);

    /* Socket setup */

	if ((lisn = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
        abort();
	}

    int val = 1;
    setsockopt(lisn, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

	if (bind(lisn, (const struct sockaddr*)&address, sizeof(address)) < 0) {
		perror("bind");
        abort();
	}

    tanger_begin();

	if (listen(lisn, SOMAXCONN) < 0) {
		perror("listen");
        abort();
	}

    /* Serving connection */

    struct sockaddr conn_addr;
    socklen_t conn_addrlen;
    char buffer[32];
    size_t size;

    conn_addrlen = 0;

    if ((sock = accept(lisn, &conn_addr, &conn_addrlen)) < 0) {
        perror("accept");
        abort();
    }

    do {
        memset(buffer, 0, sizeof(buffer));
        size = 0;

        if ((size = recv(sock, buffer, sizeof(buffer), 0)) < 0) {
            perror("recv");
            abort();
        }

        fprintf(stderr, "size=%d <%.*s>\n", (int)size, (int)size, buffer);
    } while (size);

    if (close(sock) < 0) {
        perror("close");
        abort();
    }

    fprintf(stdout, "\n");

    if (shutdown(lisn, SHUT_RDWR) < 0) {
        perror("shutdown");
        abort();
    }

    if (close(lisn) < 0) {
        perror("close");
        abort();
    }

    tanger_commit();
}

void
tanger_stm_socket_test_3(unsigned int tid)
{
    struct sockaddr_in address;
    int sock;

    getaddress(&address);

    /* Socket setup */

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		abort();
	}

    /* Connecting to server */

    tanger_begin();

    if (connect(sock, (const struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("connect");
        abort();
    }

    sleep(1);

    /* Sending data */

    ssize_t res;

    do {
        res = send(sock, msg, strlen(msg), 0);
    } while ((res < 0) && (errno == EINTR));

    if (res < 0) {
        perror("send");
        abort();
    }

    do {
        res = send(sock, msg, strlen(msg), 0);
    } while ((res < 0) && (errno == EINTR));

    if (res < 0) {
        perror("send");
        abort();
    }

    /* Closing connection */

    if (shutdown(sock, SHUT_RDWR) < 0) {
        perror("shutdown");
        abort();
    }

    tanger_commit();

    if (close(sock) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_socket_test_4(unsigned int tid)
{
    struct sockaddr_in address;
	int lisn;
    int sock;

    getaddress(&address);

    /* Socket setup */

	if ((lisn = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
        abort();
	}

	if (bind(lisn, (const struct sockaddr*)&address, sizeof(address)) < 0) {
		perror("bind");
        abort();
	}

	if (listen(lisn, SOMAXCONN) < 0) {
		perror("listen");
        abort();
	}

    /* Serving connection */

    struct sockaddr conn_addr;
    socklen_t conn_addrlen;
    char buffer[32];
    size_t size;

    conn_addrlen = 0;

    if ((sock = accept(lisn, &conn_addr, &conn_addrlen)) < 0) {
        perror("accept");
        abort();
    }

    do {
        memset(buffer, 0, sizeof(buffer));
        size = 0;

        if ((size = recv(sock, buffer, sizeof(buffer), 0)) < 0) {
            perror("recv");
            abort();
        }

        fprintf(stdout, "size=%d ", (int)size);
        fflush(stdout);
        fprintf(stdout, "<%.*s>\n", (int)size, buffer);
        fflush(stdout);
    } while (size);

    if (close(sock) < 0) {
        perror("close");
        abort();
    }

    fprintf(stdout, "\n");

    if (close(lisn) < 0) {
        perror("close");
        abort();
    }
}

