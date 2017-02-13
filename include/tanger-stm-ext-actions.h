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

#ifndef TANGERSTMEXTACTIONS_H
#define TANGERSTMEXTACTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Error handling
 */

/**
 * \brief Return value of error handler, signals next step to framework
 */
enum stm_error_action
{
    STM_ERR_EXIT = 0, /**< \brief Abort process */
    STM_ERR_AGAIN, /**< \brief  Try again */
    STM_ERR_IGNORE /**< \brief  Go on with commit */
};

/** \brief Type of error-handler callback */
typedef enum stm_error_action (*stm_error_handler)(int component, int call, int cookie);

/**
 * \brief Push error handler to stack
 */
extern int tanger_stm_push_error_handler(stm_error_handler);

/**
 * \brief Pop error handler from stack
 */
extern int tanger_stm_pop_error_handler(void);

/**
 * \brief Push error handler to stack
 */
extern int tanger_stm_failoncommit(void);

static int tanger_wrapperpure_tanger_stm_push_error_handler(stm_error_handler)  __attribute__ ((weakref("tanger_stm_push_error_handler")));
static int tanger_wrapperpure_tanger_stm_pop_error_handler(void)                __attribute__ ((weakref("tanger_stm_pop_error_handler")));
static int tanger_wrapperpure_tanger_stm_failoncommit(void)                     __attribute__ ((weakref("tanger_stm_failoncommit")));

#ifdef __GNUC__

#define BEGINCALL(num)          \
        goto onerror ## num;    \
        call ## num:

#define ONERROR(num)        \
        goto end ## num;    \
        onerror ## num:     \
        {                   \
            enum stm_error_action __error_handler(int system, int call, int cookie) {   \

#define ENDCALL(num)                                        \
                return STM_ERR_EXIT;                        \
            }                                               \
            tanger_stm_push_error_handler(__error_handler); \
        }                                                   \
        goto call ## num;                                   \
        end ## num:                                         \
        tanger_stm_pop_error_handler();                     \

#endif

/*
 * File-I/O CC mode
 */

enum ofd_type
{
    TYPE_ANY = 0, /**< \brief Any file */
    TYPE_REGULAR, /**< \brief Regular file */
    TYPE_FIFO, /**< \brief FIFO */
    TYPE_SOCKET /**< \brief Socket */
};

/**
 * \brief Error handling
 */
enum ccmode
{
    CC_MODE_NOUNDO = 0, /**< \brief Set CC mode to irrevocablilty */
    CC_MODE_TS, /**< \brief Set CC mode to optimistic timestamp checking */
    CC_MODE_2PL, /**< \brief Set CC mode to pessimistic two-phase locking */
    CC_MODE_2PL_EXT /**< \brief (Inofficial)Set CC mode to pessimistic two-phase locking with socket commit protocol */
};

/**
 * \brief Set CC mode for specified file type
 */
extern void        tanger_stm_ofd_type_set_ccmode(enum ofd_type ofd_type, enum ccmode ccmode);

/**
 * \brief Get CC mode for specified file type
 */
extern enum ccmode tanger_stm_ofd_type_get_ccmode(enum ofd_type ofdtype);

static void        tanger_wrapperpure_tanger_stm_ofd_type_set_ccmode(enum ofd_type, enum ccmode) __attribute__ ((weakref("tanger_stm_ofd_type_set_ccmode")));
static enum ccmode tanger_wrapperpure_tanger_stm_ofd_type_get_ccmode(enum ofd_type)              __attribute__ ((weakref("tanger_stm_ofd_type_get_ccmode")));

/*
 * Component support
 */

enum component_name
{
    COMPONENT_ERROR = 0, /**< \brief Error handling */
    COMPONENT_ALLOC, /**< \brief Memory management */
    COMPONENT_FD, /**< \brief File-descriptor IO */
    COMPONENT_STREAM, /**< \brief Stream IO (Not used) */
    COMPONENT_FS, /**< \brief File-system operations */
    COMPONENT_USER0 = 8, /**< \brief User component */
    COMPONENT_USER1, /**< \brief User component */
    COMPONENT_USER2, /**< \brief User component */
    COMPONENT_USER3, /**< \brief User component */
    LAST_COMPONENT
};

struct event
{
    enum component_name component;
    unsigned short      call;
    unsigned short      cookie;
};

/**
 * \brief Register a new component with the transactions
 */
extern int tanger_stm_register_component(enum component_name component,
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
                                         void *cbdata);

/**
 * \brief Get data attached to component
 */
extern void* tanger_stm_get_component_data(enum component_name component);

/**
 * \brief Inject an event into log
 */
extern int   tanger_stm_inject_event(enum component_name component,
                                     unsigned short call,
                                     unsigned short cookie);

#ifdef __cplusplus
}
#endif

#endif

