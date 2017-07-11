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

#include "picotm/stdlib-tm.h"
#include <errno.h>
#include <malloc.h>
#include <picotm/picotm-module.h>
#include <picotm/picotm.h>
#include <string.h>
#include "allocator/module.h"
#include "error/module.h"
#include "fd/module.h"

PICOTM_EXPORT
void
free_tm(void* ptr)
{
    allocator_module_free(ptr, malloc_usable_size(ptr));
}

PICOTM_EXPORT
char*
mkdtemp_tm(char* template)
{
    error_module_save_errno();

    char* str;

    do {
        str = mkdtemp(template);
        if (!str) {
            picotm_recover_from_errno(errno);
        }
    } while (!str);

    return str;
}

PICOTM_EXPORT
int
mkstemp_tm(char* template)
{
    error_module_save_errno();

    int res;

    do {
        res = fd_module_mkstemp(template);
        if (res < 0) {
            picotm_recover_from_errno(errno);
        }
    } while (res < 0);

    return res;
}

PICOTM_EXPORT
int
posix_memalign_tm(void** memptr, size_t alignment, size_t size)
{
    allocator_module_posix_memalign(memptr, alignment, size);
    return 0;
}

PICOTM_EXPORT
void
qsort_tm(void* base, size_t nel, size_t width,
         int (*compar)(const void*, const void*))
{
    qsort(base, nel, width, compar);
}

PICOTM_EXPORT
void*
realloc_tm(void* ptr, size_t size)
{
    error_module_save_errno();

    size_t usiz = malloc_usable_size(ptr);

    void* mem = NULL;

    if (size) {
        allocator_module_posix_memalign(&mem, 2 * sizeof(void*), size);
    }

    if (ptr && mem) {
        /* Valgrind might report invalid reads and out-of-bounds access
         * within this function. This is a false positive. The result of
         * malloc_usable_size() is the maximum available buffer space,
         * not the amount of allocated or valid memory. Any memcpy() could
         * therefore operate on uninitialized data.
         */
        memcpy(mem, ptr, size < usiz ? size : usiz);
    }

    if (ptr && !size) {
        allocator_module_free(ptr, usiz);
    }

    return mem;
}

PICOTM_EXPORT
int
rand_r_tm(unsigned int* seed)
{
    return rand_r(seed);
}
