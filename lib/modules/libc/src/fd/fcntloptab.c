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

#include "fcntloptab.h"
#include <assert.h>
#include <picotm/picotm-error.h>
#include <picotm/picotm-lib-tab.h>
#include <stdio.h>
#include <string.h>
#include "fcntlop.h"

unsigned long
fcntloptab_append(struct fcntlop** tab, size_t* nelems, int command,
                  const union fcntl_arg*value,
                  const union fcntl_arg* oldvalue,
                  struct picotm_error* error)
{
    assert(tab);
    assert(nelems);

    void *tmp = picotm_tabresize(*tab, *nelems, *nelems + 1,
                                 sizeof((*tab)[0]), error);
    if (picotm_error_is_set(error)) {
        return (unsigned long)-1;
    }
    *tab = tmp;

    fcntlop_init((*tab)+(*nelems), command, value, oldvalue);

    return (*nelems)++;
}

void
fcntloptab_clear(struct fcntlop **tab, size_t *nelems)
{
    assert(tab);
    assert(nelems);

    picotm_tabfree(*tab);
    *tab = NULL;
    *nelems = 0;
}

void
fcntloptab_dump(struct fcntlop *tab, size_t nelems)
{
    struct fcntlop *ent;

    assert(tab || !nelems);

    for (ent = tab; ent < tab+nelems; ++ent) {
        fprintf(stderr, "%ld: ", (long)(ent-tab));
        fcntlop_dump(ent);
        fprintf(stderr, "\n");
    }
}

