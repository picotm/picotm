/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <tanger-stm-internal.h>
#include <tanger-stm-internal-errcode.h>
#include <tanger-stm-internal-extact.h>
#include <tanger-stm-ext-actions.h>
#include "malloc.h"
#include "rnd2wb.h"
#include "comalloc.h"
#include "comalloctx.h"

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
    free(data);

    return 0;
}

struct com_alloc *
com_alloc_tx_aquire_data()
{
    struct com_alloc *data = tanger_stm_get_component_data(COMPONENT_ALLOC);

    if (!data) {

        data = malloc(sizeof(*data));

        if (!data) {
            return NULL;
        }

        com_alloc_init(data);

        /* See `lib/libta/ext-actions.c' for this function */
        int res = tanger_stm_register_component(COMPONENT_ALLOC,
                                                NULL,
                                                NULL,
                                                NULL,
                                                com_alloc_tx_apply_event,
                                                com_alloc_tx_undo_event,
                                                NULL,
                                                NULL,
                                                com_alloc_tx_finish,
                                                com_alloc_tx_uninit,
                                                NULL,
                                                NULL,
                                                NULL,
                                                NULL,
                                                data);

        if (res < 0) {
            abort();
        }
    }

    return data;
}

int
com_alloc_tx_posix_memalign(void **memptr, size_t alignment, size_t size)
{
    extern int com_alloc_exec_posix_memalign(struct com_alloc*, void**, size_t, size_t);

    struct com_alloc *data = com_alloc_tx_aquire_data();
    assert(data);

    return com_alloc_exec_posix_memalign(data, memptr, alignment, size);
}

void
com_alloc_tx_free(void *mem)
{
    extern void com_alloc_exec_free(struct com_alloc*, void*);

    size_t usiz;
    struct com_alloc *data = com_alloc_tx_aquire_data();
    assert(data);

    /* Abort other transactions */

    usiz = malloc_usable_size(mem);

    if (usiz) {
        tanger_stm_tx_t* tx = tanger_stm_get_tx();
        tanger_stm_store_mark_written(tx, mem, usiz);
    }

    com_alloc_exec_free(data, mem);
}

void *
com_alloc_tx_calloc(size_t nelem, size_t elsize)
{
    size_t size;
    void *ptr;

    size = rnd2wb(nelem*elsize);

    if (com_alloc_tx_posix_memalign(&ptr, sizeof(void*)*2, size) < 0) {
        return NULL;
    }

    return memset(ptr, 0, size);
}

void *
com_alloc_tx_malloc(size_t siz)
{
    void *ptr;

    if (com_alloc_tx_posix_memalign(&ptr, sizeof(void*)*2, siz) < 0) {
        return NULL;
    }

    return ptr;
}

void *
com_alloc_tx_realloc(void *ptr, size_t siz)
{
    if (!ptr) {
        return com_alloc_tx_malloc(siz);
    } else if (!siz) {
        com_alloc_tx_free(ptr);
        return NULL;
    }

    void *mem = com_alloc_tx_malloc(siz);

    if (!mem) {
        return NULL;
    }

    /* Copy data */

    size_t usiz = malloc_usable_size(ptr);

    if (usiz) {
        tanger_stm_tx_t *tx = tanger_stm_get_tx();
        tanger_stm_store_mark_written(tx, ptr, usiz);

        size_t n = siz<usiz?siz:usiz;

        void *src = tanger_stm_loadregionpre(tx, ptr, n);
        /* Might create warnings with valgrind
            when copying uninitialized bytes */
        memcpy(mem, src, n);
        tanger_stm_loadregionpost(tx, ptr, n);
    }

    struct com_alloc *data = com_alloc_tx_aquire_data();
    assert(data);

    /* Inject events */
    if (com_alloc_inject(data, ACTION_FREE, ptr) < 0) {
        return NULL;
    }

    return mem;
}

