/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

/**
 * Delays a transaction while generating potential conflicts.
 */
void
delay_transaction(unsigned int tid);

/**
 * Aborts a transaction if a error occured.
 */
void
abort_transaction_on_error(const char* origin);
