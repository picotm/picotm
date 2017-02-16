/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <stddef.h>
#include <sys/types.h>
#include <tanger-stm-ext-actions.h>
#include "component.h"
#include "log.h"
#include "ext-actions.h"
#include "comerror.h"

int
com_error_exec_pop_commit_error_handler(struct com_error *data)
{
    return com_error_inject(data, ACTION_POP_ERROR_HANDLER, NULL);
}

int
com_error_apply_pop_commit_error_handler(struct com_error *data, unsigned int cookie)
{
    return ext_actions_pop_commit_error_handler();
}

int
com_error_undo_pop_commit_error_handler(struct com_error *data, unsigned int cookie)
{
    return 0;
}

