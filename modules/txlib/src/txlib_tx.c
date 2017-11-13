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

void
txlib_tx_init(struct txlib_tx* self, unsigned long module)
{
    assert(self);

    self->module = module;

    SLIST_INIT(&self->allocated_entries);
    SLIST_INIT(&self->acquired_list_tx);
    SLIST_INIT(&self->acquired_queue_tx);
    SLIST_INIT(&self->acquired_stack_tx);

    self->event = NULL;
    self->nevents = 0;
}

static void
free_allocated_txlib_tx_entries(struct txlib_tx* self)
{
    struct txlib_tx_entry* entry = SLIST_FIRST(&self->allocated_entries);

    for (; entry; entry = SLIST_FIRST(&self->allocated_entries)) {
        SLIST_REMOVE_HEAD(&self->allocated_entries, slist_entry);
        free(entry);
    }
}

void
txlib_tx_uninit(struct txlib_tx* self)
{
    assert(self);

    free_allocated_txlib_tx_entries(self);

    assert(SLIST_EMPTY(&self->acquired_list_tx));
    assert(SLIST_EMPTY(&self->acquired_queue_tx));
    assert(SLIST_EMPTY(&self->acquired_stack_tx));

    picotm_tabfree(self->event);
}

struct txlib_tx_entry*
allocate_txlib_tx_entry(struct txlib_tx* self, struct picotm_error* error)
{
    struct txlib_tx_entry* entry;

    if (SLIST_EMPTY(&self->allocated_entries)) {

        entry = malloc(sizeof(*entry));
        if (!entry) {
            picotm_error_set_errno(error, errno);
            return NULL;
        }
        memset(&entry->slist_entry, 0, sizeof(entry->slist_entry));

    } else {

        entry = SLIST_FIRST(&self->allocated_entries);
        SLIST_REMOVE_HEAD(&self->allocated_entries, slist_entry);
    }

    return entry;
}

struct txlist_tx*
txlib_tx_acquire_txlist_of_state(struct txlib_tx* self,
                                 struct txlist_state* list_state,
                                 struct picotm_error* error)
{
    assert(self);

    struct txlib_tx_entry* entry;

    SLIST_FOREACH(entry, &self->acquired_list_tx, slist_entry) {
        if (entry->data.list_tx.list_state == list_state) {
            return &entry->data.list_tx;
        }
    }

    entry = allocate_txlib_tx_entry(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    txlist_tx_init(&entry->data.list_tx, list_state, self);
    SLIST_INSERT_HEAD(&self->acquired_list_tx, entry, slist_entry);

    return &entry->data.list_tx;
}

struct txqueue_tx*
txlib_tx_acquire_txqueue_of_state(struct txlib_tx* self,
                                  struct txqueue_state* queue_state,
                                  struct picotm_error* error)
{
    assert(self);

    struct txlib_tx_entry* entry;

    SLIST_FOREACH(entry, &self->acquired_queue_tx, slist_entry) {
        if (entry->data.queue_tx.queue_state == queue_state) {
            return &entry->data.queue_tx;
        }
    }

    entry = allocate_txlib_tx_entry(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    txqueue_tx_init(&entry->data.queue_tx, queue_state, self);
    SLIST_INSERT_HEAD(&self->acquired_queue_tx, entry, slist_entry);

    return &entry->data.queue_tx;
}

struct txstack_tx*
txlib_tx_acquire_txstack_of_state(struct txlib_tx* self,
                                  struct txstack_state* stack_state,
                                  struct picotm_error* error)
{
    assert(self);

    struct txlib_tx_entry* entry;

    SLIST_FOREACH(entry, &self->acquired_stack_tx, slist_entry) {
        if (entry->data.stack_tx.stack_state == stack_state) {
            return &entry->data.stack_tx;
        }
    }

    entry = allocate_txlib_tx_entry(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    txstack_tx_init(&entry->data.stack_tx, stack_state, self);
    SLIST_INSERT_HEAD(&self->acquired_stack_tx, entry, slist_entry);

    return &entry->data.stack_tx;
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

        picotm_append_event(self->module, 0, beg - self->event, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }
}

static void
init2(struct txlib_event* event, void* data1, void* data2, void* data3,
      struct picotm_error* error)
{
    void (*init)(struct txlib_event*, void*, void*, struct picotm_error*) =
        data3;

