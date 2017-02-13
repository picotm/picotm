/* Copyright (C) 2009  Thomas Zimmermann
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include <tanger-stm.h>
#include <tanger-stm-ext-actions.h>
#include "error_test.h"

void
tanger_stm_error_test_1(unsigned int tid)
{
	tanger_begin();

    BEGINCALL(0)
        tanger_stm_failoncommit();
    ONERROR(0)
        fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
        return STM_ERR_IGNORE;
    ENDCALL(0)

    BEGINCALL(1)
        tanger_stm_failoncommit();
    ONERROR(1)
        fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
        return STM_ERR_IGNORE;
    ENDCALL(1)

	tanger_commit();
}

void
tanger_stm_error_test_2(unsigned int tid)
{
	tanger_begin();

    BEGINCALL(0)
        tanger_stm_failoncommit();

        BEGINCALL(1)
            tanger_stm_failoncommit();
        ONERROR(1)
            fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
            return STM_ERR_IGNORE;
        ENDCALL(1)

    ONERROR(0)
        fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
        return STM_ERR_IGNORE;
    ENDCALL(0)

	tanger_commit();
}

void
tanger_stm_error_test_3(unsigned int tid)
{
	tanger_begin();

    BEGINCALL(0)
        BEGINCALL(1)
            tanger_stm_failoncommit();
        ONERROR(1)
            fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
            return STM_ERR_IGNORE;
        ENDCALL(1)

        tanger_stm_failoncommit();

    ONERROR(0)
        fprintf(stderr, "%s:%d\n", __FILE__, __LINE__);
        return STM_ERR_IGNORE;
    ENDCALL(0)

	tanger_commit();
}

