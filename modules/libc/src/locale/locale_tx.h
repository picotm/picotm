/*
 * picotm - A system-level transaction manager
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

#include "picotm/picotm-lib-rwstate.h"
#include "picotm/picotm-lib-slist.h"
#include <stddef.h>
#include <stdint.h>
#include "locale.h"
#include "locale_event.h"

/**
 * \cond impl || libc_impl || libc_impl_locale
 * \ingroup libc_impl
 * \ingroup libc_impl_locale
 * \file
 * \endcond
 */

struct locale_log;
struct picotm_error;

struct locale_tx {

    struct locale_log* log;

    struct locale* locale;

    struct picotm_slist locales;

    struct picotm_rwstate rwstate[NUMBER_OF_LOCALE_FIELDS];
};

void
locale_tx_init(struct locale_tx* self, struct locale_log* log,
               struct locale* locale);

void
locale_tx_uninit(struct locale_tx* self);

/**
 * Tries to acquire a reader lock on the current locale.
 *
 * \param       self    The locale structure.
 * \param       field   The reader lock's field.
 * \param[out]  error   Returns an error.
 */
void
locale_tx_try_rdlock_field(struct locale_tx* self, enum locale_field field,
                           struct picotm_error* error);

/**
 * Tries to acquire a writer lock on the current locale.
 *
 * \param       self    The locale structure.
 * \param       field   The writer lock's field.
 * \param[out]  error   Returns an error.
 */
void
locale_tx_try_wrlock_field(struct locale_tx* self, enum locale_field field,
                           struct picotm_error* error);

/*
 * duplocale()
 */

#if defined(PICOTM_LIBC_HAVE_DUPLOCALE) && PICOTM_LIBC_HAVE_DUPLOCALE
locale_t
locale_tx_duplocale_exec(struct locale_tx* self, locale_t locobj,
                         struct picotm_error* error);
#endif

/*
 * freelocale()
 */

#if defined(PICOTM_LIBC_HAVE_FREELOCALE) && PICOTM_LIBC_HAVE_FREELOCALE
void
locale_tx_freelocale_exec(struct locale_tx* self, locale_t locobj,
                          struct picotm_error* error);
#endif

/*
 * localeconv()
 */

struct lconv*
locale_tx_localeconv_exec(struct locale_tx* self, struct picotm_error* error);

/*
 * newlocale()
 */

#if defined(PICOTM_LIBC_HAVE_NEWLOCALE) && PICOTM_LIBC_HAVE_NEWLOCALE
locale_t
locale_tx_newlocale_exec(struct locale_tx* self, int category_mask,
                         const char* locale, locale_t base,
                         struct picotm_error* error);
#endif

/*
 * setlocale()
 */

#if defined(PICOTM_LIBC_HAVE_SETLOCALE) && PICOTM_LIBC_HAVE_SETLOCALE
char*
locale_tx_setlocale_exec(struct locale_tx* self, int category,
                         const char* locale, struct picotm_error* error);
#endif

/*
 * uselocale()
 */

#if defined(PICOTM_LIBC_HAVE_USELOCALE) && PICOTM_LIBC_HAVE_USELOCALE
locale_t
locale_tx_uselocale_exec(struct locale_tx* self, locale_t newloc,
                         struct picotm_error* error);
#endif

/*
 * Module interface
 */

void
locale_tx_apply_event(struct locale_tx* self, enum locale_op op,
                      const struct locale_event* arg,
                      struct picotm_error* error);

void
locale_tx_undo_event(struct locale_tx* self, enum locale_op op,
                     const struct locale_event* arg,
                     struct picotm_error* error);

void
locale_tx_finish(struct locale_tx* self);
