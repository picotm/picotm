/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

/**
 * Wraps math functions in error-handling helpers.
 */
#define MATHERR(type_, res_, func_ , except_)   \
    matherr_save_and_clear((except_));          \
    type_ res_ = (func_);                       \
    matherr_test_and_resolve((except_));

void
matherr_save_and_clear(int except);

void
matherr_test_and_resolve(int except);
