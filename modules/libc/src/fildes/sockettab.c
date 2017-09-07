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

#include "sockettab.h"
#include <errno.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-array.h>
#include <picotm/picotm-lib-tab.h>
#include <picotm/picotm-module.h>
#include <pthread.h>
#include "range.h"
#include "socket.h"

static struct socket    sockettab[MAXNUMFD];
static size_t           sockettab_len = 0;
static pthread_rwlock_t sockettab_rwlock = PTHREAD_RWLOCK_INITIALIZER;

/* Destructor */

static void sockettab_uninit(void) __attribute__ ((destructor));

static size_t
sockettab_socket_uninit_walk(void* socket, struct picotm_error* error)
{
    socket_uninit(socket);
    return 1;
}

static void
sockettab_uninit(void)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_tabwalk_1(sockettab, sockettab_len, sizeof(sockettab[0]),
                     sockettab_socket_uninit_walk, &error);

    pthread_rwlock_destroy(&sockettab_rwlock);
}

/* End of destructor */

static void
rdlock_sockettab(struct picotm_error* error)
{
    int err = pthread_rwlock_rdlock(&sockettab_rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
wrlock_sockettab(struct picotm_error* error)
{
    int err = pthread_rwlock_wrlock(&sockettab_rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
unlock_sockettab(void)
{
    do {
        int err = pthread_rwlock_unlock(&sockettab_rwlock);
        if (err) {
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_errno(&error, err);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
            continue;
        }
        break;
    } while (true);
}

/* requires a writer lock */
static struct socket*
append_empty_socket(struct picotm_error* error)
{
    if (sockettab_len == picotm_arraylen(sockettab)) {
        /* Return error if not enough ids available */
        picotm_error_set_conflicting(error, NULL);
        return NULL;
    }

    struct socket* socket = sockettab + sockettab_len;

    socket_init(socket, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    ++sockettab_len;

    return socket;
}

/* requires reader lock */
static struct socket*
find_by_id(const struct file_id* id)
{
    struct socket *socket_beg = picotm_arraybeg(sockettab);
    const struct socket* socket_end = picotm_arrayat(sockettab,
                                                     sockettab_len);

    while (socket_beg < socket_end) {

        const int cmp = socket_cmp_and_ref(socket_beg, id);
        if (!cmp) {
            return socket_beg;
        }

        ++socket_beg;
    }

    return NULL;
}

/* requires writer lock */
static struct socket*
search_by_id(const struct file_id* id, int fildes, struct picotm_error* error)
{
    struct socket* socket_beg = picotm_arraybeg(sockettab);
    const struct socket* socket_end = picotm_arrayat(sockettab,
                                                     sockettab_len);

    while (socket_beg < socket_end) {

        const int cmp = socket_cmp_and_ref_or_set_up(socket_beg, id, fildes,
                                                      error);
        if (!cmp) {
            if (picotm_error_is_set(error)) {
                return NULL;
            }
            return socket_beg; /* set-up socket structure; return */
        }

        ++socket_beg;
    }

    return NULL;
}

struct socket*
sockettab_ref_fildes(int fildes, struct picotm_error* error)
{
    struct file_id id;
    file_id_init_from_fildes(&id, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    /* Try to find an existing socket structure with the given id.
     */

    rdlock_sockettab(error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct socket* socket = find_by_id(&id);
    if (socket) {
        goto unlock; /* found socket for id; return */
    }

    unlock_sockettab();

    /* Not found entry; acquire writer lock to create a new entry in
     * the socket table.
     */

    wrlock_sockettab(error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    /* Re-try find operation; maybe element was added meanwhile. */
    socket = find_by_id(&id);
    if (socket) {
        goto unlock; /* found socket for id; return */
    }

    /* No entry with the id exists; try to set up an existing, but
     * currently unused, socket structure.
     */

    struct file_id empty_id;
    file_id_clear(&empty_id);

    socket = search_by_id(&empty_id, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_search_by_id;
    } else if (socket) {
        goto unlock; /* found socket for id; return */
    }

    /* The socket table is full; create a new entry for the socket id at the
     * end of the table.
     */

    socket = append_empty_socket(error);
    if (picotm_error_is_set(error)) {
        goto err_append_empty_socket;
    }

    /* To perform the setup, we must have acquired a writer lock at
     * this point. No other transaction may interfere. */
    socket_ref_or_set_up(socket, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_socket_ref_or_set_up;
    }

unlock:
    unlock_sockettab();

    return socket;

err_socket_ref_or_set_up:
err_append_empty_socket:
err_search_by_id:
    unlock_sockettab();
    return NULL;
}

size_t
sockettab_index(struct socket* socket)
{
    return socket - sockettab;
}
