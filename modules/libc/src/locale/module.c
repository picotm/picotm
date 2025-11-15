/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2018   Thomas Zimmermann
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

#include "module.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-global-state.h"
#include "picotm/picotm-lib-shared-state.h"
#include "picotm/picotm-lib-state.h"
#include "picotm/picotm-lib-thread-state.h"
#include "picotm/picotm-module.h"
#include "locale.h"
#include "locale_log.h"
#include "locale_tx.h"

/*
 * Shared state
 */

static void
init_locale_shared_state_fields(struct locale* locale,
                                struct picotm_error* error)
{
    locale_init(locale);
}

static void
uninit_locale_shared_state_fields(struct locale* locale)
{
    locale_uninit(locale);
}

PICOTM_SHARED_STATE(locale, struct locale);
PICOTM_SHARED_STATE_STATIC_IMPL(locale, struct locale,
                                init_locale_shared_state_fields,
                                uninit_locale_shared_state_fields)

/*
 * Global state
 */

PICOTM_GLOBAL_STATE_STATIC_IMPL(locale)

/*
 * Module interface
 */

struct locale_module {
    struct locale_log log;
    struct locale_tx tx;
};

static void
locale_module_init(struct locale_module* self, struct locale* locale,
                   unsigned long module_id)
{
    assert(self);

    locale_log_init(&self->log, module_id);
    locale_tx_init(&self->tx, &self->log, locale);
}

static void
locale_module_uninit(struct locale_module* self)
{
    assert(self);

    locale_tx_uninit(&self->tx);
    locale_log_uninit(&self->log);
}

static void
locale_module_apply_event(struct locale_module* self, uint16_t head,
                          uintptr_t tail, struct picotm_error* error)
{
    assert(self);

    const struct locale_event* event = locale_log_at(&self->log, tail);
    assert(event);

    locale_tx_apply_event(&self->tx, head, event, error);
}

static void
locale_module_undo_event(struct locale_module* self, uint16_t head,
                         uintptr_t tail, struct picotm_error* error)
{
    assert(self);

    const struct locale_event* event = locale_log_at(&self->log, tail);
    assert(event);

    locale_tx_undo_event(&self->tx, head, event, error);
}

static void
locale_module_finish(struct locale_module* self)
{
    assert(self);

    locale_tx_finish(&self->tx);
    locale_log_clear(&self->log);
}

/*
 * Thread-local state
 */

PICOTM_STATE(locale_module, struct locale_module);
PICOTM_STATE_STATIC_DECL(locale_module, struct locale_module)
PICOTM_THREAD_STATE_STATIC_DECL(locale_module)

static void
apply_event_cb(uint16_t head, uintptr_t tail, void* data,
               struct picotm_error* error)
{
    struct locale_module* module = data;
    locale_module_apply_event(module, head, tail, error);
}

static void
undo_event_cb(uint16_t head, uintptr_t tail, void* data,
              struct picotm_error* error)
{
    struct locale_module* module = data;
    locale_module_undo_event(module, head, tail, error);
}

static void
finish_cb(void* data, struct picotm_error* error)
{
    struct locale_module* module = data;
    locale_module_finish(module);
}

static void
release_cb(void* data)
{
    PICOTM_THREAD_STATE_RELEASE(locale_module);
}

static void
init_locale_module(struct locale_module* module, struct picotm_error* error)
{
    static const struct picotm_module_ops s_ops = {
        .apply_event = apply_event_cb,
        .undo_event = undo_event_cb,
        .finish = finish_cb,
        .release = release_cb
    };

    struct locale* locale = PICOTM_GLOBAL_STATE_REF(locale, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    unsigned long module_id = picotm_register_module(&s_ops, module, error);
    if (picotm_error_is_set(error)) {
        goto err_picotm_register_module;
    }

    locale_module_init(module, locale, module_id);

    return;

err_picotm_register_module:
    PICOTM_GLOBAL_STATE_UNREF(locale);
}

static void
uninit_locale_module(struct locale_module* module)
{
    locale_module_uninit(module);
    PICOTM_GLOBAL_STATE_UNREF(locale);
}

PICOTM_STATE_STATIC_IMPL(locale_module, struct locale_module,
                         init_locale_module,
                         uninit_locale_module)
PICOTM_THREAD_STATE_STATIC_IMPL(locale_module)

static struct locale_tx*
get_locale_tx(struct picotm_error* error)
{
    struct locale_module* module = PICOTM_THREAD_STATE_ACQUIRE(locale_module,
                                                               true, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }
    return &module->tx;
}

/*
 * Public interface
 */

#if defined(PICOTM_LIBC_HAVE_DUPLOCALE) && PICOTM_LIBC_HAVE_DUPLOCALE
locale_t
locale_module_duplocale(locale_t locobj, struct picotm_error* error)
{
    struct locale_tx* locale_tx = get_locale_tx(error);
    if (picotm_error_is_set(error)) {
        return (locale_t)0;
    }
    return locale_tx_duplocale_exec(locale_tx, locobj, error);
}
#endif

#if defined(PICOTM_LIBC_HAVE_FREELOCALE) && PICOTM_LIBC_HAVE_FREELOCALE
void
locale_module_freelocale(locale_t locobj, struct picotm_error* error)
{
    struct locale_tx* locale_tx = get_locale_tx(error);
    if (picotm_error_is_set(error)) {
        return;
    }
    return locale_tx_freelocale_exec(locale_tx, locobj, error);
}
#endif

struct lconv*
locale_module_localeconv(struct picotm_error* error)
{
    struct locale_tx* locale_tx = get_locale_tx(error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }
    return locale_tx_localeconv_exec(locale_tx, error);
}

#if defined(PICOTM_LIBC_HAVE_NEWLOCALE) && PICOTM_LIBC_HAVE_NEWLOCALE
locale_t
locale_module_newlocale(int category_mask, const char* locale, locale_t base,
                        struct picotm_error* error)
{
    struct locale_tx* locale_tx = get_locale_tx(error);
    if (picotm_error_is_set(error)) {
        return (locale_t)0;
    }
    return locale_tx_newlocale_exec(locale_tx, category_mask, locale, base,
                                    error);
}
#endif

char*
locale_module_setlocale(int category, const char* locale,
                        struct picotm_error* error)
{
    struct locale_tx* locale_tx = get_locale_tx(error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }
    return locale_tx_setlocale_exec(locale_tx, category, locale, error);
}

#if defined(PICOTM_LIBC_HAVE_USELOCALE) && PICOTM_LIBC_HAVE_USELOCALE
locale_t
locale_module_uselocale(locale_t newloc, struct picotm_error* error)
{
    struct locale_tx* locale_tx = get_locale_tx(error);
    if (picotm_error_is_set(error)) {
        return (locale_t)0;
    }
    return locale_tx_uselocale_exec(locale_tx, newloc, error);
}
#endif
