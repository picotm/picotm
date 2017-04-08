/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#pragma once

#include "compiler.h"

SYSTX_BEGIN_DECLS

/**
 * File-type constants
 */
enum systx_libc_file_type {
    SYSTX_LIBC_FILE_TYPE_OTHER = 0, /**< \brief Any file */
    SYSTX_LIBC_FILE_TYPE_REGULAR,   /**< \brief Regular file */
    SYSTX_LIBC_FILE_TYPE_FIFO,      /**< \brief FIFO */
    SYSTX_LIBC_FILE_TYPE_SOCKET     /**< \brief Socket */
};

/**
 * File-I/O CC mode
 */
enum systx_libc_cc_mode {
    SYSTX_LIBC_CC_MODE_NOUNDO = 0,  /**< \brief Set CC mode to irrevocablilty */
    SYSTX_LIBC_CC_MODE_TS,          /**< \brief Set CC mode to optimistic timestamp checking */
    SYSTX_LIBC_CC_MODE_2PL,         /**< \brief Set CC mode to pessimistic two-phase locking */
    SYSTX_LIBC_CC_MODE_2PL_EXT      /**< \brief (Inofficial)Set CC mode to pessimistic two-phase locking with socket commit protocol */
};

/**
 * Validation mode
 */
enum systx_libc_validation_mode {
    SYSTX_LIBC_VALIDATE_OP = 0,
    SYSTX_LIBC_VALIDATE_DOMAIN,
    SYSTX_LIBC_VALIDATE_FULL
};

SYSTX_NOTHROW
/**
 * Sets the preferred mode of concurrency control for I/O on a specific
 * file type.
 */
void
systx_libc_set_file_type_cc_mode(enum systx_libc_file_type file_type,
                                 enum systx_libc_cc_mode cc_mode);

SYSTX_NOTHROW
/**
 * Returns the currently preferred mode of concurrency control for I/O on
 * a specific file type.
 */
enum systx_libc_cc_mode
systx_libc_get_file_type_cc_mode(enum systx_libc_file_type file_type);

SYSTX_NOTHROW
/**
 * Sets the mode of validation.
 */
void
systx_libc_set_validation_mode(enum systx_libc_validation_mode val_mode);

SYSTX_NOTHROW
/**
 * Returns the current mode of validation.
 */
enum systx_libc_validation_mode
systx_libc_get_validation_mode(void);

SYSTX_END_DECLS
