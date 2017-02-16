/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <tanger-stm-internal.h>
#include <tanger-stm-internal-errcode.h>
#include <tanger-stm-internal-extact.h>
#include <tanger-stm-ext-actions.h>
#include "table.h"
#include "component.h"
#include "log.h"
#include "ext-actions.h"

static pthread_once_t keygen_once = PTHREAD_ONCE_INIT;
static pthread_key_t log_key;

static int
ext_actions_lock(void *txinit)
{
    int res = log_lock(pthread_getspecific(log_key));

    return res;
}

static int
ext_actions_unlock(void *txinit)
{

    int res = log_unlock(pthread_getspecific(log_key));

    return res;
}

static int
ext_actions_validate(void *txinit, int eotx)
{
    int res = log_validate(pthread_getspecific(log_key), eotx,
                           tanger_stm_is_noundo(tanger_stm_get_tx()));

    return res;
}

static int
ext_actions_apply_events(void *txinit)
{
    int noundo = tanger_stm_is_noundo(tanger_stm_get_tx());

    /* Apply events */

    int res = log_apply_events(pthread_getspecific(log_key), noundo);

    /* Update CC */

    res = log_updatecc(pthread_getspecific(log_key), noundo);

    return res;
}

static int
ext_actions_undo_events(void *txinit)
{
    int noundo = tanger_stm_is_noundo(tanger_stm_get_tx());

    /* Apply events */

    int res = log_undo_events(pthread_getspecific(log_key), noundo);

    /* Clear CC */

    res = log_clearcc(pthread_getspecific(log_key), noundo);

    return res;
}

static int
ext_actions_finish(void *txinit)
{
    int res = log_finish(pthread_getspecific(log_key));

    return res;
}

static void
ext_actions_uninit(void *log)
{
    log_uninit(log);
    free(log);
}

static void
ext_actions_generate_key(void)
{
    int err;

    if ( (err = pthread_key_create(&log_key, ext_actions_uninit)) ) {
        errno = err;
        perror("pthread_key_create");
        abort();
    }
}

static struct log *
ext_actions_aquire_log(void)
{
    struct log *log;

    pthread_once(&keygen_once, ext_actions_generate_key);

    // Aquire log data

    log = pthread_getspecific(log_key);

    if (!log) {

        log = malloc(sizeof(*log));
        assert(log);

        if (log_init(log) < 0) {
            free(log);
            return NULL;
        }

        pthread_setspecific(log_key, log);
    }

    // Register with STM

    tanger_stm_tx_t *tx = tanger_stm_get_tx();
    assert(tx);

    if (!tanger_stm_get_data(tx)) {
        tanger_stm_set_ext_calls(tx, ext_actions_lock,
                                     ext_actions_unlock,
                                     ext_actions_validate,
                                     ext_actions_apply_events,
                                     ext_actions_undo_events,
                                     ext_actions_finish);
        tanger_stm_set_data(tx, (void*)1);
    }

    return log;
}

/* Framework-internal interface
 */

int
ext_actions_push_commit_error_handler(stm_error_handler onerr)
{
    struct log *log = ext_actions_aquire_log();
    assert(log);

    void *tmp = tabresize(log->errhdlrtab,
                          log->errhdlrtablen,
                          log->errhdlrtablen+1, sizeof(log->errhdlrtab[0]));
    if (!tmp) {
        return ERR_SYSTEM;
    }
    log->errhdlrtab = tmp;

    log->errhdlrtab[log->errhdlrtablen] = onerr;
    log->errhdlrtablen++;

    return 0;
}

int
ext_actions_pop_commit_error_handler()
{
    struct log *log = ext_actions_aquire_log();
    assert(log);

    assert(log->errhdlrtablen);
    --log->errhdlrtablen;

    return 0;
}

/* Public interface
 */

int
tanger_stm_push_error_handler(stm_error_handler f)
{
    extern int com_error_tx_push_commit_error_handler(stm_error_handler);
    return com_error_tx_push_commit_error_handler(f);
}

int
tanger_stm_pop_error_handler()
{
    extern int com_error_tx_pop_commit_error_handler();
    return com_error_tx_pop_commit_error_handler();
}

int
tanger_stm_failoncommit()
{
    extern int com_error_tx_failoncommit(void);
    return com_error_tx_failoncommit();
}

static enum ccmode g_ofd_ccmode[] = {CC_MODE_NOUNDO,
                                     CC_MODE_2PL,
                                     CC_MODE_TS,
                                     CC_MODE_TS};

void
tanger_stm_ofd_type_set_ccmode(enum ofd_type ofdtype, enum ccmode ccmode)
{
    assert(ofdtype < sizeof(g_ofd_ccmode)/sizeof(g_ofd_ccmode[0]));
    g_ofd_ccmode[ofdtype] = ccmode;
}

enum ccmode
tanger_stm_ofd_type_get_ccmode(enum ofd_type ofdtype)
{
    assert(ofdtype < sizeof(g_ofd_ccmode)/sizeof(g_ofd_ccmode[0]));
    return g_ofd_ccmode[ofdtype];
}

int
tanger_stm_register_component(enum component_name component,
                              int (*lock)(void*),
                              int (*unlock)(void*),
                              int (*validate)(void*, int),
                              int (*apply_event)(const struct event*, size_t, void*),
                              int (*undo_event)(const struct event*, size_t, void*),
                              int (*updatecc)(void*, int),
                              int (*clearcc)(void*, int),
                              int (*finish)(void*),
                              int (*uninit)(void*),
                              int (*tpc_request)(void*, int),
                              int (*tpc_success)(void*, int),
                              int (*tpc_failure)(void*, int),
                              int (*tpc_noundo)(void*, int),
                              void *data)
{
    struct log *log = ext_actions_aquire_log();
    assert(log);

    return component_init(log_get_component_by_name(log, component),
                          lock,
                          unlock,
                          validate,
                          apply_event,
                          undo_event,
                          updatecc,
                          clearcc,
                          finish,
                          uninit,
                          tpc_request,
                          tpc_success,
                          tpc_failure,
                          tpc_noundo, data);
}

void *
tanger_stm_get_component_data(enum component_name component)
{
    struct log *log = ext_actions_aquire_log();
    assert(log);

    return component_get_data(log_get_component_by_name(log, component));
}

int
tanger_stm_inject_event(enum component_name component, unsigned short call,
                                                       unsigned short cookie)
{
    struct log *log = ext_actions_aquire_log();
    assert(log);

    return log_inject_event(log, component, call, cookie);
}

