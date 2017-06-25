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

#include "log.h"
#include <stdlib.h>
#include <picotm/picotm-module.h>
#include "module.h"
#include "table.h"

int
log_init(struct log* self)
{
    self->eventtab = NULL;
    self->eventtablen = 0;
    self->eventtabsiz = 0;

    return 0;
}

void
log_uninit(struct log* self)
{
    free(self->eventtab);
}

void
log_append_event(struct log* self, unsigned long module, unsigned long call,
                 uintptr_t cookie, struct picotm_error* error)
{
    if (self->eventtablen >= self->eventtabsiz) {

        size_t eventtabsiz = self->eventtabsiz + 1;

        void* tmp = tabresize(self->eventtab,
                              self->eventtabsiz, eventtabsiz,
                              sizeof(self->eventtab[0]),
                              error);
        if (picotm_error_is_set(error)) {
            return;
        }
        self->eventtab = tmp;
        self->eventtabsiz = eventtabsiz;
    }

    struct picotm_event* event = self->eventtab + self->eventtablen;

    event->cookie = cookie;
    event->module = module;
    event->call = call;

    ++self->eventtablen;
}

int
log_apply_events(struct log* self, const struct module* module, bool noundo,
                 struct picotm_error* error)
{
    /* Apply events in chronological order */

    const struct picotm_event* beg = self->eventtab;
    const struct picotm_event* end = self->eventtab + self->eventtablen;

    while (beg < end) {
        module_apply_event(module + beg->module, beg, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
        ++beg;
    }

    self->eventtablen = 0;

    return 0;
}

int
log_undo_events(struct log* self, const struct module* module, bool noundo,
                struct picotm_error* error)
{
    /* Undo events in reversed-chronological order */

    const struct picotm_event* beg = self->eventtab + self->eventtablen;
    const struct picotm_event* end = self->eventtab;

    while (beg > end) {
        --beg;
        module_undo_event(module + beg->module, beg, error);
        if (picotm_error_is_set(error)) {
            return -1;
        }
    }

    self->eventtablen = 0;

    return 0;
}
