/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018-2019  Thomas Zimmermann <contact@tzimmermann.org>
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
#include "picotm/picotm-lib-ptr.h"
#include "picotm/picotm-module.h"
#include "picotm/picotm-ptrdata.h"
#include <assert.h>
#include <errno.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>
#include "locale_log.h"

#if defined(PICOTM_LIBC_HAVE_TYPE_LOCALE_T) && PICOTM_LIBC_HAVE_TYPE_LOCALE_T
struct locale_state {
    unsigned long ref_count;
    struct picotm_rwlock rwlock;
};

static void
locale_state_init(struct locale_state* self)
{
    assert(self);

    self->ref_count = 1;
    picotm_rwlock_init(&self->rwlock);
}

static void
locale_state_uninit(struct locale_state* self)
{
    assert(self);
    assert(!self->ref_count);

    picotm_rwlock_uninit(&self->rwlock);
}

struct locale_state_tx {
    struct picotm_slist slist;
    struct picotm_rwstate rwstate;
    struct locale_state* lstate;
    locale_t locobj;
};

static struct locale_state_tx*
locale_state_tx_of_slist(struct picotm_slist* slist)
{
    return picotm_containerof(slist, struct locale_state_tx, slist);
}

static void
locale_state_tx_init(struct locale_state_tx* self, locale_t locobj,
                     struct locale_state* lstate)
{
    assert(self);

    picotm_slist_init_item(&self->slist);
    picotm_rwstate_init(&self->rwstate);
    self->lstate = lstate;
    self->locobj = locobj;
}

static void
locale_state_tx_uninit(struct locale_state_tx* self)
{
    assert(self);
    assert(!picotm_slist_is_enqueued(&self->slist));

    picotm_rwstate_uninit(&self->rwstate);
    picotm_slist_uninit_item(&self->slist);
}
#endif

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

void
locale_tx_init(struct locale_tx* self, struct locale_log* log,
               struct locale* locale)
{
    assert(self);
    assert(locale);

    self->log = log;
    self->locale = locale;

    picotm_slist_init_head(&self->locales);

    init_rwstates(picotm_arraybeg(self->rwstate),
                  picotm_arrayend(self->rwstate));
}

void
locale_tx_uninit(struct locale_tx* self)
{
    assert(self);

    uninit_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate));

    picotm_slist_uninit_head(&self->locales);
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

#if defined(PICOTM_LIBC_HAVE_TYPE_LOCALE_T) && PICOTM_LIBC_HAVE_TYPE_LOCALE_T
static struct locale_state*
acquire_locale_state(locale_t locobj, struct locale* global,
                     struct picotm_error* error)
{
    struct locale_state* lstate = NULL;

    do {
        picotm_spinlock_lock(&global->locale_state_lock);
        lstate = ptr_get_shared_data(locobj, error);
        if (lstate) {
            ++lstate->ref_count;
            picotm_spinlock_unlock(&global->locale_state_lock);
            return lstate;
        }
        picotm_spinlock_unlock(&global->locale_state_lock);

        lstate = malloc(sizeof(*lstate));
        if (!lstate) {
            picotm_error_set_errno(error, ENOMEM);
            return NULL;
        }
        locale_state_init(lstate);

        bool succ = ptr_test_and_set_shared_data(locobj, NULL, lstate, error);
        if (picotm_error_is_set(error)) {
            goto err_ptr_test_and_set_shared_data;
        } else if (succ) {
            return lstate;
        }

        --lstate->ref_count;
        locale_state_uninit(lstate);
        free(lstate);
    } while (1);

    /* not reachable */

err_ptr_test_and_set_shared_data:
    locale_state_uninit(lstate);
    free(lstate);
    return NULL;
}

static void
release_locale_state(locale_t locobj, struct locale_state* lstate,
                     struct locale* global, struct picotm_error* error)
{
    assert(lstate);
    assert(global);

    picotm_spinlock_lock(&global->locale_state_lock);
    --lstate->ref_count;
    if (lstate->ref_count) {
        picotm_spinlock_unlock(&global->locale_state_lock);
        return;
    }
    bool succ = ptr_test_and_set_shared_data(locobj, lstate, NULL, error);
    picotm_spinlock_unlock(&global->locale_state_lock);

    if (picotm_error_is_set(error)) {
        return; /* critical, but should not happen */
    } else if (!succ) {
        return; /* should not happen */
    }

    locale_state_uninit(lstate);
    free(lstate);
}

