/* Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "module.h"
#include <assert.h>
#include <stdbool.h>
#include <picotm/picotm-module.h>
#include "allocator_tx.h"

struct allocator_module {
    bool                is_initialized;
    struct allocator_tx tx;
};

static void
apply_event_cb(const struct picotm_event* event, void* data,
               struct picotm_error* error)
{
    struct allocator_module* module = data;

    allocator_tx_apply_event(&module->tx, event, error);
}

static void
undo_event_cb(const struct picotm_event* event, void* data,
              struct picotm_error* error)
{
    struct allocator_module* module = data;

    allocator_tx_undo_event(&module->tx, event, error);
}

static void
finish_cb(void* data, struct picotm_error* error)
{
    struct allocator_module* module = data;

    allocator_tx_finish(&module->tx);
}

static void
uninit_cb(void* data)
{
    struct allocator_module* module = data;

    allocator_tx_uninit(&module->tx);
    module->is_initialized = false;
}

static struct allocator_tx*
get_allocator_tx(bool initialize, struct picotm_error* error)
{
    static __thread struct allocator_module t_module;

    if (t_module.is_initialized) {
        return &t_module.tx;
    } else if (!initialize) {
        return NULL;
    }

    unsigned long module = picotm_register_module(NULL, NULL, NULL,
                                                  NULL, NULL,
                                                  apply_event_cb,
                                                  undo_event_cb,
                                                  NULL, NULL,
                                                  finish_cb,
                                                  uninit_cb,
                                                  &t_module,
                                                  error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    allocator_tx_init(&t_module.tx, module);

    t_module.is_initialized = true;

    return &t_module.tx;
}

static struct allocator_tx*
get_non_null_allocator_tx(void)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        struct allocator_tx* allocator_tx = get_allocator_tx(true, &error);
        if (!picotm_error_is_set(&error)) {
            /* assert() here as there's no legal way that allocator_tx
             * could be NULL */
            assert(allocator_tx);
            return allocator_tx;
        }

        picotm_recover_from_error(&error);
    } while (true);
}

/*
 * Public interface
 */

void
allocator_module_free(void* mem, size_t usiz)
{
    struct allocator_tx* data = get_non_null_allocator_tx();

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        allocator_tx_exec_free(data, mem, &error);

        if (!picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

void
allocator_module_posix_memalign(void** memptr, size_t alignment, size_t size)
{
    struct allocator_tx* data = get_non_null_allocator_tx();
    assert(data);

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        allocator_tx_exec_posix_memalign(data, memptr, alignment, size, &error);

        if (!picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);

    } while (true);
}
