/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

ceuta_hdrl(#ifndef TANGER_STM_STD_SYS_SELECT_H);
ceuta_hdrl(#define TANGER_STM_STD_SYS_SELECT_H);
ceuta_hdrl(#include <sys/select.h>);

#include <sys/select.h>

extern int com_fd_tx_select(int nfds, fd_set *readfds,
                                      fd_set *writefds,
                                      fd_set *errorfds,
                                      struct timeval *timeout);

ceuta_wrap(int, select, com_fd_tx_select, int nfds, [in|out] fd_set *readfds, [in|out] fd_set *writefds, [in|out] fd_set *errorfds, [in|out] struct timeval *timeout);

ceuta_hdrl(#endif);