static struct locale_state_tx*
acquire_locale_state_tx(locale_t locobj, struct locale_tx* tx,
                        struct picotm_error* error)
{
    assert(tx);

    struct locale_state_tx* lstate_tx = ptr_get_data(locobj, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    } else if (lstate_tx) {
        return lstate_tx;
    }

    struct locale_state* lstate = acquire_locale_state(locobj, tx->locale,
                                                       error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    lstate_tx = malloc(sizeof(*lstate_tx));
    if (!lstate_tx) {
        picotm_error_set_errno(error, ENOMEM);
        goto err_malloc;
    }
    locale_state_tx_init(lstate_tx, locobj, lstate);

    ptr_set_data(locobj, lstate_tx, error);
    if (picotm_error_is_set(error)) {
        goto err_ptr_set_data;
    }

    picotm_slist_enqueue_front(&tx->locales, &lstate_tx->slist);

    return lstate_tx;

err_ptr_set_data:
    locale_state_tx_uninit(lstate_tx);
    free(lstate_tx);
err_malloc:
    {
        struct picotm_error tmp_error = PICOTM_ERROR_INITIALIZER;
        release_locale_state(locobj, lstate, tx->locale, &tmp_error);
        if (picotm_error_is_set(&tmp_error)) {
            picotm_error_mark_as_non_recoverable(error);
        }
    }
    return NULL;
}

static void
release_locale_state_tx(struct locale_state_tx* lstate_tx,
                        struct locale* global,
                        struct picotm_error* error)
{
    assert(lstate_tx);
    assert(global);

