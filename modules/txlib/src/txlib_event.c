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

#include "txlib_event.h"
#include <assert.h>
#include "picotm/picotm-error.h"

static void
process_event(struct txlib_event* event,
              void (* const op_func[])(struct txlib_event*,
                                       struct picotm_error*),
              struct picotm_error* error)
{
    assert(event);
    assert(op_func);

    if (!op_func[event->op]) {
        return;
    }

    op_func[event->op](event, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * Apply events
 */

void
txlib_event_apply(struct txlib_event* self, struct picotm_error* error)
{
    static void (* const apply[])(struct txlib_event*,
                                  struct picotm_error*) = {
        NULL
    };

    process_event(self, apply, error);
}

/*
 * Un-do events
 */

void
txlib_event_undo(struct txlib_event* self, struct picotm_error* error)
{
    static void (* const undo[])(struct txlib_event*,
                                 struct picotm_error*) = {
        NULL
    };

    process_event(self, undo, error);
}
