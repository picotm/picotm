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

#include "safe_stdio.h"
#include <errno.h>
#include "safeblk.h"
#include "taputils.h"

/*
 * Interfaces with variable number of arguments
 */

int
safe_snprintf(char* str, size_t size, const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    int res = safe_vsnprintf(str, size, format, ap);
    va_end(ap);

    return res;
}

int
safe_sscanf(const char* str, const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    int res = safe_vsscanf(str, format, ap);
    va_end(ap);

    return res;
}

/*
 * Interfaces with va_list argument
 */

int
safe_vsnprintf(char* str, size_t size, const char* format, va_list ap)
{
    int res = vsnprintf(str, size, format, ap);
    if (res < 0) {
        tap_error_errno("vsnprintf()", errno);
        abort_safe_block();
    } else if ((size_t)res >= size) {
        tap_error("vsnprintf() output truncated\n");
        abort_safe_block();
    }
    return res;
}

int
safe_vsscanf(const char* str, const char* format, va_list ap)
{
    int res = vsscanf(str, format, ap);
    if (res < 0) {
        tap_error_errno("vsscanf()", errno);
        abort_safe_block();
    }
    return res;
}