    ptr_clear_data(lstate_tx->locobj, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    release_locale_state(lstate_tx->locobj, lstate_tx->lstate, global, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    picotm_slist_dequeue(&lstate_tx->slist);
    locale_state_tx_uninit(lstate_tx);
    free(lstate_tx);
}
#endif

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

/*
 * duplocale()
 */

#if defined(PICOTM_LIBC_HAVE_DUPLOCALE) && PICOTM_LIBC_HAVE_DUPLOCALE
locale_t
locale_tx_duplocale_exec(struct locale_tx* self, locale_t locobj,
                         struct picotm_error* error)
{
    assert(self);

    /* read-lock locobj object */

    struct locale_state_tx* lstate_tx = acquire_locale_state_tx(locobj, self,
                                                                error);
    if (picotm_error_is_set(error)) {
        return (locale_t)0;
    }
    picotm_rwstate_try_wrlock(&lstate_tx->rwstate,
                              &lstate_tx->lstate->rwlock,
                              error);
    if (picotm_error_is_set(error)) {
        return (locale_t)0;
    }

    /* duplicate locale */

    locale_t newlocobj = duplocale(locobj);
    if (newlocobj == (locale_t)0) {
        picotm_error_set_errno(error, errno);
        return (locale_t)0;
    }

    struct locale_event arg;
    arg.arg.newlocale.result = newlocobj;

    locale_log_append(self->log, LOCALE_OP_DUPLOCALE, &arg, error);
    if (picotm_error_is_set(error)) {
        goto err_locale_log_append;
    }

    return newlocobj;

err_locale_log_append:
    freelocale(newlocobj);
    return NULL;
}

static void
apply_duplocale(struct locale_tx* self, const struct locale_event* arg,
                struct picotm_error* error)
{ }

static void
undo_duplocale(struct locale_tx* self, const struct locale_event* arg,
               struct picotm_error* error)
{
    assert(arg);

    freelocale(arg->arg.duplocale.result);
}
#else
#define apply_duplocale NULL
#define undo_duplocale  NULL
#endif

/*
 * freelocale()
 */

#if defined(PICOTM_LIBC_HAVE_FREELOCALE) && PICOTM_LIBC_HAVE_FREELOCALE
void
locale_tx_freelocale_exec(struct locale_tx* self, locale_t locobj,
                          struct picotm_error* error)
{
    assert(self);

    /* write-lock locobj object */

    struct locale_state_tx* lstate_tx = acquire_locale_state_tx(locobj, self,
                                                                error);
    if (picotm_error_is_set(error)) {
        return;
    }
    picotm_rwstate_try_wrlock(&lstate_tx->rwstate,
                              &lstate_tx->lstate->rwlock,
                              error);
    if (picotm_error_is_set(error)) {
        return;
    }

    /* append event to queue */

    struct locale_event arg;
    arg.arg.freelocale.locobj = locobj;

    locale_log_append(self->log, LOCALE_OP_FREELOCALE, &arg, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
apply_freelocale(struct locale_tx* self, const struct locale_event* arg,
                 struct picotm_error* error)
{
    assert(arg);

    freelocale(arg->arg.duplocale.result);
}

static void
undo_freelocale(struct locale_tx* self, const struct locale_event* arg,
                struct picotm_error* error)
{ }
#else
#define apply_freelocale    NULL
#define undo_freelocale     NULL
#endif

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
 * newlocale()
 */

#if defined(PICOTM_LIBC_HAVE_NEWLOCALE) && PICOTM_LIBC_HAVE_NEWLOCALE
locale_t
locale_tx_newlocale_exec(struct locale_tx* self, int category_mask,
                         const char* locale, locale_t base,
                         struct picotm_error* error)
{
    assert(self);

    /* write-lock base object */

    struct locale_state_tx* lstate_tx = acquire_locale_state_tx(base, self,
                                                                error);
    if (picotm_error_is_set(error)) {
        return (locale_t)0;
    }
    picotm_rwstate_try_wrlock(&lstate_tx->rwstate,
                              &lstate_tx->lstate->rwlock,
                              error);
    if (picotm_error_is_set(error)) {
        return (locale_t)0;
    }

    /* create a new locale */

    locale_t locobj = newlocale(category_mask, locale, base);
    if (locobj == (locale_t)0) {
        picotm_error_set_errno(error, errno);
        return (locale_t)0;
    }

    struct locale_event arg;
    arg.arg.newlocale.result = locobj;

    locale_log_append(self->log, LOCALE_OP_NEWLOCALE, &arg, error);
    if (picotm_error_is_set(error)) {
        goto err_locale_log_append;
    }

    return locobj;

err_locale_log_append:
    freelocale(locobj);
    return NULL;
}

static void
apply_newlocale(struct locale_tx* self, const struct locale_event* arg,
                struct picotm_error* error)
{ }

static void
undo_newlocale(struct locale_tx* self, const struct locale_event* arg,
               struct picotm_error* error)
{
    assert(arg);

    freelocale(arg->arg.newlocale.result);
}
#else
#define apply_newlocale NULL
#define undo_newlocale  NULL
#endif

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
    picotm_spinlock_lock(&self->locale->setlocale_lock);
    char* locale = setlocale(category, NULL);
    picotm_spinlock_unlock(&self->locale->setlocale_lock);

    if (!locale)
        return NULL; /* TODO: should this be an error? */

    char* duplocale = strdup(locale);
    if (!duplocale) {
        picotm_error_set_errno(error, errno);
        return NULL;
    }

    struct locale_event arg;
    arg.arg.setlocale.category = category;
    arg.arg.setlocale.oldlocale = duplocale;

    locale_log_append(self->log, LOCALE_OP_SETLOCALE, &arg, error);
    if (picotm_error_is_set(error))
        goto err_free_duplocale;

    /* The string returned by setlocale() is owned by setlocale()'s
     * implementation. This interferes with TM operatons, such as
     * privatization. We have to return a copy of the string to make
     * it work reliably. The transaction's apply/undo phases will
     * release the memory. */
    return duplocale;

err_free_duplocale:
    free(duplocale);
    return NULL;
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

    picotm_spinlock_lock(&self->locale->setlocale_lock);
    char* curlocale = setlocale(category, NULL);
    picotm_spinlock_unlock(&self->locale->setlocale_lock);
    if (!curlocale) {
        picotm_error_set_error_code(error, PICOTM_GENERAL_ERROR);
        return NULL;
    }

    char* oldlocale = strdup(curlocale);
    if (!oldlocale) {
        picotm_error_set_errno(error, errno);
        return NULL;
    }

    picotm_spinlock_lock(&self->locale->setlocale_lock);
    curlocale = setlocale(category, locale);
    picotm_spinlock_unlock(&self->locale->setlocale_lock);
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
    picotm_spinlock_lock(&self->locale->setlocale_lock);
    curlocale = setlocale(category, oldlocale);
    picotm_spinlock_unlock(&self->locale->setlocale_lock);
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
    picotm_spinlock_lock(&self->locale->setlocale_lock);
    char* locale = setlocale(arg->arg.setlocale.category,
                             arg->arg.setlocale.oldlocale);
    picotm_spinlock_unlock(&self->locale->setlocale_lock);
    if (!locale) {
        picotm_error_set_error_code(error, PICOTM_GENERAL_ERROR);
    }

    free(arg->arg.setlocale.oldlocale);
}

/*
 * uselocale()
 */

#if defined(PICOTM_LIBC_HAVE_USELOCALE) && PICOTM_LIBC_HAVE_USELOCALE
static locale_t
uselocale_rd_exec(struct picotm_error* error)
{
    locale_t oldloc = uselocale((locale_t)0);
    if (oldloc == (locale_t)0) {
        picotm_error_set_errno(error, errno);
        return (locale_t)0;
    }
    return oldloc;
}

static locale_t
uselocale_wr_exec(struct locale_tx* self, locale_t newloc,
                  struct picotm_error* error)
{
    assert(self);

    /* read-lock newloc object */

    if (newloc != LC_GLOBAL_LOCALE) {
        struct locale_state_tx* lstate_tx = acquire_locale_state_tx(newloc,
                                                                    self,
                                                                    error);
        if (picotm_error_is_set(error)) {
            return (locale_t)0;
        }
        picotm_rwstate_try_rdlock(&lstate_tx->rwstate,
                                  &lstate_tx->lstate->rwlock,
                                  error);
        if (picotm_error_is_set(error)) {
            return (locale_t)0;
        }
    }

    /* create a new locale */

    locale_t oldloc = uselocale(newloc);
    if (oldloc == (locale_t)0) {
        picotm_error_set_errno(error, errno);
        return (locale_t)0;
    }

    struct locale_event arg;
    arg.arg.uselocale.oldloc = oldloc;

    locale_log_append(self->log, LOCALE_OP_USELOCALE, &arg, error);
    if (picotm_error_is_set(error)) {
        return (locale_t)0;
    }

    return oldloc;
}

locale_t
locale_tx_uselocale_exec(struct locale_tx* self, locale_t newloc,
                         struct picotm_error* error)
{
    if (newloc == (locale_t)0) {
        return uselocale_rd_exec(error);
    } else {
        return uselocale_wr_exec(self, newloc, error);
    }
}

static void
apply_uselocale(struct locale_tx* self, const struct locale_event* arg,
                struct picotm_error* error)
{ }

static void
undo_uselocale(struct locale_tx* self, const struct locale_event* arg,
               struct picotm_error* error)
{
    assert(arg);

    locale_t res = uselocale(arg->arg.uselocale.oldloc);
    if (res == (locale_t)0) {
        picotm_error_set_errno(error, errno);
        picotm_error_mark_as_non_recoverable(error);
        return;
    }
}
#else
#define apply_uselocale NULL
#define undo_uselocale  NULL
#endif

/*
 * Module interface
 */

void
locale_tx_apply_event(struct locale_tx* self, enum locale_op op,
                      const struct locale_event* arg,
                      struct picotm_error* error)
{
    static void (* const apply[LAST_LOCALE_OP])(struct locale_tx*,
                                                const struct locale_event*,
                                                struct picotm_error*) = {
        [LOCALE_OP_DUPLOCALE]  = apply_duplocale,
        [LOCALE_OP_FREELOCALE] = apply_freelocale,
        [LOCALE_OP_NEWLOCALE]  = apply_newlocale,
        [LOCALE_OP_SETLOCALE]  = apply_setlocale,
        [LOCALE_OP_USELOCALE]  = apply_uselocale
    };

    assert(op < picotm_arraylen(apply));
    assert(apply[op]);

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
    static void (* const undo[LAST_LOCALE_OP])(struct locale_tx*,
                                               const struct locale_event*,
                                               struct picotm_error*) = {
        [LOCALE_OP_DUPLOCALE]  = undo_duplocale,
        [LOCALE_OP_FREELOCALE] = undo_freelocale,
        [LOCALE_OP_NEWLOCALE]  = undo_newlocale,
        [LOCALE_OP_SETLOCALE]  = undo_setlocale,
        [LOCALE_OP_USELOCALE]  = undo_uselocale
    };

    assert(op < picotm_arraylen(undo));
    assert(undo[op]);

    undo[op](self, arg, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
locale_tx_finish(struct locale_tx* self)
{
    assert(self);

#if defined(PICOTM_LIBC_HAVE_TYPE_LOCALE_T) && PICOTM_LIBC_HAVE_TYPE_LOCALE_T
    while (!picotm_slist_is_empty(&self->locales)) {

        struct picotm_slist* pos = picotm_slist_front(&self->locales);
        struct locale_state_tx* lstate_tx = locale_state_tx_of_slist(pos);

        do {
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            release_locale_state_tx(lstate_tx, self->locale, &error);
            if (!picotm_error_is_set(&error)) {
                break;
            }
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
        } while (true);
    }
#endif

    /* release reader/writer locks on locale fields */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    self->locale);
}
