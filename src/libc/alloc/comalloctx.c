/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "comalloctx.h"
#include <assert.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <systx/systx-module.h>
#include <systx/systx-tm.h>
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

    long res = systx_register_module(NULL,
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

int
com_alloc_tx_posix_memalign(void **memptr, size_t alignment, size_t size)
{
    struct com_alloc *data = get_com_alloc();
    assert(data);

    return com_alloc_exec_posix_memalign(data, memptr, alignment, size);
}

void
com_alloc_tx_free(void *mem)
{
    struct com_alloc *data = get_com_alloc();
    assert(data);

    /* Abort other transactions */

    size_t usiz = malloc_usable_size(mem);

    if (usiz) {
        privatize_tx(mem, usiz);
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

void*
com_alloc_tx_realloc(void* ptr, size_t siz)
{
    if (!ptr) {
        return com_alloc_tx_malloc(siz);
    } else if (!siz) {
        com_alloc_tx_free(ptr);
        return NULL;
    }

    void* mem = com_alloc_tx_malloc(siz);
    if (!mem) {
        return NULL;
    }

    /* Valgrind might report invalid reads and out-of-bounds access
     * within this function. This is a false positive. The result of
     * malloc_usable_size() is the maximum available buffer space,
     * not the amount of allocated or valid memory. Any memcpy() within
     * load_tx() could therefore operate on uninitialized data.
     */
    size_t usiz = malloc_usable_size(ptr);

    if (usiz) {
        load_tx(ptr, mem, usiz);
    }

    com_alloc_tx_free(ptr);

    return mem;
}
