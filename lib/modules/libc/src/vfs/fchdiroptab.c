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

#include "fchdiroptab.h"
#include <assert.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-tab.h>
#include <stdio.h>
#include "fchdirop.h"

unsigned long
fchdiroptab_append(struct fchdirop **tab, size_t *nelems, int oldcwd,
                                                          int newcwd)
{
    assert(tab);
    assert(nelems);

    struct picotm_error error = PICOTM_ERROR_INITIALIZER;

    void *tmp = picotm_tabresize(*tab, *nelems, (*nelems)+1,
                                 sizeof((*tab)[0]), &error);
    if (picotm_error_is_set(&error)) {
        return -1;
    }
    *tab = tmp;

    if (fchdirop_init((*tab)+(*nelems), oldcwd, newcwd) < 0) {
        return -1;
    }

    return (*nelems)++;
}

void
fchdiroptab_clear(struct fchdirop **tab, size_t *nelems)
{
    assert(tab);
    assert(nelems);

    picotm_tabfree(*tab);
    *tab = NULL;
    *nelems = 0;
}

void
fchdiroptab_dump(struct fchdirop *tab, size_t nelems)
{
    struct fchdirop *ent;

    assert(tab || !nelems);

    for (ent = tab; ent < tab+nelems; ++ent) {
        fprintf(stderr, "%ld: ", (long)(ent-tab));
        fchdirop_dump(ent);
        fprintf(stderr, "\n");
    }
}

