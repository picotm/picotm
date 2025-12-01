/*
 * picotm - A system-level transaction manager
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#include "sockbuf.h"
#include "picotm/picotm-lib-array.h"
#include "picotm/picotm-lib-tab.h"

static size_t
init_rwlock(void* data, struct picotm_error error[static 1])
{
    picotm_rwlock_init(data);

    return 1;
}

void
sockbuf_init(struct sockbuf self[static 1],
             struct picotm_error error[static 1])
{
    filebuf_init(&self->base, error);
    if (picotm_error_is_set(error))
        return;

    picotm_tabwalk_1(self->rwlock, picotm_arraylen(self->rwlock),
                     sizeof(self->rwlock[0]), init_rwlock, error);
    if (picotm_error_is_set(error))
        goto err_filebuf_uninit;

    return;

err_filebuf_uninit:
    filebuf_uninit(&self->base);
}

static size_t
uninit_rwlock(void* data, struct picotm_error error[static 1])
{
    picotm_rwlock_uninit(data);

    return 1;
}

void
sockbuf_uninit(struct sockbuf self[static 1])
{
    struct picotm_error ignored = PICOTM_ERROR_INITIALIZER;

    picotm_tabwalk_1(self->rwlock, picotm_arraylen(self->rwlock),
                     sizeof(self->rwlock[0]), uninit_rwlock, &ignored);

    filebuf_uninit(&self->base);
}

/*
 * Locking
 */

void
sockbuf_try_rdlock_field(struct sockbuf self[static 1],
                         enum sockbuf_field field,
                         struct picotm_rwstate rwstate[static 1],
                         struct picotm_error error[static 1])
{
    picotm_rwstate_try_rdlock(rwstate, self->rwlock + field, error);
}

void
sockbuf_try_wrlock_field(struct sockbuf self[static 1],
                         enum sockbuf_field field,
                         struct picotm_rwstate rwstate[static 1],
                         struct picotm_error error[static 1])
{
    picotm_rwstate_try_wrlock(rwstate, self->rwlock + field, error);
}

void
sockbuf_unlock_field(struct sockbuf self[static 1], enum sockbuf_field field,
                     struct picotm_rwstate rwstate[static 1])
{
    picotm_rwstate_unlock(rwstate, self->rwlock + field);
}
