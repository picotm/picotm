/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef EXT_ACTIONS_H
#define EXT_ACTIONS_H

int
ext_actions_push_commit_error_handler(stm_error_handler onerr);

int
ext_actions_pop_commit_error_handler();

#endif

