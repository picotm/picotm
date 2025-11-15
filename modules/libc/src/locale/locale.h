/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018-2019  Thomas Zimmermann
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

#include "picotm/picotm-lib-spinlock.h"
#include "picotm/picotm-lib-rwlock.h"

/**
 * \cond impl || libc_impl || libc_impl_locale
 * \ingroup libc_impl
 * \ingroup libc_impl_locale
 * \file
 * \endcond
 */

struct picotm_error;
struct picotm_rwstate;

/**
 * Enumerates the fields of `struct locale`.
 */
enum locale_field {
    LOCALE_FIELD_COLLATE,
    LOCALE_FIELD_CTYPE,
    LOCALE_FIELD_MESSAGES,
    LOCALE_FIELD_MONETARY,
    LOCALE_FIELD_NUMERIC,
    LOCALE_FIELD_TIME,
    NUMBER_OF_LOCALE_FIELDS
};

/**
 * Represents the global state of the current locale settings.
 */
struct locale {
    struct picotm_rwlock    rwlock[NUMBER_OF_LOCALE_FIELDS];

    struct picotm_spinlock locale_state_lock;

    /* TSAN instrumentation reports a read/write race condition
     * for setlocale() even if the function only gets called for
     * reading. Apparantly even reading implies a write operation.
     * to protect against this, we acquire this lock in every
     * function that calls setlocale().
     */
    struct picotm_spinlock setlocale_lock;
};

/**
 * Initialized a locale structure
 *
 * \param   self    The locale structure.
 */
void
locale_init(struct locale* self);

/**
 * Uninitialized a locale structure
 *
 * \param   self    The locale structure.
 */
void
locale_uninit(struct locale* self);

/**
 * Tries to acquire a reader lock on the current locale.
 *
 * \param       self    The locale structure.
 * \param       field   The reader lock's field.
 * \param       rwstate The transaction's reader-writer state.
 * \param[out]  error   Returns an error.
 */
void
locale_try_rdlock_field(struct locale* self, enum locale_field field,
                        struct picotm_rwstate* rwstate,
                        struct picotm_error* error);

/**
 * Tries to acquire a writer lock on the current locale.
 *
 * \param       self    The locale structure.
 * \param       field   The writer lock's field.
 * \param       rwstate The transaction's reader-writer state.
 * \param[out]  error   Returns an error.
 */
void
locale_try_wrlock_field(struct locale* self, enum locale_field field,
                        struct picotm_rwstate* rwstate,
                        struct picotm_error* error);

/**
 * Releases a reader/writer lock on the current locale.
 *
 * \param   self    The locale structure.
 * \param   field   The reader/writer lock's field.
 * \param   rwstate The transaction's reader-writer state.
 */
void
locale_unlock_field(struct locale* self, enum locale_field field,
                    struct picotm_rwstate* rwstate);
