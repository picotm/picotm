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

#include "fdtab.h"
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-tab.h>
#include <stdlib.h>

struct fd fdtab[MAXNUMFD];

/* Initializer */

static void fdtab_init(void) __attribute__((constructor));

static size_t
fdtab_fd_init_walk(void *fd, struct picotm_error* error)
{
    fd_init(fd, error);
    if (picotm_error_is_set(error)) {
        return 0;
    }
    return 1;
}

static void
fdtab_init(void)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_tabwalk_1(fdtab, sizeof(fdtab)/sizeof(fdtab[0]), sizeof(fdtab[0]),
                     fdtab_fd_init_walk, &error);
    if (picotm_error_is_set(&error)) {
        abort();
    }
}

/* End of initalizer */

/* Destructor */

static void fdtab_uninit(void) __attribute__((destructor));

static size_t
fdtab_fd_uninit_walk(void* fd, struct picotm_error* error)
{
    fd_uninit(fd);
    return 1;
}

static void
fdtab_uninit(void)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_tabwalk_1(fdtab, sizeof(fdtab)/sizeof(fdtab[0]), sizeof(fdtab[0]),
                     fdtab_fd_uninit_walk, &error);
    if (picotm_error_is_set(&error)) {
        abort();
    }
}

/* End of destructor */

#include "fdtab.h"

struct fd*
fdtab_ref_fildes(int fildes, bool want_new, struct picotm_error* error)
{
    struct fd* fd = fdtab + fildes;

    fd_ref_or_set_up(fd, fildes, want_new, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    return fd;
}
