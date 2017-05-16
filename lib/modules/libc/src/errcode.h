/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

/**
 * \cond impl || libc_impl
 * \ingroup libc_impl
 * \file
 * \endcond
 */

enum error_code {
    ERR_SYSTEM = -1,
    ERR_CONFLICT = -2,
    ERR_NOUNDO = -4,
    ERR_DOMAIN = -5
};
