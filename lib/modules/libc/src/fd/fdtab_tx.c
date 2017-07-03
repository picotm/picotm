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

#include "fdtab_tx.h"
#include <assert.h>
#include "fdtab.h"

void
fdtab_tx_init(struct fdtab_tx* self)
{
    assert(self);
}

void
fdtab_tx_uninit(struct fdtab_tx* self)
{
    assert(self);
}

struct fd*
fdtab_tx_ref_fildes(struct fdtab_tx* self, int fildes,
                    struct picotm_error* error)
{
    assert(self);

    return fdtab_ref_fildes(fildes, error);
}

/*
 * Module Interface
 */

void
fdtab_tx_update_cc(struct fdtab_tx* self, struct picotm_error* error)
{
    assert(self);
}

void
fdtab_tx_clear_cc(struct fdtab_tx* self, struct picotm_error* error)
{
    assert(self);
}
