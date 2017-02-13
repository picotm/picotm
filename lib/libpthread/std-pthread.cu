/* Copyright (C) 2008-2009  Thomas Zimmermann
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

ceuta_hdrl(#ifndef TANGER_STM_STD_PTHREAD_H);
ceuta_hdrl(#define TANGER_STM_STD_PTHREAD_H);
ceuta_hdrl(#include <pthread.h>);

#include <pthread.h>

ceuta_excl(int,  pthread_attr_init,            pthread_attr_init,            pthread_attr_t *attr);
ceuta_excl(int,  pthread_attr_destroy,         pthread_attr_destroy,         pthread_attr_t *attr);
ceuta_excl(int,  pthread_attr_getdetachstate,  pthread_attr_getdetachstate,  pthread_attr_t *attr, int *detachstate);
ceuta_excl(int,  pthread_attr_getschedpolicy,  pthread_attr_getschedpolicy,  pthread_attr_t *attr, int *policy);
ceuta_excl(int,  pthread_attr_getschedparam,   pthread_attr_getschedparam,   pthread_attr_t *attr, struct sched_param *param);
ceuta_excl(int,  pthread_attr_getinheritsched, pthread_attr_getinheritsched, pthread_attr_t *attr, int *inherit);
ceuta_excl(int,  pthread_attr_getscope,        pthread_attr_getscope,        pthread_attr_t *attr, int *scope);
ceuta_excl(int,  pthread_attr_setdetachstate,  pthread_attr_setdetachstate,  pthread_attr_t *attr, int  detachstate);
ceuta_excl(int,  pthread_attr_setschedpolicy,  pthread_attr_setschedpolicy,  pthread_attr_t *attr, int  policy);
ceuta_excl(int,  pthread_attr_setschedparam,   pthread_attr_setschedparam,   pthread_attr_t *attr, const struct sched_param *param);
ceuta_excl(int,  pthread_attr_setinheritsched, pthread_attr_setinheritsched, pthread_attr_t *attr, int  inherit);
ceuta_excl(int,  pthread_attr_setscope,        pthread_attr_setscope,        pthread_attr_t *attr, int  scope);

ceuta_hdrl(static int tanger_wrapper_tanger_stm_pthread_create(pthread_t*, const pthread_attr_t*, void* (*)(void*), void*) __attribute__ ((weakref("pthread_create"))););

extern int
tanger_stm_pthread_create(pthread_t *thread, const pthread_attr_t *attr, void* (*start_routine)(void*), void *arg)
{
    tanger_stm_tx_t *tx;
    enum error_code err;

    tx = tanger_stm_get_tx();
    assert(tx);

    if ( (err = tanger_stm_go_noundo(tx)) ) {
        if (err == ERR_CONFLICT) {
            tanger_stm_abort_self(tx);
        } else {
            abort();
        }
    }

    return pthread_create(thread, attr, start_routine, arg);
}

ceuta_excl(int,  pthread_detach, pthread_detach, pthread_t thread);
ceuta_pure(int,  pthread_equal,  pthread_equal,  pthread_t t1, pthread_t t2);
ceuta_excl(void, pthread_exit,   pthread_exit,   void *value_ptr);
ceuta_excl(int,  pthread_join,   pthread_join,   pthread_t thread, void **value_ptr);

ceuta_hdrl(static int tanger_wrapper_tanger_stm_pthread_once(pthread_once_t*, void (*)(void)) __attribute__ ((weakref("pthread_once"))););

extern int
tanger_stm_pthread_once(pthread_once_t *once_control, void (*init_routine)(void))
{
    tanger_stm_tx_t *tx;
    enum error_code err;

    tx = tanger_stm_get_tx();
    assert(tx);

    if ( (err = tanger_stm_go_noundo(tx)) ) {
        if (err == ERR_CONFLICT) {
            tanger_stm_abort_self(tx);
        } else {
            abort();
        }
    }

    return pthread_once(once_control, init_routine);
}

ceuta_pure(pthread_t, pthread_self,  pthread_self);

ceuta_hdrl(#endif);

