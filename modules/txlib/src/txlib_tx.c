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

#include "txlib_tx.h"
#include <assert.h>
#include <errno.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-array.h>
#include <picotm/picotm-lib-tab.h>
#include <picotm/picotm-module.h>
#include <stdlib.h>
#include <string.h>
#include "txlib_event.h"
#include "txlist_tx.h"

void
txlib_tx_init(struct txlib_tx* self, unsigned long module)
{
    assert(self);

    self->module = module;

    SLIST_INIT(&self->allocated_list_tx);
    SLIST_INIT(&self->acquired_list_tx);

    self->event = NULL;
    self->nevents = 0;
}

void
txlib_tx_uninit(struct txlib_tx* self)
{
    assert(self);

    struct txlist_tx_entry* entry =
        SLIST_FIRST(&self->allocated_list_tx);

    while (entry) {
        SLIST_REMOVE_HEAD(&self->allocated_list_tx, slist_entry);
        txlist_tx_uninit(&entry->list_tx);
        free(entry);
        entry = SLIST_FIRST(&self->allocated_list_tx);
    }

    assert(SLIST_EMPTY(&self->acquired_list_tx));

    picotm_tabfree(self->event);
}

struct txlist_tx*
txlib_tx_acquire_txlist_of_state(struct txlib_tx* self,
                                 struct txlist_state* list_state,
                                 struct picotm_error* error)
{
    assert(self);

    struct txlist_tx_entry* entry =
        SLIST_FIRST(&self->acquired_list_tx);

    while (entry) {

        if (entry->list_tx.list_state == list_state) {
            return &entry->list_tx;
        }

        entry = SLIST_NEXT(entry, slist_entry);
    }

    entry = SLIST_FIRST(&self->allocated_list_tx);

    if (entry) {
        SLIST_REMOVE_HEAD(&self->allocated_list_tx, slist_entry);
    } else {
        entry = malloc(sizeof(*entry));
        if (!entry) {
            picotm_error_set_errno(error, errno);
            return NULL;
        }
        memset(&entry->slist_entry, 0, sizeof(entry->slist_entry));
    }

    txlist_tx_init(&entry->list_tx, list_state, self);
    SLIST_INSERT_HEAD(&self->acquired_list_tx, entry, slist_entry);

    return &entry->list_tx;
}

void
txlib_tx_append_events1(struct txlib_tx* self, size_t nevents,
                        void (*init)(struct txlib_event*, void*,
                                     struct picotm_error*),
                        void* data1, struct picotm_error* error)
{
    assert(self);
    assert(init);

    size_t newnevents = self->nevents + nevents;

    void* newevent = picotm_tabresize(self->event, self->nevents,
                                      newnevents, sizeof(*self->event),
                                      error);
    if (picotm_error_is_set(error)) {
        return;
    }

    self->event = newevent;

    struct txlib_event* beg = picotm_arrayat(self->event, self->nevents);
    struct txlib_event* end = picotm_arrayat(self->event, newnevents);

    for (; beg < end; ++beg, ++self->nevents) {

        init(beg, data1, error);
        if (picotm_error_is_set(error)) {
            return;
        }

        picotm_append_event(self->module, beg - self->event, (uintptr_t)NULL,
                            error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }
}

void
txlib_tx_append_events2(struct txlib_tx* self, size_t nevents,
                        void (*init)(struct txlib_event*, void*, void*,
                                     struct picotm_error*),
                        void* data1, void* data2,
                        struct picotm_error* error)
{
    assert(self);
    assert(init);

    size_t newnevents = self->nevents + nevents;

    void* newevent = picotm_tabresize(self->event, self->nevents,
                                      newnevents, sizeof(*self->event),
                                      error);
    if (picotm_error_is_set(error)) {
        return;
    }

    self->event = newevent;

    struct txlib_event* beg = picotm_arrayat(self->event, self->nevents);
    struct txlib_event* end = picotm_arrayat(self->event, newnevents);

    for (; beg < end; ++beg, ++self->nevents) {

        init(beg, data1, data2, error);
        if (picotm_error_is_set(error)) {
            return;
        }

        picotm_append_event(self->module, beg - self->event, (uintptr_t)NULL,
                            error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }
}

void
txlib_tx_append_events3(struct txlib_tx* self, size_t nevents,
                        void (*init)(struct txlib_event*, void*, void*, void*,
                                     struct picotm_error*),
                        void* data1, void* data2, void* data3,
                        struct picotm_error* error)
{
    assert(self);
    assert(init);

    size_t newnevents = self->nevents + nevents;

    void* newevent = picotm_tabresize(self->event, self->nevents,
                                      newnevents, sizeof(*self->event),
                                      error);
    if (picotm_error_is_set(error)) {
        return;
    }

    self->event = newevent;

    struct txlib_event* beg = picotm_arrayat(self->event, self->nevents);
    struct txlib_event* end = picotm_arrayat(self->event, newnevents);

    for (; beg < end; ++beg, ++self->nevents) {

        init(beg, data1, data2, data3, error);
        if (picotm_error_is_set(error)) {
            return;
        }

        picotm_append_event(self->module, beg - self->event, (uintptr_t)NULL,
                            error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }
}

/*
 * Module interface
 */

void
txlib_tx_apply_event(struct txlib_tx* self, const struct picotm_event* event,
                     struct picotm_error* error)
{
    assert(self);
    assert(event);
    assert(event->call < self->nevents);

    txlib_event_apply(self->event + event->call, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
txlib_tx_undo_event(struct txlib_tx* self, const struct picotm_event* event,
                    struct picotm_error* error)
{
    assert(self);
    assert(event);
    assert(event->call < self->nevents);

    txlib_event_undo(self->event + event->call, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
txlib_tx_finish(struct txlib_tx* self)
{
    assert(self);

    self->nevents = 0;

    struct txlist_tx_entry* entry = SLIST_FIRST(&self->acquired_list_tx);

    while (entry) {
        txlist_tx_finish(&entry->list_tx);
        txlist_tx_uninit(&entry->list_tx);
        SLIST_REMOVE_HEAD(&self->acquired_list_tx, slist_entry);
        SLIST_INSERT_HEAD(&self->allocated_list_tx, entry, slist_entry);
        entry = SLIST_FIRST(&self->acquired_list_tx);
    }
}
