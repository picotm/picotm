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

#include "picotm/picotm-libc.h"
#include <assert.h>
#include <picotm/picotm-module.h>
#include <stdlib.h>
#include "fpu_tx.h"

/*
 * Module interface
 */

struct fpu_module {
    struct fpu_tx tx;
    bool          is_initialized;
};

static void
fpu_module_undo(struct fpu_module* module, struct picotm_error* error)
{
    fpu_tx_undo(&module->tx, error);
}

static void
fpu_module_finish(struct fpu_module* module, struct picotm_error* error)
{
    fpu_tx_finish(&module->tx);
}

static void
fpu_module_uninit(struct fpu_module* module)
{
    fpu_tx_uninit(&module->tx);
    module->is_initialized = false;
}

/*
 * Thread-local data
 */

static void
undo_cb(void* data, struct picotm_error* error)
{
    fpu_module_undo(data, error);
}

static void
finish_cb(void* data, struct picotm_error* error)
{
    fpu_module_finish(data, error);
}

static void
uninit_cb(void* data)
{
    fpu_module_uninit(data);
}

static struct fpu_tx*
get_fpu_tx(bool initialize, struct picotm_error* error)
{
    static __thread struct fpu_module t_module;

    if (t_module.is_initialized) {
        return &t_module.tx;
    } else if (!initialize) {
        return NULL;
    }

    unsigned long module = picotm_register_module(NULL, NULL, NULL,
                                                  NULL, undo_cb,
                                                  NULL, NULL,
                                                  NULL, NULL,
                                                  finish_cb,
                                                  uninit_cb,
                                                  &t_module,
                                                  error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    fpu_tx_init(&t_module.tx, module, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    t_module.is_initialized = true;

    return &t_module.tx;
}

static struct fpu_tx*
get_non_null_fpu_tx(void)
{
    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        struct fpu_tx* fpu_tx = get_fpu_tx(true, &error);

        if (!picotm_error_is_set(&error)) {
            /* assert() here as there's no legal way that fpu_tx
             * could be NULL */
            assert(fpu_tx);
            return fpu_tx;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

/*
 * Public interface
 */

void
fpu_module_save_fenv()
{
    struct fpu_tx* fpu_tx = get_non_null_fpu_tx();

    /* We have to save the floating-point enviroment
     * only once per transaction. */
    if (fpu_tx_fenv_saved(fpu_tx)) {
        return;
    }

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        fpu_tx_save_fenv(fpu_tx, &error);

        if (!picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);

    } while (true);
}

void
fpu_module_save_fexcept()
{
    struct fpu_tx* fpu_tx = get_non_null_fpu_tx();

    /* We have to save the floating-point status
     * flags only once per transaction. */
    if (fpu_tx_fexcept_saved(fpu_tx)) {
        return;
    }

    do {
        struct picotm_error error = PICOTM_ERROR_INITIALIZER;

        fpu_tx_save_fexcept(fpu_tx, &error);
        if (picotm_error_is_set(&error)) {
            return;
        }
        picotm_recover_from_error(&error);

    } while (true);
}
