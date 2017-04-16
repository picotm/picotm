/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "malloc_test.h"
#include <systx/stddef.h>
#include <systx/stdlib.h>
#include <systx/stdlib-tm.h>
#include <systx/systx.h>
#include <systx/systx-tm.h>
#include "ptr.h"
#include "testhlp.h"

/**
 * Allocate and free within transaction.
 */
void
malloc_test_1(unsigned int tid)
{
    systx_begin

        char* mem[10];

        for (char** ptr = mem; ptr < mem + arraylen(mem); ++ptr) {
            *ptr = calloc_tx(15, 34);
    	    if (!*ptr) {
    	        abort_tx();
    	    }
        }

        for (char** ptr = mem; ptr < mem + arraylen(mem); ++ptr) {
            const char *beg = *ptr;
            const char *end = *ptr + (15 * 34);
            while (beg < end) {
	            if (*beg) {
	                abort_tx();
                }
                ++beg;
            }
        }

        delay_transaction(tid);

        for (char** ptr = mem; ptr < mem + arraylen(mem); ++ptr) {
	        free_tm(*ptr);
        }

    systx_commit
    systx_end
}

/**
 * Allocate outside of transaction; free within transaction.
 */
void
malloc_test_2(unsigned int tid)
{
    char* buf[10];

    for (char** ptr = buf; ptr < buf + arraylen(buf); ++ptr) {
        *ptr = calloc(15, 34);
    	if (!*ptr) {
    	    abort();
    	}
    }

    systx_begin

        char* mem[10];

        for (size_t i = 0; i < arraylen(mem); ++i) {
            mem[i] = load_ptr_tx(buf + i);
        }

        for (char** ptr = mem; ptr < mem + arraylen(mem); ++ptr) {
            const char *beg = *ptr;
            const char *end = *ptr + (15 * 34);
            while (beg < end) {
	            if (*beg) {
	                abort_tx();
                }
                ++beg;
            }
        }

        delay_transaction(tid);

        for (char** ptr = mem; ptr < mem + arraylen(mem); ++ptr) {
	        free_tm(*ptr);
        }

    systx_commit
    systx_end
}

/**
 * Allocate within transaction; free outside of transaction.
 */
void
malloc_test_3(unsigned int tid)
{
    char* buf[10];

    systx_begin

        char* mem[10];

        for (char** ptr = mem; ptr < mem + arraylen(mem); ++ptr) {
            *ptr = calloc_tx(15, 34);
    	    if (!*ptr) {
    	        abort_tx();
    	    }
        }

        for (char** ptr = mem; ptr < mem + arraylen(mem); ++ptr) {
            const char *beg = *ptr;
            const char *end = *ptr + (15 * 34);
            while (beg < end) {
	            if (*beg) {
	                abort_tx();
                }
                ++beg;
            }
        }

        delay_transaction(tid);

        for (size_t i = 0; i < arraylen(buf); ++i) {
            store_ptr_tx(buf + i, mem[i]);
        }

    systx_commit
    systx_end

    for (char** ptr = buf; ptr < buf + arraylen(buf); ++ptr) {
	    free(*ptr);
    }
}

/**
 * Allocate, realloc and free within transaction.
 */
void
malloc_test_4(unsigned int tid)
{
    systx_begin

        uint8_t* mem = malloc_tx(30 * sizeof(mem[0]));
        if (!mem) {
            abort_tx();
        }
        for (uint8_t i = 0; i < 30; ++i) {
            mem[i] = i;
        }

        mem = realloc_tm(mem, 15 * 34 * sizeof(mem[0]));
        if (!mem) {
            abort_tx();
        }
        for (uint8_t i = 0; i < 30; ++i) {
            if (mem[i] != i) {
                abort_tx();
            }
        }

        for (int i = 0; i < (15 * 34); ++i) {
            mem[i] = 0;
        }

        delay_transaction(tid);

        free_tm(mem);

    systx_commit
    systx_end
}

/**
 * Allocate outside of transaction; realloc and free within transaction.
 */
