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

#ifndef FD_H
#define FD_H

#include <picotm/picotm-lib-rwlock.h>
#include <picotm/picotm-lib-shared-ref-obj.h>

/**
 * \cond impl || libc_impl || libc_impl_fd
 * \ingroup libc_impl
 * \ingroup libc_impl_fd
 * \file
 * \endcond
 */

struct picotm_error;
struct picotm_rwstate;

union fcntl_arg;

enum fd_state
{
    FD_ST_UNUSED = 0, /** \brief File descriptor is currenty not in use */
    FD_ST_INUSE, /** \brief File descriptor is open */
    FD_ST_CLOSING /** \brief File descriptor has been closed by transaction; other users need to abort */
};

/**
 * Enumerates fields of `struct fd`.
 */
enum fd_field {
    FD_FIELD_STATE,
    NUMBER_OF_FD_FIELDS
};

struct fd {

    /** Reference-counting base object. */
    struct picotm_shared_ref16_obj ref_obj;

    int fildes;
    enum fd_state state;

    /** File-descriptor reader/writer locks */
    struct picotm_rwlock rwlock[NUMBER_OF_FD_FIELDS];
};

/** \brief Init global file-descriptor state*/
void
fd_init(struct fd *fd, struct picotm_error* error);

/** \brief Uninit state */
void
fd_uninit(struct fd *fd);

/**
 * Tries to acquires a reader lock on a file descriptor.
 *
 * \param       fd      The file-descriptor structure
 * \param       field   The reader lock's field.
 * \param       rwstate The transaction's reader-writer state.
 * \param[out]  error   Returns an error.
 */
void
fd_try_rdlock_field(struct fd *fd, enum fd_field field,
                    struct picotm_rwstate* state,
                    struct picotm_error* error);

/**
 * Tries to acquires a writer lock on a file descriptor.
 *
 * \param       fd      The file-descriptor structure
 * \param       field   The writer lock's field.
 * \param       rwstate The transaction's reader-writer state.
 * \param[out]  error   Returns an error.
 */
void
fd_try_wrlock_field(struct fd *fd, enum fd_field field,
                    struct picotm_rwstate* state,
                    struct picotm_error* error);

/**
 * Releases a reader/writer lock.
 *
 * \param   fd      The file-descriptor structure
 * \param   field   The reader/writer lock's field.
 * \param   rwstate The transaction's reader-writer state.
 */
void
fd_unlock_field(struct fd *fd, enum fd_field field,
                struct picotm_rwstate* state);

/** \brief Validates a file descriptor. */
void
fd_validate(struct fd* fd, struct picotm_error* error);

/** \brief Aquires a reference on the file dscriptor */
void
fd_ref(struct fd *fd, struct picotm_error* error);

/** \brief Aquires a reference on the file descriptor */
void
fd_ref_or_set_up(struct fd *fd, int fildes, struct picotm_error* error);

/** \brief Releases a reference on the file descriptor */
void
fd_unref(struct fd *fd);

/** \brief Return non-zero value if file-descriptor is open */
int
fd_is_open_nl(const struct fd *fd);

/** \brief Set file descriptor to state close */
void
fd_close(struct fd *fd);

/** \brief Dump file-descriptor state to stderr */
void
fd_dump(const struct fd *fd);

/** Set file-descriptor flags. */
int
fd_setfd(struct fd* fd, int arg, struct picotm_error* error);

/** Get file-descriptor flags. */
int
fd_getfd(struct fd* fd, struct picotm_error* error);

#endif
