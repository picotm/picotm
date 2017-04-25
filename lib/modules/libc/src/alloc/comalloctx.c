/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "comalloctx.h"
#include <assert.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <picotm/picotm-module.h>
#include <picotm/picotm-tm.h>
#include "comalloc.h"
#include "rnd2wb.h"

static int
com_alloc_tx_apply_event(const struct event *event, size_t n, void *data)
{
    return com_alloc_apply_event(data, event, n);
}

static int
com_alloc_tx_undo_event(const struct event *event, size_t n, void *data)
{
    return com_alloc_undo_event(data, event, n);
}

static int
com_alloc_tx_finish(void *data)
{
    com_alloc_finish(data);

    return 0;
}

static int
com_alloc_tx_uninit(void *data)
{
    com_alloc_uninit(data);

    return 0;
}

static struct com_alloc*
get_com_alloc()
{
    static __thread struct {
        bool             is_initialized;
        struct com_alloc instance;
    } t_com_alloc;

    if (t_com_alloc.is_initialized) {
        return &t_com_alloc.instance;
    }

    long res = picotm_register_module(NULL,
                                      NULL,
                                      NULL,
                                      com_alloc_tx_apply_event,
                                      com_alloc_tx_undo_event,
                                      NULL,
                                      NULL,
                                      com_alloc_tx_finish,
                                      com_alloc_tx_uninit,
                                      &t_com_alloc.instance);
    if (res < 0) {
        return NULL;
    }
    unsigned long module = res;

    res = com_alloc_init(&t_com_alloc.instance, module);
    if (res < 0) {
        return NULL;
    }

    t_com_alloc.is_initialized = true;

    return &t_com_alloc.instance;
}

void
com_alloc_tx_free(void *mem, size_t usiz)
{
    struct com_alloc *data = get_com_alloc();
    assert(data);

    com_alloc_exec_free(data, mem);
}

int
com_alloc_tx_posix_memalign(void** memptr, size_t alignment, size_t size)
{
    struct com_alloc* data = get_com_alloc();
    assert(data);

    return com_alloc_exec_posix_memalign(data, memptr, alignment, size);
}
