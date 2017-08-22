/* Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "picotm/sys/socket.h"
#include <errno.h>
#include <picotm/picotm-module.h>
#include <picotm/picotm-tm.h>
#include "error/module.h"
#include "fildes/module.h"
#include "picotm/sys/socket-tm.h"

PICOTM_EXPORT
int
accept_tx(int socket, struct sockaddr* address, socklen_t* address_len)
{
    if (address) {
        privatize_tx(address_len, sizeof(*address_len), PICOTM_TM_PRIVATIZE_STORE);
        privatize_tx(address, *address_len, PICOTM_TM_PRIVATIZE_LOADSTORE);
    }
    return accept_tm(socket, address, address_len);
}

PICOTM_EXPORT
int
bind_tx(int socket, const struct sockaddr* address, socklen_t address_len)
{
    privatize_tx(address, address_len, PICOTM_TM_PRIVATIZE_LOAD);
    return bind_tm(socket, address, address_len);
}

PICOTM_EXPORT
int
connect_tx(int socket, const struct sockaddr* address, socklen_t address_len)
{
    privatize_tx(address, address_len, PICOTM_TM_PRIVATIZE_LOAD);
    return connect_tm(socket, address, address_len);
}

PICOTM_EXPORT
int
listen_tx(int socket, int backlog)
{
    error_module_save_errno();

    int res;

    do {
        res = fd_module_listen(socket, backlog);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
ssize_t
send_tx(int socket, const void* buffer, size_t length, int flags)
{
    privatize_tx(buffer, length, PICOTM_TM_PRIVATIZE_LOAD);
    return send_tm(socket, buffer, length, flags);
}

PICOTM_EXPORT
int
shutdown_tx(int socket, int how)
{
    error_module_save_errno();

    int res;

    do {
        res = fd_module_shutdown(socket, how);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
int
socket_tx(int domain, int type, int protocol)
{
    error_module_save_errno();

    int res;

    do {
        res = fd_module_socket(domain, type, protocol);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
ssize_t
recv_tx(int socket, void* buffer, size_t length, int flags)
{
    privatize_tx(buffer, length, PICOTM_TM_PRIVATIZE_STORE);
    return recv_tm(socket, buffer, length, flags);
}


