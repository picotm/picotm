/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

ceuta_hdrl(#ifndef TANGER_STM_STDLIB_H);
ceuta_hdrl(#define TANGER_STM_STDLIB_H);
ceuta_hdrl(#include <stdlib.h>);

#include <stdlib.h>

ceuta_pure(void, abort,  abort);
ceuta_excl(int,  system, system, const char *command);

ceuta_hdrl(static void tanger_wrapper_tanger_stm_std_qsort(void*, size_t, size_t, int (*)(const void *, const void *)) __attribute__ ((weakref("qsort"))););

void
tanger_stm_std_qsort(void *base, size_t nel,
                                 size_t width,
                                 int (*compar)(const void *, const void *))
{
    tanger_stm_tx_t* tx = tanger_stm_get_tx();
    void *addr = tanger_stm_updateregionpre(tx, base, nel * width);
    qsort(addr, nel, width, compar);
}

extern char *
tanger_stm_std_mkdtemp(char *template)
{
    extern void* com_alloc_tx_malloc(size_t);
    extern void  com_alloc_tx_free(void*);

    char *arg0;
    unsigned int len=0;

    tanger_stm_tx_t *tx = tanger_stm_get_tx();
    assert(tx);

    if (template) {

        const void *tmp
            = tanger_stm_loadregionstring(tx, template, (uintptr_t*)&len);

        arg0 = com_alloc_tx_malloc(len+1);

        if (!arg0) {
            perror("malloc");
            abort();
        }
        memcpy(arg0, tmp, len);
        ((char*)arg0)[len] = '\0';
        tanger_stm_loadregionpost(tx, (uint8_t*)template, len);
    } else {
        arg0 = NULL;
    }

    char *res = mkdtemp(arg0);

    if (template) {
        tanger_stm_storeregion(tx, (uint8_t*)arg0, (uintptr_t)(len), (uint8_t*)template);
        com_alloc_tx_free(arg0);
    }

    return res ? template : NULL;
}

ceuta_hdrl(static char* tanger_wrapper_tanger_stm_std_mkdtemp(char*) __attribute__ ((weakref("mkdtemp"))););

extern int com_fs_tx_mkstemp(char*);

ceuta_wrap(int, mkstemp, com_fs_tx_mkstemp, [cstring|in|out] char *template);

/* Memory management
 */

extern int   com_alloc_tx_posix_memalign(void**, size_t, size_t);
extern void  com_alloc_tx_free(void*);
extern void* com_alloc_tx_calloc(size_t, size_t);
extern void* com_alloc_tx_malloc(size_t);
extern void* com_alloc_tx_realloc(void*, size_t);

ceuta_decl(int,   posix_memalign,  com_alloc_tx_posix_memalign, void **memptr, size_t alignment, size_t size);
ceuta_wrap(void,  free,            com_alloc_tx_free,           void *ptr);
ceuta_wrap(void*, realloc,         com_alloc_tx_realloc,        void *ptr, size_t size);
ceuta_wrap(void*, malloc,          com_alloc_tx_malloc,         size_t siz);
ceuta_wrap(void*, calloc,          com_alloc_tx_calloc,         size_t nelem, size_t elsize);

extern int
tanger_stm_std_posix_memalign(void **memptr, size_t alignment, size_t  size)
{
    void *mem;
    int res;

    tanger_stm_tx_t *tx = tanger_stm_get_tx();
    assert(tx);

    if (memptr) {
        tanger_stm_loadregion(tx, (uint8_t*)memptr, sizeof(*memptr), (uint8_t*)&mem);
    } else {
        mem = NULL;
    }

    res = com_alloc_tx_posix_memalign(&mem, alignment, size);

    if (memptr) {
        tanger_stm_storeregion(tx, (uint8_t*)&mem, (uintptr_t)sizeof(mem), (uint8_t*)memptr);
    }

    return res;
}

ceuta_pure(void,  tanger_stm_free,   tanger_stm_free,   void  *ptr,  tanger_stm_tx_t *tx);
ceuta_pure(void*, tanger_stm_malloc, tanger_stm_malloc, size_t size, tanger_stm_tx_t *tx);

/* ISO PRNG
 */

ceuta_decl(int,  rand,   rand);
ceuta_decl(void, srand,  srand,           unsigned int seed);
ceuta_wrap(int,  rand_r, rand_r, [in|out] unsigned int *seed);

static __thread unsigned rand_seed = 1;

int
tanger_stm_std_rand(void)
{
    return rand_r(&rand_seed);
}

void
tanger_stm_std_srand(unsigned int seed)
{
    rand_seed = seed;
}

ceuta_hdrl(#endif);

