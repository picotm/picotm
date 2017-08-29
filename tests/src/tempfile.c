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

#include "tempfile.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

static char        g_path_template[] = "/tmp/picotm-XXXXXX";
static const char* g_path = NULL;

const char*
temp_path()
{
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

    int err = pthread_mutex_lock(&lock);
    if (err) {
        abort();
    }

    if (g_path) {
        err = pthread_mutex_unlock(&lock);
        if (err) {
            abort();
        }
        return g_path;
    }

    char* path = mkdtemp(g_path_template);
    if (!path) {
        perror("snprintf");
        abort();
    }
    g_path = path;

    err = pthread_mutex_unlock(&lock);
    if (err) {
        abort();
    }

    return g_path;
}
