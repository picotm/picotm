/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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

