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

#include "picotm_os_mutex.h"
#include <assert.h>
#include "picotm/picotm-error.h"
#include "picotm/picotm-module.h"

void
picotm_os_mutex_init(struct picotm_os_mutex* self, struct picotm_error* error)
{
    assert(self);

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);

    int err = pthread_mutex_init(&self->instance, &attr);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }

    pthread_mutexattr_destroy(&attr);
}

void
picotm_os_mutex_uninit(struct picotm_os_mutex* self)
{
    assert(self);

    do {
        int err = pthread_mutex_destroy(&self->instance);
        if (err) {
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_errno(&error, err);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
            continue;
        }
        break;
    } while (true);
}

void
picotm_os_mutex_lock(struct picotm_os_mutex* self, struct picotm_error* error)
{
    assert(self);

    int err = pthread_mutex_lock(&self->instance);
    if (err) {
        picotm_error_set_errno(error, err);
        return;
    }
}

void
picotm_os_mutex_unlock(struct picotm_os_mutex* self)
{
    assert(self);

    do {
        int err = pthread_mutex_unlock(&self->instance);
        if (err) {
            struct picotm_error error = PICOTM_ERROR_INITIALIZER;
            picotm_error_set_errno(&error, err);
            picotm_error_mark_as_non_recoverable(&error);
            picotm_recover_from_error(&error);
            continue;
        }
        break;
    } while (true);
}
