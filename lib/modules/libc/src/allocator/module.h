/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include <stddef.h>

/**
 * \cond impl || libc_impl || libc_impl_allocator
 * \ingroup libc_impl
 * \ingroup libc_impl_allocator
 * \file
 * \endcond
 */

int
allocator_module_free(void* mem, size_t usiz);

int
allocator_module_posix_memalign(void** memptr, size_t alignment, size_t size);

/**
 * \cond impl || libc_impl || libc_impl_allocator
 * \defgroup libc_impl_allocator libc Allocator
 * \endcond
 */
