/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <tdz@users.sourceforge.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
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

#include "chrdevtab.h"
#include <errno.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-array.h>
#include <picotm/picotm-lib-tab.h>
#include <picotm/picotm-module.h>
#include <pthread.h>
#include "chrdev.h"
#include "range.h"

static struct chrdev    chrdevtab[MAXNUMFD];
static size_t           chrdevtab_len = 0;
static pthread_rwlock_t chrdevtab_rwlock = PTHREAD_RWLOCK_INITIALIZER;

/* Destructor */

static void chrdevtab_uninit(void) __attribute__ ((destructor));

static size_t
chrdevtab_chrdev_uninit_walk(void* chrdev, struct picotm_error* error)
{
    chrdev_uninit(chrdev);
    return 1;
}

static void
chrdevtab_uninit(void)
{
    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    picotm_tabwalk_1(chrdevtab, chrdevtab_len, sizeof(chrdevtab[0]),
                     chrdevtab_chrdev_uninit_walk, &error);

    pthread_rwlock_destroy(&chrdevtab_rwlock);
}

/* End of destructor */

static void
rdlock_chrdevtab(struct picotm_error* error)
{
    int err = pthread_rwlock_rdlock(&chrdevtab_rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
wrlock_chrdevtab(struct picotm_error* error)
{
    int err = pthread_rwlock_wrlock(&chrdevtab_rwlock);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

static void
unlock_chrdevtab(void)
{
    do {
        int err = pthread_rwlock_unlock(&chrdevtab_rwlock);
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
static struct chrdev*
append_empty_chrdev(struct picotm_error* error)
{
    if (chrdevtab_len == picotm_arraylen(chrdevtab)) {
        /* Return error if not enough ids available */
        picotm_error_set_conflicting(error, NULL);
        return NULL;
    }

    struct chrdev* chrdev = chrdevtab + chrdevtab_len;

    chrdev_init(chrdev, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    ++chrdevtab_len;

    return chrdev;
}

/* requires reader lock */
static struct chrdev*
find_by_id(const struct file_id* id)
{
    struct chrdev *chrdev_beg = picotm_arraybeg(chrdevtab);
    const struct chrdev* chrdev_end = picotm_arrayat(chrdevtab, chrdevtab_len);

    while (chrdev_beg < chrdev_end) {

        const int cmp = chrdev_cmp_and_ref(chrdev_beg, id);
        if (!cmp) {
            return chrdev_beg;
        }

        ++chrdev_beg;
    }

    return NULL;
}

/* requires writer lock */
static struct chrdev*
search_by_id(const struct file_id* id, int fildes, struct picotm_error* error)
{
    struct chrdev* chrdev_beg = picotm_arraybeg(chrdevtab);
    const struct chrdev* chrdev_end = picotm_arrayat(chrdevtab, chrdevtab_len);

    while (chrdev_beg < chrdev_end) {

        const int cmp = chrdev_cmp_and_ref_or_set_up(chrdev_beg, id, fildes,
                                                     error);
        if (!cmp) {
            if (picotm_error_is_set(error)) {
                return NULL;
            }
            return chrdev_beg; /* set-up chrdev structure; return */
        }

        ++chrdev_beg;
    }

    return NULL;
}

struct chrdev*
chrdevtab_ref_fildes(int fildes, struct picotm_error* error)
{
    struct file_id id;
    file_id_init_from_fildes(&id, fildes, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    /* Try to find an existing chrdev structure with the given id.
     */

    rdlock_chrdevtab(error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct chrdev* chrdev = find_by_id(&id);
    if (chrdev) {
        goto unlock; /* found chrdev for id; return */
    }

    unlock_chrdevtab();

    /* Not found entry; acquire writer lock to create a new entry in
     * the chrdev table.
     */

    wrlock_chrdevtab(error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    /* Re-try find operation; maybe element was added meanwhile. */
    chrdev = find_by_id(&id);
    if (chrdev) {
        goto unlock; /* found chrdev for id; return */
    }

    /* No entry with the id exists; try to set up an existing, but
     * currently unused, chrdev structure.
     */

    struct file_id empty_id;
    file_id_clear(&empty_id);

    chrdev = search_by_id(&empty_id, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_search_by_id;
    } else if (chrdev) {
        goto unlock; /* found chrdev for id; return */
    }

    /* The chrdev table is full; create a new entry for the chrdev id at the
     * end of the table.
     */

    chrdev = append_empty_chrdev(error);
    if (picotm_error_is_set(error)) {
        goto err_append_empty_chrdev;
    }

    /* To perform the setup, we must have acquired a writer lock at
     * this point. No other transaction may interfere. */
    chrdev_ref_or_set_up(chrdev, fildes, error);
    if (picotm_error_is_set(error)) {
        goto err_chrdev_ref_or_set_up;
    }

unlock:
    unlock_chrdevtab();

    return chrdev;

err_chrdev_ref_or_set_up:
err_append_empty_chrdev:
err_search_by_id:
    unlock_chrdevtab();
    return NULL;
}

size_t
chrdevtab_index(struct chrdev* chrdev)
{
    return chrdev - chrdevtab;
}
