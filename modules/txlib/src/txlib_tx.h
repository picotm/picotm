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

#pragma once

#include <stddef.h>

/**
 * \cond impl || txlib_impl
 * \ingroup txlib_impl
 * \file
 * \endcond
 */

struct picotm_error;
struct picotm_event;
struct txlib_event;

struct txlib_tx {
    unsigned long module;

    struct txlib_event* event;
    size_t              nevents;
};

/**
 * \brief Initializes a data-structure transaction.
 * \param self The data-stucture transaction to initialize.
 * \param module The module number.
 */
void
txlib_tx_init(struct txlib_tx* self, unsigned long module);

/**
 * \brief Cleans up a data-structure transaction.
 */
void
txlib_tx_uninit(struct txlib_tx* self);

/**
 * \brief Appends a new event to the txlib tranacstion's event log.
 */
void
txlib_tx_append_events1(struct txlib_tx* self, size_t nevents,
                        void (*init)(struct txlib_event*, void*,
                                     struct picotm_error*),
                        void* data1, struct picotm_error* error);

/**
 * \brief Appends a new event to the txlib tranacstion's event log.
 */
void
txlib_tx_append_events2(struct txlib_tx* self, size_t nevents,
                        void (*init)(struct txlib_event*, void*, void*,
                                     struct picotm_error*),
                        void* data1, void* data2,
                        struct picotm_error* error);

void
txlib_tx_append_events3(struct txlib_tx* self, size_t nevents,
                        void (*init)(struct txlib_event*, void*, void*, void*,
                                     struct picotm_error*),
                        void* data1, void* data2, void* data3,
                        struct picotm_error* error);

/**
 * \brief Allocates memory for a new event.
 * \param self The data-structure transaction.
 * \param[out] error Returns an error to the caller.
 * \returns The newly allocated event on success, or NULL on error.
 *
 * \attention A call to `txlib_tx_create_event()` allocates
 *            uninitialized memory. The caller is responsible for
 *            filling in the event fields and afterwards calling
 *            `txlib_tx_seal_event()` to activate the event.
 */
struct txlib_event*
txlib_tx_create_events(struct txlib_tx* self, size_t nevents,
                       struct picotm_error* error);

/**
 * \brief Discards the current events.
 * \param self The data-structure transaction.
 * \param nevents The number of events to discard.
 */
void
txlib_tx_discard_events(struct txlib_tx* self, size_t nevents);

/**
 * \brief Finishes and activates the current events.
 * \param[out] error Returns an error to the caller.
 * \param nevents The number of events to finish.
 * \param self The data-structure transaction.
 */
void
txlib_tx_seal_events(struct txlib_tx* self, size_t nevents,
                     struct picotm_error* error);

/*
 * Module interface
 */

void
txlib_tx_apply_event(struct txlib_tx* self,
                     const struct picotm_event* event,
                     struct picotm_error* error);

void
txlib_tx_undo_event(struct txlib_tx* self,
                    const struct picotm_event* event,
                    struct picotm_error* error);

void
txlib_tx_finish(struct txlib_tx* self);
