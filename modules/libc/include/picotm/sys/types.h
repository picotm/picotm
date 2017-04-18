/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <picotm/picotm-tm.h>
#include <sys/types.h>

PICOTM_TM_LOAD_TX(blkcnt_t, blkcnt_t);
PICOTM_TM_LOAD_TX(blksize_t, blksize_t);
PICOTM_TM_LOAD_TX(clock_t, clock_t);
PICOTM_TM_LOAD_TX(clockid_t, clockid_t);
PICOTM_TM_LOAD_TX(dev_t, dev_t);
PICOTM_TM_LOAD_TX(fsblkcnt_t, fsblkcnt_t);
PICOTM_TM_LOAD_TX(fsfilcnt_t, fsfilcnt_t);
PICOTM_TM_LOAD_TX(gid_t, gid_t);
PICOTM_TM_LOAD_TX(id_t, id_t);
PICOTM_TM_LOAD_TX(ino_t, ino_t);
PICOTM_TM_LOAD_TX(key_t, key_t);
PICOTM_TM_LOAD_TX(mode_t, mode_t);
PICOTM_TM_LOAD_TX(nlink_t, nlink_t);
PICOTM_TM_LOAD_TX(off_t, off_t);
PICOTM_TM_LOAD_TX(pid_t, pid_t);
/*PICOTM_TM_LOAD_TX(size_t, size_t);*/ /* defined in stddef.h */
PICOTM_TM_LOAD_TX(ssize_t, ssize_t);
PICOTM_TM_LOAD_TX(suseconds_t, suseconds_t);
PICOTM_TM_LOAD_TX(time_t, time_t);
PICOTM_TM_LOAD_TX(timer_t, timer_t);
PICOTM_TM_LOAD_TX(uid_t, uid_t);

PICOTM_TM_STORE_TX(blkcnt_t, blkcnt_t);
PICOTM_TM_STORE_TX(blksize_t, blksize_t);
PICOTM_TM_STORE_TX(clock_t, clock_t);
PICOTM_TM_STORE_TX(clockid_t, clockid_t);
PICOTM_TM_STORE_TX(dev_t, dev_t);
PICOTM_TM_STORE_TX(fsblkcnt_t, fsblkcnt_t);
PICOTM_TM_STORE_TX(fsfilcnt_t, fsfilcnt_t);
PICOTM_TM_STORE_TX(gid_t, gid_t);
PICOTM_TM_STORE_TX(id_t, id_t);
PICOTM_TM_STORE_TX(ino_t, ino_t);
PICOTM_TM_STORE_TX(key_t, key_t);
PICOTM_TM_STORE_TX(mode_t, mode_t);
PICOTM_TM_STORE_TX(nlink_t, nlink_t);
PICOTM_TM_STORE_TX(off_t, off_t);
PICOTM_TM_STORE_TX(pid_t, pid_t);
/*PICOTM_TM_STORE_TX(size_t, size_t);*/ /* defined in stddef.h */
PICOTM_TM_STORE_TX(ssize_t, ssize_t);
PICOTM_TM_STORE_TX(suseconds_t, suseconds_t);
PICOTM_TM_STORE_TX(time_t, time_t);
PICOTM_TM_STORE_TX(uid_t, uid_t);
