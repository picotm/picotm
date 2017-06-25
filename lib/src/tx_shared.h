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

#include <pthread.h>

/**
 * \cond impl || lib_impl
 * \ingroup lib_impl
 * \file
 * \endcond
 */

struct tx;

struct tx_shared {
    /**
     * Locks the transaction system for either one exclusive transactions,
     * or mutliple non-exclusive transactions.
     *
     * \todo This lock implements irrevocability. Replace this lock with a
     * more fine-grained system that allows recovable transactions while an
     * irrevocable transaction is present.
     */
    pthread_rwlock_t exclusive_tx_lock;
    struct tx*       exclusive_tx;
};

int
tx_shared_init(struct tx_shared* self);

void
tx_shared_uninit(struct tx_shared* self);

int
tx_shared_make_irrevocable(struct tx_shared* self, struct tx* exclusive_tx);

int
tx_shared_wait_irrevocable(struct tx_shared* self);

void
tx_shared_release_irrevocability(struct tx_shared* self);
