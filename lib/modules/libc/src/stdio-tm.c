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

#include "picotm/stdio-tm.h"
#include <errno.h>
#include <picotm/picotm-module.h>
#include "error/module.h"

PICOTM_EXPORT
int
sscanf_tm(const char* s, const char* format, ...)
{
    va_list arg;

    va_start(arg, format);
    int res = vsscanf_tm(s, format, arg);
    va_end(arg);

    return res;
}

PICOTM_EXPORT
int
vsscanf_tm(const char* s, const char* format, va_list arg)
{
    error_module_save_errno();

    do {
        int res = vsscanf(s, format, arg);
        if (res < 0) {
            picotm_recover_from_errno(errno);
            continue;
        }
        return res;
    } while (true);
}
