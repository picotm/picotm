/*
 * MIT License
 * Copyright (c) 2018   Thomas Zimmermann <tdz@users.sourceforge.net>
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
 *
 * SPDX-License-Identifier: MIT
 */

#include "fildes.h"
#include "chrdevtab.h"
#include "dirtab.h"
#include "fdtab.h"
#include "fifotab.h"
#include "ofdtab.h"
#include "regfiletab.h"
#include "sockettab.h"

/*
 * fdtab
 */

static struct fildes_fdtab fdtab = FILDES_FDTAB_INITIALIZER;

static __attribute__((destructor)) void
fdtab_uninit(void)
{
    fildes_fdtab_uninit(&fdtab);
}

struct fd*
fdtab_ref_fildes(int fildes, struct picotm_rwstate* lock_state,
                 struct picotm_error* error)
{
    return fildes_fdtab_ref_fildes(&fdtab, fildes, lock_state, error);
}

struct fd*
fdtab_get_fd(int fildes)
{
    return fildes_fdtab_get_fd(&fdtab, fildes);
}

void
fdtab_try_rdlock(struct picotm_rwstate* lock_state,
                 struct picotm_error* error)
{
    fildes_fdtab_try_rdlock(&fdtab, lock_state, error);
}

void
fdtab_try_wrlock(struct picotm_rwstate* lock_state,
                 struct picotm_error* error)
{
    fildes_fdtab_try_wrlock(&fdtab, lock_state, error);
}

void
fdtab_unlock(struct picotm_rwstate* lock_state)
{
    fildes_fdtab_unlock(&fdtab, lock_state);
}

/*
 * ofdtab
 */

static struct fildes_ofdtab ofdtab = FILDES_OFDTAB_INITIALIZER;

static __attribute__ ((destructor)) void
ofdtab_uninit(void)
{
    fildes_ofdtab_uninit(&ofdtab);
}

struct ofd*
ofdtab_ref_fildes(int fildes, bool newly_created, struct picotm_error* error)
{
    return fildes_ofdtab_ref_fildes(&ofdtab, fildes, newly_created, error);
}

size_t
ofdtab_index(struct ofd* ofd)
{
    return fildes_ofdtab_index(&ofdtab, ofd);
}

/*
 * chrdevtab
 */

static struct fildes_chrdevtab chrdevtab = FILDES_CHRDEVTAB_INITIALIZER;

static __attribute__((destructor)) void
chrdevtab_uninit(void)
{
    fildes_chrdevtab_uninit(&chrdevtab);
}

struct chrdev*
chrdevtab_ref_fildes(int fildes, struct picotm_error* error)
{
    return fildes_chrdevtab_ref_fildes(&chrdevtab, fildes, error);
}

size_t
chrdevtab_index(struct chrdev* chrdev)
{
    return fildes_chrdevtab_index(&chrdevtab, chrdev);
}

/*
 * dirtab
 */

static struct fildes_dirtab dirtab = FILDES_DIRTAB_INITIALIZER;

static __attribute__((destructor)) void
dirtab_uninit(void)
{
    fildes_dirtab_uninit(&dirtab);
}

struct dir*
dirtab_ref_fildes(int fildes, struct picotm_error* error)
{
    return fildes_dirtab_ref_fildes(&dirtab, fildes, error);
}

size_t
dirtab_index(struct dir* dir)
{
    return fildes_dirtab_index(&dirtab, dir);
}

/*
 * fifotab
 */

static struct fildes_fifotab fifotab = FILDES_FIFOTAB_INITIALIZER;

static __attribute__((destructor)) void
fifotab_uninit(void)
{
    fildes_fifotab_uninit(&fifotab);
}

struct fifo*
fifotab_ref_fildes(int fildes, struct picotm_error* error)
{
    return fildes_fifotab_ref_fildes(&fifotab, fildes, error);
}

size_t
fifotab_index(struct fifo* fifo)
{
    return fildes_fifotab_index(&fifotab, fifo);
}

/*
 * regfiletab
 */

static struct fildes_regfiletab regfiletab = FILDES_REGFILETAB_INITIALIZER;

static __attribute__((destructor)) void
regfiletab_uninit(void)
{
    fildes_regfiletab_uninit(&regfiletab);
}

struct regfile*
regfiletab_ref_fildes(int fildes, struct picotm_error* error)
{
    return fildes_regfiletab_ref_fildes(&regfiletab, fildes, error);
}

size_t
regfiletab_index(struct regfile* regfile)
{
    return fildes_regfiletab_index(&regfiletab, regfile);
}

/*
 * sockettab
 */

static struct fildes_sockettab sockettab = FILDES_SOCKETTAB_INITIALIZER;

static __attribute__((destructor)) void
sockettab_uninit(void)
{
    fildes_sockettab_uninit(&sockettab);
}

struct socket*
sockettab_ref_fildes(int fildes, struct picotm_error* error)
{
    return fildes_sockettab_ref_fildes(&sockettab, fildes, error);
}

size_t
sockettab_index(struct socket* socket)
{
    return fildes_sockettab_index(&sockettab, socket);
}
