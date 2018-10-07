/*
 * MIT License
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
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

#pragma once

/**
 * \cond impl || txlib_impl
 * \ingroup txlib_impl
 * \file
 * \endcond
 */

struct picotm_error;
struct txlist_state;
struct txlist_tx;
struct txmultiset_state;
struct txmultiset_tx;
struct txqueue_state;
struct txqueue_tx;
struct txstack_state;
struct txstack_tx;

struct txlist_tx*
txlib_module_acquire_txlist_of_state(struct txlist_state* list_state,
                                     struct picotm_error* error);

struct txmultiset_tx*
txlib_module_acquire_txmultiset_of_state(
    struct txmultiset_state* multiset_state,
    struct picotm_error* error);

struct txqueue_tx*
txlib_module_acquire_txqueue_of_state(struct txqueue_state* queue_state,
                                      struct picotm_error* error);

struct txstack_tx*
txlib_module_acquire_txstack_of_state(struct txstack_state* stack_state,
                                      struct picotm_error* error);
