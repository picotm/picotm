/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
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
