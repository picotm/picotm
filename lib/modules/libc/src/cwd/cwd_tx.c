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

#include "cwd_tx.h"
#include <assert.h>
#include <errno.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-array.h>
#include <picotm/picotm-lib-tab.h>
#include <picotm/picotm-module.h>
#include <stdlib.h>
#include <unistd.h>

enum cwd_tx_cmd {
    CMD_CHDIR,
    CMD_GETCWD,
    CMD_REALPATH
};

struct cwdop {
    union {
        struct {
            char* old_cwd;
        } chdir;
        struct {
            char* alloced_cwd;
        } getcwd;
        struct {
            char* alloced_res;
        } realpath;
    } data;
};

static struct cwdop*
append_cwdop(struct cwdop** tab, size_t* nelems, struct picotm_error* error)
{
    assert(tab);
    assert(nelems);

    void* tmp = picotm_tabresize(*tab, *nelems, *nelems + 1,
                                 sizeof((*tab)[0]), error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }
    *tab = tmp;

    struct cwdop* cwdop = (*tab) + (*nelems);
    ++(*nelems);

    return cwdop;
}

static void
init_rwstates(struct picotm_rwstate* beg, const struct picotm_rwstate* end)
{
    while (beg < end) {
        picotm_rwstate_init(beg);
        ++beg;
    }
}

static void
uninit_rwstates(struct picotm_rwstate* beg, const struct picotm_rwstate* end)
{
    while (beg < end) {
        picotm_rwstate_uninit(beg);
        ++beg;
    }
}

static void
unlock_rwstates(struct picotm_rwstate* beg, const struct picotm_rwstate* end,
                struct cwd* cwd)
{
    enum cwd_field field = 0;

    while (beg < end) {
        cwd_unlock_field(cwd, field, beg);
        ++field;
        ++beg;
    }
}

void
cwd_tx_init(struct cwd_tx* self, unsigned long module, struct cwd* cwd)
{
    assert(self);
    assert(cwd);

    self->module = module;
    self->cwd = cwd;

    init_rwstates(picotm_arraybeg(self->rwstate),
                  picotm_arrayend(self->rwstate));

    self->cwdoptab = NULL;
    self->cwdoptablen = 0;
}

void
cwd_tx_uninit(struct cwd_tx* self)
{
    assert(self);

    picotm_tabfree(self->cwdoptab);

    uninit_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate));
}

void
cwd_tx_try_rdlock_field(struct cwd_tx* self, enum cwd_field field,
                        struct picotm_error* error)
{
    assert(self);

    cwd_try_rdlock_field(self->cwd, field, self->rwstate + field, error);
}

void
cwd_tx_try_wrlock_field(struct cwd_tx* self, enum cwd_field field,
                        struct picotm_error* error)
{
    assert(self);

    cwd_try_wrlock_field(self->cwd, field, self->rwstate + field, error);
}

