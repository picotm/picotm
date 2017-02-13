/* Copyright (C) 2009  Thomas Zimmermann
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <tanger-stm-ext-actions.h>
#include "comerror.h"
#include "comerrortx.h"

static int
com_error_tx_apply_event(const struct event *event, size_t n, void *data)
{
    return com_error_apply_event(data, event, n);
}

static int
com_error_tx_undo_event(const struct event *event, size_t n, void *data)
{
    return com_error_undo_event(data, event, n);
}

static int
com_error_tx_finish(void *data)
{
    com_error_finish(data);

    return 0;
}

static int
com_error_tx_uninit(void *data)
{
    com_error_uninit(data);

    free(data);

    return 0;
}

/* Public API
 */

struct com_error *
com_error_tx_aquire_data()
{
    struct com_error *data = tanger_stm_get_component_data(COMPONENT_ERROR);

    if (!data) {

        data = malloc(sizeof(*data));

        if (!data) {
            perror("malloc");
            return NULL;
        }

        com_error_init(data);

        int res = tanger_stm_register_component(COMPONENT_ERROR,
                                                NULL,
                                                NULL,
                                                NULL,
                                                com_error_tx_apply_event,
                                                com_error_tx_undo_event,
                                                NULL,
                                                NULL,
                                                com_error_tx_finish,
                                                com_error_tx_uninit,
                                                NULL,
                                                NULL,
                                                NULL,
                                                NULL,
                                                data);

        if (res < 0) {
            free(data);
            return NULL;
        }
    }

    return data;
}

int
com_error_tx_failoncommit()
{
    extern int com_error_exec_failoncommit(struct com_error*);

    struct com_error *data = com_error_tx_aquire_data();
    assert(data);

    return com_error_exec_failoncommit(data);
}

int
com_error_tx_pop_commit_error_handler()
{
    int com_error_exec_pop_commit_error_handler(struct com_error*);

    struct com_error *data = com_error_tx_aquire_data();
    assert(data);

    return com_error_exec_pop_commit_error_handler(data);
}

int
com_error_tx_push_commit_error_handler(stm_error_handler func)
{
    extern int com_error_exec_push_commit_error_handler(struct com_error*, 
                                                        stm_error_handler);

    struct com_error *data = com_error_tx_aquire_data();
    assert(data);

    return com_error_exec_push_commit_error_handler(data, func);
}

