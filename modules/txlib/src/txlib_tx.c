/*
 * MIT License
 * Copyright (c) 2017-2018  Thomas Zimmermann <contact@tzimmermann.org>
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

#include "txlib_tx.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-tab.h"
#include "picotm/picotm-module.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include "txlib_event.h"

void
txlib_tx_init(struct txlib_tx* self, unsigned long module)
{
    assert(self);

    self->module = module;

    picotm_slist_init_head(&self->allocated_entries);
    picotm_slist_init_head(&self->acquired_list_tx);
    picotm_slist_init_head(&self->acquired_multiset_tx);
    picotm_slist_init_head(&self->acquired_queue_tx);
    picotm_slist_init_head(&self->acquired_stack_tx);

    self->event = NULL;
    self->nevents = 0;
}

static void
free_entry(struct txlib_tx_entry* entry)
{
    picotm_slist_uninit_item(&entry->slist_entry);
    free(entry);
}

static void
free_entry_cb(struct picotm_slist* item)
{
    free_entry(txlib_tx_entry_of_slist(item));
}

static void
free_allocated_txlib_tx_entries(struct txlib_tx* self)
{
    picotm_slist_cleanup_0(&self->allocated_entries, free_entry_cb);
}

void
txlib_tx_uninit(struct txlib_tx* self)
{
    assert(self);

    free_allocated_txlib_tx_entries(self);

    assert(picotm_slist_is_empty(&self->acquired_list_tx));
    assert(picotm_slist_is_empty(&self->acquired_multiset_tx));
    assert(picotm_slist_is_empty(&self->acquired_queue_tx));
    assert(picotm_slist_is_empty(&self->acquired_stack_tx));

    picotm_slist_uninit_head(&self->allocated_entries);
    picotm_slist_uninit_head(&self->acquired_list_tx);
    picotm_slist_uninit_head(&self->acquired_multiset_tx);
    picotm_slist_uninit_head(&self->acquired_queue_tx);
    picotm_slist_uninit_head(&self->acquired_stack_tx);

    picotm_tabfree(self->event);
}

struct txlib_tx_entry*
allocate_txlib_tx_entry(struct txlib_tx* self, struct picotm_error* error)
{
    struct txlib_tx_entry* entry;

    if (picotm_slist_is_empty(&self->allocated_entries)) {

        entry = malloc(sizeof(*entry));
        if (!entry) {
            picotm_error_set_errno(error, errno);
            return NULL;
        }
        picotm_slist_init_item(&entry->slist_entry);

    } else {
        entry = txlib_tx_entry_of_slist(
            picotm_slist_front(&self->allocated_entries));
        picotm_slist_dequeue_front(&self->allocated_entries);
    }

    return entry;
}

static _Bool
has_txlist_state(const struct txlib_tx_entry* entry, struct txlist_state* list_state)
{
    return entry->data.list_tx.list_state == list_state;
}

static _Bool
has_txlist_state_cb(const struct picotm_slist* item, void* list_state)
{
    return has_txlist_state(txlib_tx_entry_of_slist_const(item), list_state);
}

struct txlist_tx*
txlib_tx_acquire_txlist_of_state(struct txlib_tx* self,
                                 struct txlist_state* list_state,
                                 struct picotm_error* error)
{
    assert(self);

    struct picotm_slist* item = picotm_slist_find_1(&self->acquired_list_tx,
                                                    has_txlist_state_cb,
                                                    list_state);
    if (item != picotm_slist_end(&self->acquired_list_tx)) {
        struct txlib_tx_entry* entry = txlib_tx_entry_of_slist(item);
        return &entry->data.list_tx;
    }

    struct txlib_tx_entry* entry = allocate_txlib_tx_entry(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    txlist_tx_init(&entry->data.list_tx, list_state, self);
    picotm_slist_enqueue_front(&self->acquired_list_tx, &entry->slist_entry);

    return &entry->data.list_tx;
}

static _Bool
has_txmultiset_state(const struct txlib_tx_entry* entry,
                           struct txmultiset_state* multiset_state)
{
    return entry->data.multiset_tx.multiset_state == multiset_state;
}

static _Bool
has_txmultiset_state_cb(const struct picotm_slist* item, void* multiset_state)
{
    return has_txmultiset_state(txlib_tx_entry_of_slist_const(item),
                                multiset_state);
}

struct txmultiset_tx*
txlib_tx_acquire_txmultiset_of_state(struct txlib_tx* self,
                                     struct txmultiset_state* multiset_state,
                                     struct picotm_error* error)
{
    assert(self);

    struct picotm_slist* item = picotm_slist_find_1(
        &self->acquired_multiset_tx,
        has_txmultiset_state_cb,
        multiset_state);
    if (item != picotm_slist_end(&self->acquired_multiset_tx)) {
        struct txlib_tx_entry* entry = txlib_tx_entry_of_slist(item);
        return &entry->data.multiset_tx;
    }

    struct txlib_tx_entry* entry = allocate_txlib_tx_entry(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    txmultiset_tx_init(&entry->data.multiset_tx, multiset_state, self);
    picotm_slist_enqueue_front(&self->acquired_multiset_tx,
                               &entry->slist_entry);

    return &entry->data.multiset_tx;
}

static _Bool
has_txqueue_state(const struct txlib_tx_entry* entry,
                        struct txqueue_state* queue_state)
{
    return entry->data.queue_tx.queue_state == queue_state;
}

static _Bool
has_txqueue_state_cb(const struct picotm_slist* item, void* queue_state)
{
    return has_txqueue_state(txlib_tx_entry_of_slist_const(item),
                             queue_state);
}

struct txqueue_tx*
txlib_tx_acquire_txqueue_of_state(struct txlib_tx* self,
                                  struct txqueue_state* queue_state,
                                  struct picotm_error* error)
{
    assert(self);

    struct picotm_slist* item = picotm_slist_find_1(&self->acquired_queue_tx,
                                                    has_txqueue_state_cb,
                                                    queue_state);
    if (item != picotm_slist_end(&self->acquired_queue_tx)) {
        struct txlib_tx_entry* entry = txlib_tx_entry_of_slist(item);
        return &entry->data.queue_tx;
    }

    struct txlib_tx_entry* entry = allocate_txlib_tx_entry(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    txqueue_tx_init(&entry->data.queue_tx, queue_state, self);
    picotm_slist_enqueue_front(&self->acquired_queue_tx, &entry->slist_entry);

    return &entry->data.queue_tx;
}

static _Bool
has_txstack_state(const struct txlib_tx_entry* entry,
                        struct txstack_state* stack_state)
{
    return entry->data.stack_tx.stack_state == stack_state;
}

static _Bool
has_txstack_state_cb(const struct picotm_slist* item, void* stack_state)
{
    return has_txstack_state(txlib_tx_entry_of_slist_const(item),
                             stack_state);
}

struct txstack_tx*
txlib_tx_acquire_txstack_of_state(struct txlib_tx* self,
                                  struct txstack_state* stack_state,
                                  struct picotm_error* error)
{
    assert(self);

    struct picotm_slist* item = picotm_slist_find_1(&self->acquired_stack_tx,
                                                    has_txstack_state_cb,
                                                    stack_state);
    if (item != picotm_slist_end(&self->acquired_stack_tx)) {
        struct txlib_tx_entry* entry = txlib_tx_entry_of_slist(item);
        return &entry->data.stack_tx;
    }

    struct txlib_tx_entry* entry = allocate_txlib_tx_entry(self, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    txstack_tx_init(&entry->data.stack_tx, stack_state, self);
    picotm_slist_enqueue_front(&self->acquired_stack_tx, &entry->slist_entry);

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

static size_t
lock_txqueue_tx_entry(struct txlib_tx_entry* entry,
                      struct picotm_error* error)
{
    txqueue_tx_lock(&entry->data.queue_tx, error);
    if (picotm_error_is_set(error)) {
        return 0;
    }
    return 1;
}

static size_t
lock_txqueue_tx_entry_cb(struct picotm_slist* item, void* data)
{
    return lock_txqueue_tx_entry(txlib_tx_entry_of_slist(item), data);
}

static void
lock_txqueue_tx_entries(struct txlib_tx* self, struct picotm_error* error)
{
    assert(self);

    picotm_slist_walk_1(&self->acquired_queue_tx, lock_txqueue_tx_entry_cb,
                        error);
}

static size_t
lock_txstack_tx_entry(struct txlib_tx_entry* entry,
                      struct picotm_error* error)
{
    txstack_tx_lock(&entry->data.stack_tx, error);
    if (picotm_error_is_set(error)) {
        return 0;
    }
    return 1;
}

static size_t
lock_txstack_tx_entry_cb(struct picotm_slist* item, void* data)
{
    return lock_txstack_tx_entry(txlib_tx_entry_of_slist(item), data);
}

static void
lock_txstack_tx_entries(struct txlib_tx* self, struct picotm_error* error)
{
    assert(self);

    picotm_slist_walk_1(&self->acquired_stack_tx, lock_txstack_tx_entry_cb,
                        error);
}

void
txlib_tx_prepare_commit(struct txlib_tx* self, struct picotm_error* error)
{
    /* Currently, no locking is required for lists and multisets. Both
     * data structures acquire their locks during the transaction's
     * execution phase. */

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
txlib_tx_apply_event(struct txlib_tx* self, unsigned short op,
                     uintptr_t cookie, struct picotm_error* error)
{
    assert(self);
    assert(cookie < self->nevents);

    txlib_event_apply(self->event + cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
txlib_tx_undo_event(struct txlib_tx* self, unsigned short op,
                    uintptr_t cookie, struct picotm_error* error)
{
    assert(self);
    assert(cookie < self->nevents);

    txlib_event_undo(self->event + cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

static void
cleanup_txlist_tx_entry(struct txlib_tx_entry* entry)
{
    txlist_tx_finish(&entry->data.list_tx);
    txlist_tx_uninit(&entry->data.list_tx);
}

static void
cleanup_txmultiset_tx_entry(struct txlib_tx_entry* entry)
{
    txmultiset_tx_finish(&entry->data.multiset_tx);
    txmultiset_tx_uninit(&entry->data.multiset_tx);
}

static void
cleanup_txqueue_tx_entry(struct txlib_tx_entry* entry)
{
    txqueue_tx_finish(&entry->data.queue_tx);
    txqueue_tx_uninit(&entry->data.queue_tx);
}

static void
cleanup_txstack_tx_entry(struct txlib_tx_entry* entry)
{
    txstack_tx_finish(&entry->data.stack_tx);
    txstack_tx_uninit(&entry->data.stack_tx);
}

static void
finish_txlib_tx_entry(struct txlib_tx_entry* entry,
                      struct picotm_slist* allocated_entries,
                      void (*cleanup)(struct txlib_tx_entry*))
{
    cleanup(entry);
    picotm_slist_enqueue_front(allocated_entries, &entry->slist_entry);
}

static void
finish_txlib_tx_entry_cb(struct picotm_slist* item, void* data1, void* data2)
{
    finish_txlib_tx_entry(txlib_tx_entry_of_slist(item), data1, data2);
}

static void
finish_txlib_tx_entries(struct picotm_slist* head,
                        struct picotm_slist* allocated_entries,
                        void (*cleanup)(struct txlib_tx_entry*))
{
    picotm_slist_cleanup_2(head, finish_txlib_tx_entry_cb,
                           allocated_entries, cleanup);
}

void
txlib_tx_finish(struct txlib_tx* self)
{
    assert(self);

    self->nevents = 0;

    finish_txlib_tx_entries(&self->acquired_list_tx,
                            &self->allocated_entries,
                            cleanup_txlist_tx_entry);
    finish_txlib_tx_entries(&self->acquired_multiset_tx,
                            &self->allocated_entries,
                            cleanup_txmultiset_tx_entry);
    finish_txlib_tx_entries(&self->acquired_queue_tx,
                            &self->allocated_entries,
                            cleanup_txqueue_tx_entry);
    finish_txlib_tx_entries(&self->acquired_stack_tx,
                            &self->allocated_entries,
                            cleanup_txstack_tx_entry);
}
