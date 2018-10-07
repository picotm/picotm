/*
 * picotm - A system-level transaction manager
 * Copyright (c) 2017   Thomas Zimmermann <contact@tzimmermann.org>
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

#include <unistd.h>

int
safe_chdir(const char* path);

int
safe_close(int fd);

char*
safe_getcwd(char* buf, size_t size);

#if defined(__MACH__)
char*
safe_mkdtemp(char* tmplate);
#endif

int
safe_pipe(int pipefd[2]);

ssize_t
safe_pread(int fd, void* buf, size_t count, off_t offset);

ssize_t
safe_pwrite(int fd, const void* buf, size_t count, off_t offset);

int
safe_rmdir(const char* path);

int
safe_unlink(const char* pathname);

ssize_t
safe_write(int fd, const void* buf, size_t count);