static void
append_event(struct cwd_tx* self, enum cwd_tx_cmd cmd,
             const struct cwdop* cwdop, struct picotm_error* error)
{
    assert(self);

    picotm_append_event(self->module, cmd, cwdop - self->cwdoptab, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

/*
 * chdir()
 */

int
cwd_tx_chdir_exec(struct cwd_tx* self, const char* path,
                  struct picotm_error* error)
{
    assert(self);

    cwd_tx_try_wrlock_field(self, CWD_FIELD_STRING, error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    struct cwdop* cwdop =  append_cwdop(&self->cwdoptab, &self->cwdoptablen,
                                        error);
    if (picotm_error_is_set(error)) {
        return -1;
    }

    char* cwd = get_current_dir_name();
    if (!cwd) {
        picotm_error_set_errno(error, errno);
        return -1;
    }

    cwdop->data.chdir.old_cwd = cwd;

    int res = chdir(path);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
        goto err_chdir;
    }

    append_event(self, CMD_CHDIR, cwdop, error);
    if (picotm_error_is_set(error)) {
        goto err_append_event;
    }

    return res;

err_append_event:
    res = chdir(cwd);
    if (res < 0) {
        /* We keep the original error code. We mark the error
         * as non-recoverable, because the function's clean-up
         * block failed. */
        picotm_error_mark_as_non_recoverable(error);
    }
err_chdir:
    free(cwd);
    return -1;
}

void
apply_chdir(struct cwd_tx* self, const struct cwdop* cwdop,
            struct picotm_error* error)
{
    assert(self);

    free(cwdop->data.chdir.old_cwd);
}

void
undo_chdir(struct cwd_tx* self, const struct cwdop* cwdop,
           struct picotm_error* error)
{
    assert(cwdop);

    /* The old working directory might have been deleted by an external
     * process. We cannot do much about it, but signal a transaction error.
     */
    int res = chdir(cwdop->data.chdir.old_cwd);
    if (res < 0) {
        picotm_error_set_errno(error, errno);
    }
}

/*
 * getcwd()
 */

char*
cwd_tx_getcwd_exec(struct cwd_tx* self, char* buf, size_t size,
                   struct picotm_error* error)
{
    assert(self);

    cwd_tx_try_rdlock_field(self, CWD_FIELD_STRING, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct cwdop* cwdop = append_cwdop(&self->cwdoptab, &self->cwdoptablen,
                                       error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    char* cwd = getcwd(buf, size);
    if (!cwd) {
        picotm_error_set_errno(error, errno);
        return NULL;
    }

    if (!buf && cwd) {
        /* Some implementations of getcwd() return a newly allocated
         * buffer if 'buf' is NULL. We free this buffer during a roll-
         * back. */
        cwdop->data.getcwd.alloced_cwd = cwd;
    } else {
        cwdop->data.getcwd.alloced_cwd = NULL;
    }

    append_event(self, CMD_CHDIR, cwdop, error);
    if (picotm_error_is_set(error)) {
        goto err_append_event;
    }

    return cwd;

err_append_event:
    if (!buf && cwd) {
        free(cwd);
    }
    return NULL;
}

void
apply_getcwd(struct cwd_tx* self, const struct cwdop* cwdop,
             struct picotm_error* error)
{
    assert(self);
}

void
undo_getcwd(struct cwd_tx* self, const struct cwdop* cwdop,
            struct picotm_error* error)
{
    assert(self);

    free(cwdop->data.getcwd.alloced_cwd);
}

/*
 * realpath()
 */

char*
cwd_tx_realpath_exec(struct cwd_tx* self, const char* path,
                     char* resolved_path, struct picotm_error* error)
{
    assert(self);

    cwd_tx_try_rdlock_field(self, CWD_FIELD_STRING, error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    struct cwdop* cwdop = append_cwdop(&self->cwdoptab, &self->cwdoptablen,
                                       error);
    if (picotm_error_is_set(error)) {
        return NULL;
    }

    char* res = realpath(path, resolved_path);
    if (!res) {
        picotm_error_set_errno(error, errno);
        return NULL;
    }

    if (!resolved_path && res) {
        /* Calls to realpath() return a newly allocated buffer if
         * 'resolved_path' is NULL. We free this buffer during a roll-
         * back. */
        cwdop->data.realpath.alloced_res = res;
    } else {
        cwdop->data.realpath.alloced_res = NULL;
    }

    append_event(self, CMD_REALPATH, cwdop, error);
    if (picotm_error_is_set(error)) {
        goto err_append_event;
    }

    return res;

err_append_event:
    if (!resolved_path && res) {
        free(res);
    }
    return NULL;
}

void
apply_realpath(struct cwd_tx* self, const struct cwdop* cwdop,
               struct picotm_error* error)
{
    assert(self);
}

void
undo_realpath(struct cwd_tx* self, const struct cwdop* cwdop,
              struct picotm_error* error)
{
    assert(self);

    free(cwdop->data.realpath.alloced_res);
}

/*
 * Module interface
 */

void
cwd_tx_apply_event(struct cwd_tx* self, const struct picotm_event* event,
                   struct picotm_error* error)
{
    static void (* const apply[])(struct cwd_tx*, const struct cwdop*,
                                  struct picotm_error*) = {
        apply_chdir,
        apply_getcwd,
        apply_realpath
    };

    assert(self);
    assert(event);
    assert(event->cookie < self->cwdoptablen);

    apply[event->call](self, self->cwdoptab + event->cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
cwd_tx_undo_event(struct cwd_tx* self, const struct picotm_event* event,
                  struct picotm_error* error)
{
    static void (* const undo[])(struct cwd_tx*, const struct cwdop*,
                                 struct picotm_error*) = {
        undo_chdir,
        undo_getcwd,
        undo_realpath
    };

    assert(self);
    assert(event);
    assert(event->cookie < self->cwdoptablen);

    undo[event->call](self, self->cwdoptab + event->cookie, error);
    if (picotm_error_is_set(error)) {
        return;
    }
}

void
cwd_tx_update_cc(struct cwd_tx* self, struct picotm_error* error)
{
    assert(self);

    /* release reader/writer locks on current working directory */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    self->cwd);
}

void
cwd_tx_clear_cc(struct cwd_tx* self, struct picotm_error* error)
{
    assert(self);

    /* release reader/writer locks on current working directory */
    unlock_rwstates(picotm_arraybeg(self->rwstate),
                    picotm_arrayend(self->rwstate),
                    self->cwd);
}

void
cwd_tx_finish(struct cwd_tx* self)
{
    assert(self);

    self->cwdoptablen = 0;
}
