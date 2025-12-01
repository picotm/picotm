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

#include "pipeoptab.h"
#include "picotm/picotm-error.h"
#include "picotm/picotm-lib-tab.h"
#include <assert.h>
#include "pipeop.h"

unsigned long
pipeoptab_append(struct pipeop** tab, size_t* nelems, int pipefd[2],
                 struct picotm_error* error)
{
    assert(tab);
    assert(nelems);

    void* tmp = picotm_tabresize(*tab, *nelems, (*nelems) + 1,
                                 sizeof((*tab)[0]), error);
    if (picotm_error_is_set(error)) {
        return (unsigned long)-1;
    }
    *tab = tmp;

    pipeop_init((*tab) + (*nelems), pipefd);

    return (*nelems)++;
}

void
pipeoptab_clear(struct pipeop** tab, size_t* nelems)
{
    assert(tab);
    assert(nelems);

    picotm_tabfree(*tab);
    *tab = NULL;
    *nelems = 0;
}