    init(event, data1, data2, error);
}

void
txlib_tx_append_events2(struct txlib_tx* self, size_t nevents,
                        void (*init)(struct txlib_event*, void*, void*,
                                     struct picotm_error*),
                        void* data1, void* data2,
                        struct picotm_error* error)
{
    txlib_tx_append_events3(self, nevents, init2, data1, data2, init, error);
}

static void
init1(struct txlib_event* event, void* data1, void* data2,
      struct picotm_error* error)
{
    void (*init)(struct txlib_event*, void*, struct picotm_error*) = data2;

    init(event, data1, error);
}

void
txlib_tx_append_events1(struct txlib_tx* self, size_t nevents,
                        void (*init)(struct txlib_event*, void*,
                                     struct picotm_error*),
                        void* data1, struct picotm_error* error)
{
    txlib_tx_append_events2(self, nevents, init1, data1, init, error);
}

/*
 * Module interface
 */

static void
lock_txqueue_tx_entries(struct txlib_tx* self, struct picotm_error* error)
{
    assert(self);

    struct txlib_tx_entry* entry;

    SLIST_FOREACH(entry, &self->acquired_queue_tx, slist_entry) {
        txqueue_tx_lock(&entry->data.queue_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }
}

static void
lock_txstack_tx_entries(struct txlib_tx* self, struct picotm_error* error)
{
    assert(self);

    struct txlib_tx_entry* entry;

    SLIST_FOREACH(entry, &self->acquired_stack_tx, slist_entry) {
        txstack_tx_lock(&entry->data.stack_tx, error);
        if (picotm_error_is_set(error)) {
            return;
        }
    }
}

void
txlib_tx_lock(struct txlib_tx* self, struct picotm_error* error)
{
    lock_txqueue_tx_entries(self, error);
    if (picotm_error_is_set(error)) {
        return;
    }

    lock_txstack_tx_entries(self, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
txlib_tx_apply_event(struct txlib_tx* self, const struct picotm_event* event,
                     struct picotm_error* error)
{
    assert(self);
    assert(event);
    assert(event->cookie < self->nevents);

    txlib_event_apply(self->event + event->cookie, error);
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
    assert(event->cookie < self->nevents);

    txlib_event_undo(self->event + event->cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
finish_txlist_tx_entries(struct txlib_tx* self)
{
    struct txlib_tx_entry* entry = SLIST_FIRST(&self->acquired_list_tx);

    for (; entry; entry = SLIST_FIRST(&self->acquired_list_tx)) {
        txlist_tx_finish(&entry->data.list_tx);
        txlist_tx_uninit(&entry->data.list_tx);

        SLIST_REMOVE_HEAD(&self->acquired_list_tx, slist_entry);
        SLIST_INSERT_HEAD(&self->allocated_entries, entry, slist_entry);
    }
}

static void
finish_txqueue_tx_entries(struct txlib_tx* self)
{
    struct txlib_tx_entry* entry = SLIST_FIRST(&self->acquired_queue_tx);

    for (; entry; entry = SLIST_FIRST(&self->acquired_queue_tx)) {
        txqueue_tx_finish(&entry->data.queue_tx);
        txqueue_tx_uninit(&entry->data.queue_tx);

        SLIST_REMOVE_HEAD(&self->acquired_queue_tx, slist_entry);
        SLIST_INSERT_HEAD(&self->allocated_entries, entry, slist_entry);
    }
}

static void
finish_txstack_tx_entries(struct txlib_tx* self)
{
    struct txlib_tx_entry* entry = SLIST_FIRST(&self->acquired_stack_tx);

    for (; entry; entry = SLIST_FIRST(&self->acquired_stack_tx)) {
        txstack_tx_finish(&entry->data.stack_tx);
        txstack_tx_uninit(&entry->data.stack_tx);

        SLIST_REMOVE_HEAD(&self->acquired_stack_tx, slist_entry);
        SLIST_INSERT_HEAD(&self->allocated_entries, entry, slist_entry);
    }
}

void
txlib_tx_finish(struct txlib_tx* self)
{
    assert(self);

    self->nevents = 0;

    finish_txlist_tx_entries(self);
    finish_txqueue_tx_entries(self);
    finish_txstack_tx_entries(self);
}
