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

#include "safe_stdlib.h"
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include "safeblk.h"
#include "taputils.h"

void*
safe_calloc(size_t nmemb, size_t size)
{
    void* mem = calloc(nmemb, size);
    if (size && !mem) {
        tap_error_errno("calloc()", size);
        abort_safe_block();
    }
    return mem;
}

void*
safe_malloc(size_t size)
{
    void* mem = malloc(size);
    if (size && !mem) {
        tap_error_errno("malloc()", size);
        abort_safe_block();
    }
    return mem;
}

char*
safe_mkdtemp(char* tmplate)
{
    char* path = mkdtemp(tmplate);
    if (!path) {
        tap_error_errno("mkdtemp()", errno);
        abort_safe_block();
    }
    return path;
}

int
safe_mkstemp(char* tmplate)
{
    int res = mkstemp(tmplate);
    if (res < 0) {
        tap_error_errno("mkstemp()", errno);
        abort_safe_block();
    }
    return res;
}

static bool
string_is_empty(const char* str)
{
    return !(*str);
}

char*
safe_mktemp(char* tmplate)
{
    char* filename = mktemp(tmplate);
    if (string_is_empty(filename)) {
        tap_error_errno("mktemp()", errno);
        abort_safe_block();
    }
    return filename;
}
