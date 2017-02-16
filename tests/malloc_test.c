/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/* TODO
 *
 * Invalid reads within tanger_stm_std_realloc are false positive; it is not
 * possible to init the memcpy src correctly without getting a out-of-bounds
 * write, which is another false positive
 *
 * Test 5+6:
 *  - Allocation before tanger_begin leads to 'invalid free' and
 *    lost bytes from that alloc
 *  - Doesn't occur without valgrind; false positive?
 */

#include <tanger-stm.h>
#include <tanger-stm-internal.h>
#include <tanger-stm-std-errno.h>
#include <tanger-stm-std-stdlib.h>
#include <tanger-stm-std-string.h>
#include <tanger-stm-std-stdio.h>
#include <tanger-stm-std-pthread.h>
#include <tanger-stm-malloc.h> /* Non-standard */
#include "malloc_test.h"

static volatile pthread_t g_curself;

static int
tanger_stm_tidcmp(pthread_t a, pthread_t b)
{
	return a-b;
}

static int
tanger_stm_spin_tid(pthread_t self)
{
    int i;

	for (i = 0; i < 10000; ++i) {
		g_curself = self;

		if (tanger_stm_tidcmp(self, g_curself)) {
		    return -1;
		}
	}

    return 0;
}

void
tanger_stm_malloc_test_1(unsigned int tid)
{
    tanger_begin();

    char *mem[10], **ptr;

    for (ptr = mem; ptr < mem+sizeof(mem)/sizeof(mem[0]); ++ptr) {

        *ptr = calloc(15, 34);

    	if (!*ptr) {
    	    perror("calloc");
    	    abort();
    	}
    }

    size_t j;
    for (j=0; j < sizeof(mem)/sizeof(mem[0]); ++j) {

        char * ptr = mem[j];
    /*for (ptr = mem; ptr < mem+sizeof(mem)/sizeof(mem[0]); ++ptr) {*/

        unsigned long i;

    	for (i = 0; i < (15*34); ++i) {
	        if (ptr[i]) {
                tanger_commit();
	            abort();
            }
        }
    }

    if (tanger_stm_spin_tid(pthread_self()) < 0) {
        tanger_commit();
        abort();
    }

    for (ptr = mem; ptr < mem+sizeof(mem)/sizeof(mem[0]); ++ptr) {
	    free(*ptr);
    }

    tanger_commit();
}

void
tanger_stm_malloc_test_2(unsigned int tid)
{
    char *mem[10], **ptr;

    for (ptr = mem; ptr < mem+sizeof(mem)/sizeof(mem[0]); ++ptr) {

        *ptr = calloc(15, 34);

    	if (!*ptr) {
    	    perror("calloc");
    	    abort();
    	}
    }

    tanger_begin();

    for (ptr = mem; ptr < mem+sizeof(mem)/sizeof(mem[0]); ++ptr) {

    	unsigned long i;

    	for (i = 0; i < (15*34); ++i) {
	        if ((*ptr)[i]) {
                tanger_commit();
            	fprintf(stderr, "content of mem[%ld] was %d, expected 0\n", i, (*ptr)[i]);
	            abort();
            }
        }
    }

    if (tanger_stm_spin_tid(pthread_self()) < 0) {
        tanger_commit();
        abort();
    }

    for (ptr = mem; ptr < mem+sizeof(mem)/sizeof(mem[0]); ++ptr) {
	    free(*ptr);
    }

    tanger_commit();
}

void
tanger_stm_malloc_test_3(unsigned int tid)
{
    tanger_begin();

    char *mem[10], **ptr;

    for (ptr = mem; ptr < mem+sizeof(mem)/sizeof(mem[0]); ++ptr) {

        *ptr = calloc(15, 34);

    	if (!*ptr) {
    	    perror("calloc");
    	    abort();
    	}
    }

    for (ptr = mem; ptr < mem+sizeof(mem)/sizeof(mem[0]); ++ptr) {

    	long i;

    	for (i = 0; i < (15*34); ++i) {
	        if ((*ptr)[i]) {
                tanger_commit();
            	fprintf(stderr, "content of mem[%ld] was %d, expected 0\n", i, (*ptr)[i]);
	            abort();
            }
        }
    }

    if (tanger_stm_spin_tid(pthread_self()) < 0) {
        tanger_commit();
        abort();
    }

    tanger_commit();

    for (ptr = mem; ptr < mem+sizeof(mem)/sizeof(mem[0]); ++ptr) {
	    free(*ptr);
    }
}

