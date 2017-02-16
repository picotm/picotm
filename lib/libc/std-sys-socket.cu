/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

ceuta_hdrl(#ifndef TANGER_STM_SYS_SOCKET_H);
ceuta_hdrl(#define TANGER_STM_SYS_SOCKET_H);
ceuta_hdrl(#include <sys/socket.h>);

#include <sys/types.h>
#include <sys/socket.h>

extern int     com_fd_tx_accept(int, struct sockaddr*, socklen_t*);
extern int     com_fd_tx_bind(int, const struct sockaddr*, socklen_t);
extern int     com_fd_tx_connect(int, const struct sockaddr*, socklen_t);
extern int     com_fd_tx_listen(int, int);
extern ssize_t com_fd_tx_send(int, const void*, size_t, int);
extern int     com_fd_tx_shutdown(int, int);
extern int     com_fd_tx_socket(int, int, int);
extern ssize_t com_fd_tx_recv(int, void*, size_t, int);

ceuta_wrap(int,     accept,   com_fd_tx_accept,   int socket, [siz=*address_len|in|out] struct sockaddr *address, [in|out] socklen_t *address_len);
ceuta_wrap(int,     bind,     com_fd_tx_bind,     int socket, [siz=address_len|in] const struct sockaddr *address, socklen_t address_len);
ceuta_wrap(int,     connect,  com_fd_tx_connect,  int sockfd, [siz=address_len|in] const struct sockaddr *address, socklen_t address_len);
ceuta_wrap(int,     listen,   com_fd_tx_listen,   int sockfd, int backlog);
ceuta_wrap(ssize_t, send,     com_fd_tx_send,     int s,      [siz=length|in] const void *buffer, size_t length, int flags);
ceuta_wrap(int,     shutdown, com_fd_tx_shutdown, int s,      int how);
ceuta_wrap(int,     socket,   com_fd_tx_socket,   int domain, int type, int protocol);
ceuta_wrap(ssize_t, recv,     com_fd_tx_recv,     int s,      [siz=length|out] void *buffer, size_t length, int flags);

ceuta_hdrl(#endif);

