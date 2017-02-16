/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

extern int run_server(const struct sockaddr_in *address);

int
main(int argc, char **argv)
{
    pid_t pid;

	struct sockaddr_in address_in;

    memset(&address_in, 0, sizeof(address_in));
	address_in.sin_family = AF_INET;
	address_in.sin_addr.s_addr = INADDR_ANY;
	address_in.sin_port = IPPORT_USERRESERVED;

    if (run_server(&address_in) < 0) {
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

