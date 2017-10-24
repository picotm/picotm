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

#pragma once

#include <setjmp.h>
#include <stddef.h>

void
_safe_block_set_jmpbuf(sigjmp_buf* jmpbuf, sigjmp_buf** old_jmpbuf);

#define begin_safe_block(_aborted)                          \
    {                                                       \
        sigjmp_buf _jmpbuf;                                 \
        sigjmp_buf* _old_jmpbuf;                            \
        _safe_block_set_jmpbuf(&_jmpbuf, &_old_jmpbuf);     \
        int _jmpval = sigsetjmp(_jmpbuf, 1);                \
        _aborted = !!_jmpval;                               \
        if (!_aborted) {

#define end_safe_block                              \
        }                                           \
        _safe_block_set_jmpbuf(_old_jmpbuf, NULL);  \
    }

void
abort_safe_block(void);
