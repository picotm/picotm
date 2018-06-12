/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018   Thomas Zimmermann <contact@tzimmermann.org>
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

#include "locale_tx.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include <assert.h>
#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include "locale_log.h"

static void
init_rwstates(struct picotm_rwstate* beg, const struct picotm_rwstate* end)
{
    while (beg < end) {
        picotm_rwstate_init(beg);
        ++beg;
    }
}

static void
uninit_rwstates(struct picotm_rwstate* beg, const struct picotm_rwstate* end)
{
    while (beg < end) {
        picotm_rwstate_uninit(beg);
        ++beg;
    }
}

static void
unlock_rwstates(struct picotm_rwstate* beg, const struct picotm_rwstate* end,
                struct locale* locale)
{
    enum locale_field field = 0;

    while (beg < end) {
        locale_unlock_field(locale, field, beg);
        ++field;
        ++beg;
    }
}

static void
try_lock_locale_fields_by_category(int category, struct locale_tx* tx,
                                   void (*try_lock)(struct locale_tx*,
                                                    enum locale_field,
                                                    struct picotm_error*),
                                   struct picotm_error* error)
{
    switch (category) {
    case LC_COLLATE:
        try_lock(tx, LOCALE_FIELD_COLLATE, error);
        break;
    case LC_CTYPE:
        try_lock(tx, LOCALE_FIELD_CTYPE, error);
        break;
    case LC_MESSAGES:
        try_lock(tx, LOCALE_FIELD_MESSAGES, error);
        break;
    case LC_MONETARY:
        try_lock(tx, LOCALE_FIELD_MONETARY, error);
        break;
    case LC_NUMERIC:
        try_lock(tx, LOCALE_FIELD_NUMERIC, error);
        break;
    case LC_TIME:
        try_lock(tx, LOCALE_FIELD_TIME, error);
        break;
    case LC_ALL:
        /* fall through */
    default:
        try_lock(tx, LOCALE_FIELD_COLLATE, error);
        if (picotm_error_is_set(error)) {
            return;
        }
        try_lock(tx, LOCALE_FIELD_CTYPE, error);
        if (picotm_error_is_set(error)) {
            return;
        }
        try_lock(tx, LOCALE_FIELD_MESSAGES, error);
        if (picotm_error_is_set(error)) {
            return;
        }
        try_lock(tx, LOCALE_FIELD_MONETARY, error);
        if (picotm_error_is_set(error)) {
            return;
        }
        try_lock(tx, LOCALE_FIELD_NUMERIC, error);
        if (picotm_error_is_set(error)) {
            return;
        }
        try_lock(tx, LOCALE_FIELD_TIME, error);
        break;
    }
}

void
locale_tx_init(struct locale_tx* self, struct locale_log* log,
               struct locale* locale)
{
    assert(self);
    assert(locale);

    self->log = log;
    self->locale = locale;

    init_rwstates(picotm_arraybeg(self->rwstate),
                  picotm_arrayend(self->rwstate));
}

void
locale_tx_uninit(struct locale_tx* self)
{
    assert(self);

    uninit_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate));
}

void
locale_tx_try_rdlock_field(struct locale_tx* self, enum locale_field field,
                           struct picotm_error* error)
{
    assert(self);

    locale_try_rdlock_field(self->locale, field, self->rwstate + field,
                            error);
}

void
locale_tx_try_wrlock_field(struct locale_tx* self, enum locale_field field,
                           struct picotm_error* error)
{
    assert(self);

    locale_try_wrlock_field(self->locale, field, self->rwstate + field,
                            error);
}

/*
 * localeconv()
 */

struct lconv*
locale_tx_localeconv_exec(struct locale_tx* self, struct picotm_error* error)
{
    try_lock_locale_fields_by_category(LC_ALL, self,
                                       locale_tx_try_rdlock_field,
                                       error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }
    return localeconv();
}

/*
 * setlocale()
 */

static char*
setlocale_rd_exec(struct locale_tx* self, int category,
                  struct picotm_error* error)
{
    try_lock_locale_fields_by_category(category, self,
                                       locale_tx_try_rdlock_field,
                                       error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }
    return setlocale(category, NULL);
}

static char*
setlocale_wr_exec(struct locale_tx* self, int category, const char* locale,
                  struct picotm_error* error)
{
    assert(self);

    try_lock_locale_fields_by_category(category, self,
                                       locale_tx_try_wrlock_field,
                                       error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    char* curlocale = setlocale(category, NULL);
    if (!curlocale) {
        picotm_error_set_error_code(error, PICOTM_GENERAL_ERROR);
        return NULL;
    }

    char* oldlocale = strdup(curlocale);
    if (!oldlocale) {
        picotm_error_set_errno(error, errno);
        return NULL;
    }

    curlocale = setlocale(category, locale);
    if (!curlocale) {
        picotm_error_set_error_code(error, PICOTM_GENERAL_ERROR);
        goto err_setlocale;
    }

    struct locale_event arg;
    arg.arg.setlocale.category = category;
    arg.arg.setlocale.oldlocale = oldlocale;

    locale_log_append(self->log, LOCALE_OP_SETLOCALE, &arg, error);
    if (picotm_error_is_set(error)) {
        goto err_locale_log_append;
    }

    return curlocale;

err_locale_log_append:
    curlocale = setlocale(category, oldlocale);
    if (!curlocale) {
        /* We keep the original error code. We mark the error
         * as non-recoverable, because the function's clean-up
         * block failed. */
        picotm_error_mark_as_non_recoverable(error);
    }
err_setlocale:
    free(oldlocale);
    return NULL;
}

char*
locale_tx_setlocale_exec(struct locale_tx* self, int category,
                         const char* locale, struct picotm_error* error)
{
    if (locale) {
        return setlocale_wr_exec(self, category, locale, error);
    } else {
        return setlocale_rd_exec(self, category, error);
    }
}

static void
apply_setlocale(struct locale_tx* self, const struct locale_event* arg,
                struct picotm_error* error)
{
    free(arg->arg.setlocale.oldlocale);
}

static void
undo_setlocale(struct locale_tx* self, const struct locale_event* arg,
               struct picotm_error* error)
{
    char* locale = setlocale(arg->arg.setlocale.category,
                             arg->arg.setlocale.oldlocale);
    if (!locale) {
        picotm_error_set_error_code(error, PICOTM_GENERAL_ERROR);
    }

    free(arg->arg.setlocale.oldlocale);
}

/*
 * Module interface
 */

void
locale_tx_apply_event(struct locale_tx* self, enum locale_op op,
                      const struct locale_event* arg,
                      struct picotm_error* error)
{
    static void (* const apply[])(struct locale_tx*,
                                  const struct locale_event*,
                                  struct picotm_error*) = {
        [LOCALE_OP_SETLOCALE] = apply_setlocale
    };

    apply[op](self, arg, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
locale_tx_undo_event(struct locale_tx* self, enum locale_op op,
                     const struct locale_event* arg,
                     struct picotm_error* error)
{
    static void (* const undo[])(struct locale_tx*,
                                 const struct locale_event*,
                                 struct picotm_error*) = {
        [LOCALE_OP_SETLOCALE] = undo_setlocale
    };

    undo[op](self, arg, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
locale_tx_finish(struct locale_tx* self)
{
    assert(self);

    /* release reader/writer locks on current working directory */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    self->locale);
}
