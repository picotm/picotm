/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017-2018  Thomas Zimmermann
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

#pragma once

#include "picotm/config/picotm-libc-config.h"
#include "picotm/compiler.h"
#include "picotm/picotm-tm.h"
#include <sys/types.h>

PICOTM_BEGIN_DECLS

/**
 * \ingroup group_libc
 * \file
 *
 * \brief Transactional wrappers for interfaces of <sys/types.h>.
 */

#if defined(PICOTM_LIBC_HAVE_TYPE_BLKCNT_T) && \
            PICOTM_LIBC_HAVE_TYPE_BLKCNT_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(blkcnt_t, blkcnt_t);
PICOTM_TM_STORE_TX(blkcnt_t, blkcnt_t);
PICOTM_TM_PRIVATIZE_TX(blkcnt_t, blkcnt_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_BLKSIZE_T) && \
            PICOTM_LIBC_HAVE_TYPE_BLKSIZE_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(blksize_t, blksize_t);
PICOTM_TM_STORE_TX(blksize_t, blksize_t);
PICOTM_TM_PRIVATIZE_TX(blksize_t, blksize_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_CLOCK_T) && \
            PICOTM_LIBC_HAVE_TYPE_CLOCK_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(clock_t, clock_t);
PICOTM_TM_STORE_TX(clock_t, clock_t);
PICOTM_TM_PRIVATIZE_TX(clock_t, clock_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_CLOCKID_T) && \
            PICOTM_LIBC_HAVE_TYPE_CLOCKID_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(clockid_t, clockid_t);
PICOTM_TM_STORE_TX(clockid_t, clockid_t);
PICOTM_TM_PRIVATIZE_TX(clockid_t, clockid_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_DEV_T) && PICOTM_LIBC_HAVE_TYPE_DEV_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(dev_t, dev_t);
PICOTM_TM_STORE_TX(dev_t, dev_t);
PICOTM_TM_PRIVATIZE_TX(dev_t, dev_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_FSBLKCNT_T) && \
            PICOTM_LIBC_HAVE_TYPE_FSBLKCNT_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(fsblkcnt_t, fsblkcnt_t);
PICOTM_TM_STORE_TX(fsblkcnt_t, fsblkcnt_t);
PICOTM_TM_PRIVATIZE_TX(fsblkcnt_t, fsblkcnt_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_FSFILCNT_T) && \
            PICOTM_LIBC_HAVE_TYPE_FSFILCNT_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(fsfilcnt_t, fsfilcnt_t);
PICOTM_TM_STORE_TX(fsfilcnt_t, fsfilcnt_t);
PICOTM_TM_PRIVATIZE_TX(fsfilcnt_t, fsfilcnt_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_GID_T) && PICOTM_LIBC_HAVE_TYPE_GID_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(gid_t, gid_t);
PICOTM_TM_STORE_TX(gid_t, gid_t);
PICOTM_TM_PRIVATIZE_TX(gid_t, gid_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_ID_T) && PICOTM_LIBC_HAVE_TYPE_ID_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(id_t, id_t);
PICOTM_TM_STORE_TX(id_t, id_t);
PICOTM_TM_PRIVATIZE_TX(id_t, id_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_INO_T) && PICOTM_LIBC_HAVE_TYPE_INO_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(ino_t, ino_t);
PICOTM_TM_STORE_TX(ino_t, ino_t);
PICOTM_TM_PRIVATIZE_TX(ino_t, ino_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_KEY_T) && PICOTM_LIBC_HAVE_TYPE_KEY_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(key_t, key_t);
PICOTM_TM_STORE_TX(key_t, key_t);
PICOTM_TM_PRIVATIZE_TX(key_t, key_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_MODE_T) && PICOTM_LIBC_HAVE_TYPE_MODE_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(mode_t, mode_t);
PICOTM_TM_STORE_TX(mode_t, mode_t);
PICOTM_TM_PRIVATIZE_TX(mode_t, mode_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_NLINK_T) && \
            PICOTM_LIBC_HAVE_TYPE_NLINK_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(nlink_t, nlink_t);
PICOTM_TM_STORE_TX(nlink_t, nlink_t);
PICOTM_TM_PRIVATIZE_TX(nlink_t, nlink_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_OFF_T) && PICOTM_LIBC_HAVE_TYPE_OFF_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(off_t, off_t);
PICOTM_TM_STORE_TX(off_t, off_t);
PICOTM_TM_PRIVATIZE_TX(off_t, off_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_PID_T) && PICOTM_LIBC_HAVE_TYPE_PID_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(pid_t, pid_t);
PICOTM_TM_STORE_TX(pid_t, pid_t);
PICOTM_TM_PRIVATIZE_TX(pid_t, pid_t);
/** \} */
#endif

/*PICOTM_TM_LOAD_TX(size_t, size_t);*/ /* defined in stddef.h */
/*PICOTM_TM_STORE_TX(size_t, size_t);*/ /* defined in stddef.h */

#if defined(PICOTM_LIBC_HAVE_TYPE_SSIZE_T) && \
            PICOTM_LIBC_HAVE_TYPE_SSIZE_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(ssize_t, ssize_t);
PICOTM_TM_STORE_TX(ssize_t, ssize_t);
PICOTM_TM_PRIVATIZE_TX(ssize_t, ssize_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_SUSECONDS_T) && \
            PICOTM_LIBC_HAVE_TYPE_SUSECONDS_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(suseconds_t, suseconds_t);
PICOTM_TM_STORE_TX(suseconds_t, suseconds_t);
PICOTM_TM_PRIVATIZE_TX(suseconds_t, suseconds_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_TIME_T) && PICOTM_LIBC_HAVE_TYPE_TIME_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(time_t, time_t);
PICOTM_TM_STORE_TX(time_t, time_t);
PICOTM_TM_PRIVATIZE_TX(time_t, time_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_TIMER_T) && \
            PICOTM_LIBC_HAVE_TYPE_TIMER_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(timer_t, timer_t);
PICOTM_TM_STORE_TX(timer_t, timer_t);
PICOTM_TM_PRIVATIZE_TX(timer_t, timer_t);
/** \} */
#endif

#if defined(PICOTM_LIBC_HAVE_TYPE_UID_T) && PICOTM_LIBC_HAVE_TYPE_UID_T || \
    defined(__PICOTM_DOXYGEN)
/** \addtogroup group_libc
 * \{ */
PICOTM_TM_LOAD_TX(uid_t, uid_t);
PICOTM_TM_STORE_TX(uid_t, uid_t);
PICOTM_TM_PRIVATIZE_TX(uid_t, uid_t);
/** \} */
#endif

PICOTM_END_DECLS