void
tanger_stm_malloc_test_4(unsigned int tid)
{
    tanger_begin();

    char *mem = malloc(30*sizeof(mem[0]));

    if (!mem) {
        perror("malloc");
        abort();
    }

    long i;

    for (i = 0; i < 30; ++i) {
        mem[i] = i;
    }

    mem = realloc(mem, 15*34*sizeof(mem[0]));

    if (!mem) {
        perror("realloc");
        abort();
    }

    for (i = 0; i < 30; ++i) {
        if (mem[i] != i) {
            tanger_commit();
            fprintf(stderr, "content of mem[%ld] was %d, expected %ld\n", i, mem[i], i);
            abort();
        }
    }

    for (i = 0; i < (15*34); ++i) {
        mem[i] = 0;
    }

    if (tanger_stm_spin_tid(pthread_self()) < 0) {
        tanger_commit();
        abort();
    }

    free(mem);

    tanger_commit();
}

void
tanger_stm_malloc_test_5(unsigned int tid)
{
    char *mem = malloc(30*sizeof(mem[0]));

	if (!mem) {
	    perror("malloc");
	    abort();
	}

	long i;

	for (i = 0; i < 30; ++i) {
	    mem[i] = i;
    }

	tanger_begin();

	mem = realloc(mem, 15*34*sizeof(mem[0]));

	if (!mem) {
	    perror("realloc");
	    abort();
	}

	for (i = 0; i < 30; ++i) {
	    if (mem[i] != i) {
	        abort();
        }
    }

	for (i = 0; i < (15*34); ++i) {
	    mem[i] = 0;
    }

    if (tanger_stm_spin_tid(pthread_self()) < 0) {
        tanger_commit();
        abort();
    }

	free(mem);
	mem = NULL;

	tanger_commit();
}

void
tanger_stm_malloc_test_6(unsigned int tid)
{
	long i;

    char *mem = malloc(30*sizeof(mem[0]));

	if (!mem) {
	    perror("malloc");
	    abort();
	}

	for (i = 0; i < 30; ++i) {
	    mem[i] = i;
    }

	tanger_begin();

	mem = realloc(mem, 15*34*sizeof(mem[0]));

	if (!mem) {
	    perror("realloc");
        abort();
	}

	for (i = 0; i < 30; ++i) {
	    if (mem[i] != i) {
	        abort();
        }
    }

	for (i = 0; i < (15*34); ++i) {
	    mem[i] = 0;
    }

    if (tanger_stm_spin_tid(pthread_self()) < 0) {
        tanger_commit();
        abort();
    }

	tanger_commit();

	free(mem);
}

void
tanger_stm_malloc_test_7(unsigned int tid)
{
    extern size_t g_txcycles;
    size_t i;
    size_t siz;

    siz = tid < 32 ? 32 : tid;

    tanger_begin();

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char *mem = malloc(siz);
        mem[tid] = 1;       /* touch memory to prevent optimization */
        free(mem);
    }

    tanger_commit();
}

void
tanger_stm_malloc_test_8(unsigned int tid)
{
    extern size_t g_txcycles;
    size_t i;

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char *mem = malloc(32);
        mem[tid] = 1;       /* touch memory to prevent optimization */
        free(mem);
    }
}

void
tanger_stm_malloc_test_9(unsigned int tid)
{
    extern size_t g_txcycles;
    tanger_stm_tx_t *tx;
    size_t i;

    tx = tanger_stm_get_tx();

    tanger_begin();

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char *mem = tanger_stm_malloc(32, tx);
        mem[tid] = 1;       /* touch memory to prevent optimization */
        tanger_stm_free(mem, tx);
    }

    tanger_commit();
}

