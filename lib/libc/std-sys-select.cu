/* Copyright (C) 2009  Thomas Zimmermann
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

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