void
malloc_test_5(unsigned int tid)
{
    uint8_t* buf = malloc(30 * sizeof(buf[0]));
	if (!buf) {
	    abort();
	}

	for (uint8_t i = 0; i < 30; ++i) {
	    buf[i] = i;
    }

	systx_begin

        /* Load 'buf' into transaction instead of using it
         * directly. The return value of realloc_tx() is
         * stored under the same name. So restarting the
         * transaction after realloc_tx() would free the
         * memory in 'buf' and create a dangling pointer.
         *
         * _Always_ loading and storing non-transactional
         * variables explicitly is ideomatic and avoids all
         * naming problems. */
        uint8_t* mem = load_ptr_tx(&buf);

	    mem = realloc_tm(mem, 15 * 34 * sizeof(mem[0]));
	    if (!mem) {
	        abort_tx();
	    }
	    for (uint8_t i = 0; i < 30; ++i) {
	        if (mem[i] != i) {
	            abort_tx();
            }
        }

	    for (int i = 0; i < (15 * 34); ++i) {
	        mem[i] = 0;
        }

        delay_transaction(tid);

	    free_tm(mem);

	systx_commit
    systx_end
}

/**
 * Allocate outside of transaction; realloc within transaction; free
 * outside of transaction.
 */
void
malloc_test_6(unsigned int tid)
{
    uint8_t* buf = malloc(30 * sizeof(buf[0]));
	if (!buf) {
	    abort();
	}
	for (uint8_t i = 0; i < 30; ++i) {
	    buf[i] = i;
    }

	systx_begin

        uint8_t* mem = load_ptr_tx(&buf);

	    mem = realloc_tm(mem, 15 * 34 * sizeof(mem[0]));
	    if (!mem) {
            abort_tx();
	    }

	    for (uint8_t i = 0; i < 30; ++i) {
	        if (mem[i] != i) {
	            abort_tx();
            }
        }

	    for (int i = 0; i < (15 * 34); ++i) {
	        mem[i] = 0;
        }

        delay_transaction(tid);

        /* _Always_ store results in non-transactional variables
         * explicitly, even if the variable is thread-local or
         * otherwise not shared.
         *
         * This is ideomatic and will work in all cases.
         */
        store_ptr_tx(&buf, mem);

	systx_commit
    systx_end

	free(buf);
}

static unsigned long long g_test_7_cycles;

void
malloc_test_7_pre(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    switch (btype) {
        case BOUND_CYCLES:
            g_test_7_cycles = bound;
            break;
        default:
            g_test_7_cycles = 10;
    }
}

/**
 * Multiple allocations and frees within transaction.
 */
void
malloc_test_7(unsigned int tid)
{
    const size_t tid_min32 = tid < 32 ? 32 : tid;

    systx_begin

        unsigned long long cycles = load_ullong_tx(&g_test_7_cycles);
        size_t siz = load_size_t_tx(&tid_min32);

        for (unsigned long long i = 0; i < cycles; ++i) {
            uint8_t* mem = malloc_tx(siz);
            mem[tid] = 1;       /* touch memory to prevent optimization */
            free_tm(mem);
        }

    systx_commit
    systx_end
}

static unsigned long long g_test_8_cycles;

void
malloc_test_8_pre(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    switch (btype) {
        case BOUND_CYCLES:
            g_test_8_cycles = bound;
            break;
        default:
            g_test_8_cycles = 10;
    }
}

/**
 * Multiple allocations and frees outside of transaction. Gives
 * non-transactional base line for comparing with malloc_test_9().
 */
void
malloc_test_8(unsigned int tid)
{
    for (unsigned long long i = 0; i < g_test_8_cycles; ++i) {
        uint8_t* mem = malloc(32);
        mem[tid] = 1;   /* touch memory to prevent optimization */
        free(mem);
    }
}

static unsigned long long g_test_9_cycles;

void
malloc_test_9_pre(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    switch (btype) {
        case BOUND_CYCLES:
            g_test_9_cycles = bound;
            break;
        default:
            g_test_9_cycles = 10;
    }
}

/**
 * Multiple allocations ands frees within transaction.
 */
void
malloc_test_9(unsigned int tid)
{
    systx_begin

        unsigned long long cycles = load_ullong_tx(&g_test_9_cycles);

        for (unsigned long long i = 0; i < cycles; ++i) {
            uint8_t* mem = malloc_tx(32);
            mem[tid] = 1;   /* touch memory to prevent optimization */
            free_tm(mem);
        }

    systx_commit
    systx_end
}
