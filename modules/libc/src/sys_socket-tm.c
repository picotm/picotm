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

#include "picotm/sys/socket-tm.h"
#include <errno.h>
#include <picotm/picotm-module.h>
#include "error/module.h"
#include "fildes/module.h"

static bool
perform_recovery(int errno_hint)
{
    enum picotm_libc_error_recovery recovery =
        picotm_libc_get_error_recovery();

    if (recovery == PICOTM_LIBC_ERROR_RECOVERY_FULL) {
        return true;
    }

    return (errno_hint != EAGAIN) && (errno_hint != EWOULDBLOCK);
}

PICOTM_EXPORT
int
accept_tm(int socket, struct sockaddr* address, socklen_t* address_len)
{
    error_module_save_errno();

    int res;

    do {
        res = fildes_module_accept(socket, address, address_len);
        if (res < 0) {
            if (perform_recovery(errno)) {
                picotm_recover_from_errno(errno);
            } else {
                return res;
            }
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
int
bind_tm(int socket, const struct sockaddr* address, socklen_t address_len)
{
    error_module_save_errno();

    int res;

    do {
        res = fildes_module_bind(socket, address, address_len);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
int
connect_tm(int socket, const struct sockaddr* address, socklen_t address_len)
{
    error_module_save_errno();

    int res;

    do {
        res = fildes_module_connect(socket, address, address_len);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
ssize_t
send_tm(int socket, const void* buffer, size_t length, int flags)
{
    error_module_save_errno();

    ssize_t res;

    do {
        res = fildes_module_send(socket, buffer, length, flags);
        if (res < 0) {
            if (perform_recovery(errno)) {
                picotm_recover_from_errno(errno);
            } else {
                return res;
            }
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
ssize_t
recv_tm(int socket, void* buffer, size_t length, int flags)
{
    error_module_save_errno();

    ssize_t res;

    do {
        res = fildes_module_recv(socket, buffer, length, flags);
        if (res < 0) {
            if (perform_recovery(errno)) {
                picotm_recover_from_errno(errno);
            } else {
                return res;
            }
        }
    } while (res < 0);

    return res;
}
