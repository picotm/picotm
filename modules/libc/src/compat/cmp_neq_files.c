/*
 * picotm - A system-level transaction manager
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

#include "cmp_neq_files.h"
#include "picotm/picotm-error.h"
#include <fcntl.h>
#if defined(HAVE_SYS_SYSCALL_H) && HAVE_SYS_SYSCALL_H
#include <sys/syscall.h>
#endif

#if defined(SYS_kcmp)
#include <errno.h>
#include <linux/kcmp.h>
#include <unistd.h>

static bool
cmp_eq_files_kcmp(int lhs, int rhs, struct picotm_error *error)
{
    pid_t pid = getpid();

    int ret = syscall(SYS_kcmp, pid, pid, KCMP_FILE, lhs, rhs);
    switch (ret) {
        case 0:
            return true;
        case 1:
        case 2:
        case 3: /* unordered sequences should not happen in our use case */
            return false;
        case -1: /* errors */
            picotm_error_set_errno(error, errno);
            break;
        default:
            picotm_error_set_error_code(error, PICOTM_GENERAL_ERROR);
            break;
    }

    return false;
}
#endif

#if defined(F_DUPFD_QUERY)
#include <errno.h>

static bool cmp_eq_files_dupfd(int lhs, int rhs, struct picotm_error *error)
{
    int ret = fcntl(lhs, F_DUPFD_QUERY, rhs);
    if (ret < 0) {
        picotm_error_set_errno(error, errno);
        return false;
    }

    return !!ret;
}
#endif

bool
cmp_neq_files(int lhs, int rhs, struct picotm_error* error)
{
#if defined(SYS_kcmp)
    bool neq = !cmp_eq_files_kcmp(lhs, rhs, error);
#if defined(F_DUPFD_QUERY)
    if (picotm_error_is_set(error)) {
        switch(error->status) {
            case PICOTM_ERRNO:
                if (error->value.errno_hint == -ENOSYS) {
                    picotm_error_clear(error);
                    /* kcmp isn't available, but maybe F_DUPFD_QUERY is */
                    return !cmp_eq_files_dupfd(lhs, rhs, error);
                }
                [[fallthrough]];
            default:
                break;
        }
    }
#endif
    return neq;
#elif defined(F_DUPFD_QUERY)
    return !cmp_eq_files_dupfd(lhs, rhs, error);
#else
#error file_id_cmp not implemented
#endif
}
