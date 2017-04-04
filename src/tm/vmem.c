/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "vmem.h"
#include <systx/systx-module.h>
#include "block.h"
#include "frame.h"

int
tm_vmem_init(struct tm_vmem* vmem)
{
    int res = pthread_rwlock_init(&vmem->lock, NULL);
    if (res < 0) {
        goto err_pthread_rwlock_init;
    }

    vmem->frametab = NULL;
    vmem->frametablen = 0;

    return 0;

err_pthread_rwlock_init:
    return res;
}

void
tm_vmem_uninit(struct tm_vmem* vmem)
{
    systx_tabfree(vmem->frametab);
    pthread_rwlock_destroy(&vmem->lock);
}

struct tm_frame*
tm_vmem_acquire_frame_by_block(struct tm_vmem* vmem, size_t block_index)
{
    do {
        /* read-lock while frame is being acquired */
        pthread_rwlock_rdlock(&vmem->lock);

        if (block_index < vmem->frametablen) {
            /* We return with the RD lock acquired. A call to
             * tm_vmem_release_frame() is required to clean up. */
            return vmem->frametab + block_index;
        }

        /* write-lock during re-allocation */
        pthread_rwlock_unlock(&vmem->lock);
        pthread_rwlock_wrlock(&vmem->lock);

        if (block_index < vmem->frametablen) {
            /* Another thread concurrently resized the table. Simply
             * loop and try again to acquire frame. */
            pthread_rwlock_unlock(&vmem->lock);
            continue;
        }

        size_t len = block_index + 1;
        void* tmp = systx_tabresize(vmem->frametab,
                                    vmem->frametablen, len,
                                    sizeof(vmem->frametab[0]));
        if (!tmp) {
            /* We're OOM. One of the upper layers has to handle
             * this problem. */
            pthread_rwlock_unlock(&vmem->lock);
            return NULL;
        }

        vmem->frametab = tmp;

        struct tm_frame* frame = vmem->frametab + vmem->frametablen;
        const struct tm_frame* frame_end = vmem->frametab + len;

        while (frame < frame_end) {
            tm_frame_init(frame, frame - vmem->frametab);
            ++frame;
        }

        vmem->frametablen = len;

        pthread_rwlock_unlock(&vmem->lock);
    } while (1);
}

struct tm_frame*
tm_vmem_acquire_frame_by_address(struct tm_vmem* vmem, uintptr_t addr)
{
    return tm_vmem_acquire_frame_by_block(vmem, tm_block_index_at(addr));
}

void
tm_vmem_release_frame(struct tm_vmem* vmem, struct tm_frame* frame)
{
    pthread_rwlock_unlock(&vmem->lock);
}
