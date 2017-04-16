/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#define _XOPEN_SOURCE 500 /* Needed for pwrite */

#include "fdio_test.h"
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <systx/fcntl.h>
#include <systx/stddef.h>
#include <systx/stdlib.h>
#include <systx/string.h>
#include <systx/string-tm.h>
#include <systx/sys/types.h>
#include <systx/systx.h>
#include <systx/systx-libc.h>
#include <systx/systx-tm.h>
#include <systx/unistd.h>
#include "ptr.h"
#include "testhlp.h"

static const char const g_test_str[] = "Hello world!\n";

/**
 * Open and close a file.
 */
void
fdio_test_1(unsigned int tid)
{
    systx_begin

        int fildes = open_tx("/tmp/fdio.test",
                             O_WRONLY | O_CREAT,
                             S_IRWXU | S_IRWXG | S_IRWXO);
        if (fildes < 0) {
            perror("open");
            abort_tx();
        }

        delay_transaction(tid);

        if (close_tx(fildes) < 0) {
            perror("close");
            abort_tx();
        }

    systx_commit
    systx_end
}

/**
 * Open-and-write test: Each thread open the file and writes a string.
 * Calling open_tx() with O_APPEND creates a load  dependency on the
 * file position, which is at the end of the file.
 */
void
fdio_test_2(unsigned int tid)
{
    systx_begin

        int fildes = open_tx("/tmp/fdio.test",
                             O_WRONLY | O_CREAT | O_APPEND,
                             S_IRWXU | S_IRWXG | S_IRWXO);

        if (fildes < 0) {
            perror("open");
            abort_tx();
        }

        if (write_tx(fildes, g_test_str, strlen_tm(g_test_str)) < 0) {
            perror("write");
            abort_tx();
        }

        delay_transaction(tid);

        if (close_tx(fildes) < 0) {
            perror("close");
            abort_tx();
        }

    systx_commit
    systx_end
}

/**
 * Each process writes to the beginning of a file. Only the last written
 * line should survive.
 */
void
fdio_test_3(unsigned int tid)
{
    systx_begin

        int fildes = open_tx("/tmp/fdio.test",
                             O_WRONLY | O_CREAT,
                             S_IRWXU | S_IRWXG | S_IRWXO);

        if (fildes < 0) {
            perror("open");
            abort_tx();
        }

        if (lseek_tx(fildes, 0, SEEK_SET) == (off_t)-1) {
            perror("lseek");
            abort_tx();
        }

        if (write_tx(fildes, g_test_str, strlen_tm(g_test_str)) < 0) {
            perror("write");
            abort_tx();
        }

        /*delay_transaction(tid)*/

        if (close_tx(fildes) < 0) {
            perror("close");
            abort_tx();
        }

    systx_commit
    systx_end
}

/**
 * Use pwrite to start writing at byte no. 2.
 *
 * \todo Initial two bytes in file are undefined; should probably
 * be 0.
 */
void
fdio_test_4(unsigned int tid)
{
    systx_begin

        int fildes = open_tx("/tmp/fdio.test",
                             O_WRONLY | O_CREAT,
                             S_IRWXU | S_IRWXG | S_IRWXO);

        if (fildes < 0) {
            perror("open");
            abort_tx();
        }

        if (pwrite_tx(fildes, g_test_str, strlen_tm(g_test_str), 2) < 0) {
            perror("pwrite");
            abort_tx();
        }

        /*delay_transaction(tid)*/

        if (close_tx(fildes) < 0) {
            perror("close");
            abort_tx();
        }

    systx_commit
    systx_end
}

/**
 * Write string, read-back and write again; should output 'Hello Hello!'.
 *
 * \todo Writes '<tid>ello <tid>ello'. Correct by code, but maybe do test
 * differently.
 */
void
fdio_test_5(unsigned int tid)
{
    char str[32];
    memcpy(str, g_test_str, strlen(g_test_str)+1);

    char tidstr[32];
    snprintf(tidstr, sizeof(tidstr), "%d", (int)tid);
    memcpy(str, tidstr, strlen(tidstr));

    systx_begin

        int fildes = open_tx("/tmp/fdio.test",
                             O_RDWR | O_CREAT,
                             S_IRWXU | S_IRWXG | S_IRWXO);

        if (fildes < 0) {
            perror("open");
            abort_tx();
        }

        if (pwrite_tx(fildes, str, strlen_tm(g_test_str), 0) < 0) {
            perror("pwrite");
            abort_tx();
        }

        char rbuf[5]; /* 5 characters for reading 'Hello' */
        memset_tm(rbuf, 0, sizeof(rbuf)); /* Workaround valgrind */

        if (pread_tx(fildes, rbuf, sizeof(rbuf), 0) < 0) {
            perror("pread");
            abort_tx();
        }

        if (pwrite_tx(fildes, rbuf, sizeof(rbuf), 6) < 0) {
            perror("pwrite");
            abort_tx();
        }

        delay_transaction(tid);

        if (close_tx(fildes) < 0) {
            perror("close");
            abort_tx();
        }

    systx_commit
    systx_end
}

/**
 * Write string at position 2 using lseek; should output '  Hello world!'.
 *
 * \todo Initial 2 bytes are undefined.
 */
void
fdio_test_6(unsigned int tid)
{
    char teststr[16];
    snprintf(teststr, 15, "%d\n", (int)tid);

    systx_begin

        /* Open file */

        int fildes = open_tx("/tmp/fdio.test",
                             O_WRONLY | O_CREAT,
                             S_IRWXU | S_IRWXG | S_IRWXO);

        if (fildes < 0) {
            perror("open");
            abort_tx();
        }

        /* Do I/O */

        if (lseek_tx(fildes, 2, SEEK_CUR) == (off_t)-1) {
            perror("lseek");
            abort_tx();
        }

        if (write_tx(fildes, g_test_str, strlen(g_test_str)) < 0) {
            perror("write");
            abort_tx();
        }

        /* Write TID to the EOF */

        if (lseek_tx(fildes, 0, SEEK_END) == (off_t)-1) {
            perror("lseek");
            abort_tx();
        }

        if (write_tx(fildes, teststr, strlen_tm(teststr)) < 0) {
            perror("write");
            abort_tx();
        }

        /* Close */

        if (close_tx(fildes) < 0) {
            perror("close");
            abort_tx();
        }

    systx_commit
    systx_end
}

/**
 * Write string, read-back and write again; should output 'world world!\n<tid>'.
 */
void
fdio_test_7(unsigned int tid)
{
    char teststr[16];
    int res = snprintf(teststr, sizeof(teststr), "%d\n", (int)tid);
    if (res < 0) {
        perror("snprintf");
        abort();
    }

    systx_begin

        /* Open */

        int fildes = open_tx("/tmp/fdio.test",
                             O_RDWR | O_CREAT,
                             S_IRWXU | S_IRWXG | S_IRWXO);

        if (fildes < 0) {
            perror("open");
            abort_tx();
        }

        /* Do I/O */

        if (write_tx(fildes, g_test_str, strlen_tm(g_test_str)) < 0) {
            perror("write");
            abort_tx();
        }

        if (lseek_tx(fildes, 6, SEEK_SET) == (off_t)-1) {
            perror("lseek");
            abort_tx();
        }

        char buf[5];
        memset_tm(buf, 0, sizeof(buf)); /* Workaround valgrind */

        if (read_tx(fildes, buf, sizeof(buf)) < 0) {
            perror("read");
            abort_tx();
        }

        if (lseek_tx(fildes, 0, SEEK_SET) == (off_t)-1) {
            perror("lseek");
            abort_tx();
        }

        if (write_tx(fildes, buf, sizeof(buf)) < 0) {
            perror("write");
            abort_tx();
        }

        /* Write TID to the EOF */

        if (lseek_tx(fildes, 0, SEEK_END) == (off_t)-1) {
            perror("lseek");
            abort_tx();
        }

        if (write_tx(fildes, teststr, strlen_tm(teststr)) < 0) {
            perror("write");
            abort_tx();
        }

        /* Close */

        if (close_tx(fildes) < 0) {
            perror("close");
            abort_tx();
        }

    systx_commit
    systx_end
}

/**
 * Read through pipe to test buffered reads.
 *
 * \todo Investigate if non-blocking reads should be supported and
 * return 0 in case of an empty pipe.
 */
void
fdio_test_8(unsigned int tid)
{
    char filename[32];
    int res = snprintf(filename, sizeof(filename), "/tmp/fdio-%u.test", tid);
    if (res < 0) {
        perror("snprintf");
        abort();
    }

    int pfd[2];
    res = pipe(pfd);
    if (res < 0) {
        perror("pipe");
        abort();
    }

    /* Set pipe's read end to non-blocking mode */
    {
        int fl = TEMP_FAILURE_RETRY(fcntl(pfd[0], F_GETFL));
        if (fl < 0) {
            perror("fcntl");
            abort();
        }
        if (TEMP_FAILURE_RETRY(fcntl(pfd[0], F_SETFL, fl | O_NONBLOCK)) < 0) {
            perror("fcntl");
            abort();
        }
    }

    /* Fill pipe */

    for (int i = 0; i < 1000; ++i) {

        char str[128];
        ssize_t res = snprintf(str, sizeof(str), "%d %s", i, g_test_str);
        if (res < 0) {
            perror("snprintf");
            abort();
        }
        size_t len = res;

        res = TEMP_FAILURE_RETRY(write(pfd[1], str, len));
        if (res < 0) {
            perror("write");
            abort();
        }
    }

    systx_begin

        /* Open file */

        int fildes = open_tx(filename,
                             O_WRONLY | O_CREAT | O_TRUNC,
                             S_IRWXU | S_IRWXG | S_IRWXO);
        if (fildes < 0) {
            perror("open");
            abort_tx();
        }

        size_t rlen;

        while (true) {

            char rbuf[1024];
            memset_tm(rbuf, 0, sizeof(rbuf)); /* Work around valgrind */

            /* Read from pipe */
            ssize_t res = read_tx(pfd[0], rbuf, sizeof(rbuf));
            if (res < 0) {
                if (errno == EAGAIN) { /* Pipe empty; leave loop */
                    break;
                } else {
                    perror("read");
                    abort_tx();
                }
            }
            rlen = res;

            /* Write to file */
            res = write_tx(fildes, rbuf, rlen);
            if (res < 0) {
                perror("write");
                abort_tx();
            }
        }

        int res = close_tx(fildes);
        if (res < 0) {
            perror("close");
            abort_tx();
        }

        /* Close pipe */

        for (size_t i = 0; i < arraylen(pfd); ++i) {
            int res = close_tx(pfd[i]);
            if (res < 0) {
                perror("close");
                abort_tx();
            }
        }

    systx_commit
    systx_end
}

/**
 * Write some lines at end of file. Should result in an integer record
 * for each thread. The order of records can vary.
 */
void
fdio_test_9(unsigned int tid)
{
    static int fildes = -1;
    static pthread_mutex_t fd_lock = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&fd_lock);

    if (fildes < 0) {
        fildes = TEMP_FAILURE_RETRY(open("/tmp/fdio.test",
                                         O_WRONLY | O_CREAT,
                                         S_IRWXU | S_IRWXG | S_IRWXO));

        if (fildes < 0) {
            perror("open");
            abort();
        }
    }

    pthread_mutex_unlock(&fd_lock);

    char str[100][256];

    for (char (*s)[256] = str; s < str + arraylen(str); ++s) {
        if (sprintf(*s, "%d line %d\n", tid, (int)(s - str)) < 0) {
            perror("snprintf");
            abort();
        }
    }

    systx_begin

        if (lseek_tx(fildes, 0, SEEK_END) == (off_t)-1) {
            perror("lseek");
            abort_tx();
        }

        for (const char (*s)[256] = str; s < str + arraylen(str); ++s) {
            if (write_tx(fildes, *s, strlen_tm(*s)) < 0) {
                perror("write");
                abort_tx();
            }
        }

    systx_commit
    systx_end

    /*if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
        perror("close");
        abort();
    }*/
}

/**
 * Write string, read-back and write again; should output 'Hello Hello!'.
 *
 * \todo Did not return.
 */
void
fdio_test_10(unsigned int tid)
{
    static int fildes = -1;
    static pthread_mutex_t fd_lock = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&fd_lock);

    if (fildes < 0) {
        fildes = TEMP_FAILURE_RETRY(open("/tmp/fdio.test",
                                         O_RDWR | O_CREAT,
                                         S_IRWXU | S_IRWXG | S_IRWXO));

        if (fildes < 0) {
            perror("open");
            abort();
        }
    }

    pthread_mutex_unlock(&fd_lock);

    systx_begin

        if (pwrite_tx(fildes, g_test_str, strlen_tm(g_test_str), 0) < 0) {
            perror("pwrite");
            abort_tx();
        }

        char rbuf[5];
        memset_tm(rbuf, 0, sizeof(rbuf)); /* Workaround valgrind */

        if (pread_tx(fildes, rbuf, sizeof(rbuf), 0) < 0) {
            perror("pread");
            abort_tx();
        }

        if (pwrite_tx(fildes, rbuf, sizeof(rbuf), 6) < 0) {
            perror("pwrite");
            abort_tx();
        }

    systx_commit
    systx_end
}

/**
 * Write string at position 2 using lseek; should output '  Hello world!'.
 */
void
fdio_test_11(unsigned int tid)
{
    char teststr[16];

    if (snprintf(teststr, 15, "%d\n", (int)tid) < 0) {
        perror("snprintf");
        abort();
    }

    static int fildes = -1;
    static pthread_mutex_t fd_lock = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&fd_lock);

    if (fildes < 0) {
        fildes = TEMP_FAILURE_RETRY(open("/tmp/fdio.test",
                                         O_WRONLY | O_CREAT,
                                         S_IRWXU | S_IRWXG | S_IRWXO));

        if (fildes < 0) {
            perror("open");
            abort();
        }
    }

    pthread_mutex_unlock(&fd_lock);

    systx_begin

        if (lseek_tx(fildes, 0, SEEK_SET) == (off_t)-1) {
            perror("lseek");
            abort_tx();
        }

        /* Do I/O */

        if (lseek_tx(fildes, 2, SEEK_CUR) == (off_t)-1) {
            perror("lseek");
            abort_tx();
        }

        if (write_tx(fildes, g_test_str, strlen_tm(g_test_str)) < 0) {
            perror("write");
            abort_tx();
        }

        /* Write TID to the EOF */

        if (lseek_tx(fildes, 0, SEEK_END) == (off_t)-1) {
            perror("lseek");
            abort_tx();
        }

        if (write_tx(fildes, teststr, strlen_tx(teststr)) < 0) {
            perror("write");
            abort_tx();
        }

    systx_commit
    systx_end
}

/**
 * Write string, read-back and write again; should output 'world world!'.
 */
void
fdio_test_12(unsigned int tid)
{
    char teststr[16];

    if (snprintf(teststr, 15, "%d\n", (int)tid) < 0) {
        perror("snprintf");
        abort();
    }

    static int fildes = -1;
    static pthread_mutex_t fd_lock = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&fd_lock);

    if (fildes < 0) {
        fildes = TEMP_FAILURE_RETRY(open("/tmp/fdio.test",
                                         O_RDWR | O_CREAT,
                                         S_IRWXU | S_IRWXG | S_IRWXO));

        if (fildes < 0) {
            perror("open");
            abort();
        }
    }

    pthread_mutex_unlock(&fd_lock);

    systx_begin

        /* Do I/O */

        if (lseek_tx(fildes, 0, SEEK_SET) == (off_t)-1) {
            perror("lseek");
            abort_tx();
        }

        if (write_tx(fildes, g_test_str, strlen_tm(g_test_str)) < 0) {
            perror("write");
            abort_tx();
        }

        if (lseek_tx(fildes, 6, SEEK_SET) == (off_t)-1) {
            perror("lseek");
            abort_tx();
        }

        char buf[5];
        memset_tm(buf, 0, sizeof(buf)); /* Workaround valgrind */

        if (read_tx(fildes, buf, sizeof(buf)) < 0) {
            perror("read");
            abort_tx();
        }

        if (lseek_tx(fildes, 0, SEEK_SET) == (off_t)-1) {
            perror("lseek");
            abort_tx();
        }

        if (write_tx(fildes, buf, sizeof(buf)) < 0) {
            perror("write");
            abort_tx();
        }

        /* Write TID to the EOF */

        if (lseek_tx(fildes, 0, SEEK_END) == (off_t)-1) {
            perror("lseek");
            abort_tx();
        }

        if (write_tx(fildes, teststr, strlen_tx(teststr)) < 0) {
            perror("write");
            abort_tx();
        }

    systx_commit
    systx_end
}

/**
 * Write to stdout to test unbuffered writes.
 */
void
fdio_test_13(unsigned int tid)
{
    int i;

    char str[20][128];

    for (i = 0; i < 20; ++i) {
        if (snprintf(str[i], sizeof(str[i]), "%u %d %s", tid, i, g_test_str) < 0) {
            perror("snprintf");
            abort();
        }
    }

    systx_begin

    for (i = 0; i < 20; ++i) {
        if (write_tx(STDOUT_FILENO, str[i], strlen_tx(str[i])) < 0) {
            perror("write");
            abort_tx();
        }
    }

    systx_commit
    systx_end
}

/**
 * Write some lines at end of file. Should result in an integer record
 * for each thread. The order of records can vary. This tests the
 * optimization for non-depending accesses.
 */
void
fdio_test_14(unsigned int tid)
{
    static volatile int fildes = -1;
    static pthread_mutex_t fd_lock = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&fd_lock);

    if (fildes < 0) {
        fildes = TEMP_FAILURE_RETRY(open("/tmp/fdio.test",
                                         O_WRONLY | O_CREAT,
                                         S_IRWXU | S_IRWXG | S_IRWXO));

        if (fildes < 0) {
            perror("open");
            abort();
        }
    }

    pthread_mutex_unlock(&fd_lock);

    char str[100][256];
    char (*s)[256];

    for (s = str; s < str+sizeof(str)/sizeof(str[0]); ++s) {
        int res = snprintf(*s, sizeof(*s), "%u line %lu\n", tid,
                           (unsigned long)(s - str));
        if (res < 0) {
            perror("snprintf");
            abort();
        }
    }

    systx_begin

        /*off_t pos = lseek_tx(fildes, 0, SEEK_END);

        if (pos == (off_t)-1) {
            perror("lseek");
            abort_tx();
        }*/

        for (s = str; s < str+sizeof(str)/sizeof(str[0]); ++s) {
            if (write_tx(fildes, *s, strlen(*s)) < 0) {
                perror("write");
                abort_tx();
            }
        }

    systx_commit
    systx_end

    /*if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
        perror("close");
        abort();
    }*/
}

/**
 * Open, dup and close a file descriptor.
 */
void
fdio_test_15(unsigned int tid)
{
    int fildes = TEMP_FAILURE_RETRY(open("/tmp/fdio.test",
                                         O_WRONLY | O_CREAT,
                                         S_IRWXU | S_IRWXG | S_IRWXO));

    if (fildes < 0) {
        perror("open");
        abort();
    }

    systx_begin

        int fildes2 = dup_tx(fildes);

        if (fildes2 < 0) {
            perror("dup");
            abort_tx();
        }

        delay_transaction(tid);

        if (close_tx(fildes2) < 0) {
            perror("close");
            abort_tx();
        }

    systx_commit
    systx_end

    if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
        perror("close");
        abort();
    }
}

/**
 * Open, dup and close a file descriptor.
 */
void
fdio_test_16(unsigned int tid)
{
    extern enum systx_libc_cc_mode g_cc_mode;
    systx_libc_set_file_type_cc_mode(SYSTX_LIBC_FILE_TYPE_REGULAR, g_cc_mode);

    systx_begin

        int fildes = open_tx("/tmp/fdio.test",
                             O_WRONLY | O_CREAT,
                             S_IRWXU | S_IRWXG | S_IRWXO);

        if (fildes < 0) {
            perror("open");
            abort_tx();
        }

        int fildes2 = dup_tx(fildes);

        if (fildes2 < 0) {
            perror("dup");
            abort_tx();
        }

        delay_transaction(tid);

        if (close_tx(fildes2) < 0) {
            perror("close");
            abort_tx();
        }

        if (close_tx(fildes) < 0) {
            perror("close");
            abort_tx();
        }

    systx_commit
    systx_end
}

/**
 * Open, dup, write, and close a file descriptor.
 */
void
fdio_test_17(unsigned int tid)
{
    extern enum systx_libc_cc_mode g_cc_mode;
    systx_libc_set_file_type_cc_mode(SYSTX_LIBC_FILE_TYPE_REGULAR, g_cc_mode);

    char str[2][128];

    for (char (*s)[128] = str; s < str + arraylen(str); ++s) {
        int res = snprintf(*s, sizeof(str[0]), "%u %lu %s", tid,
                           (unsigned long)(s - str), g_test_str);
        if (res < 0) {
            perror("snprintf");
            abort();
        }
    }

    systx_begin

        int fildes = open_tx("/tmp/fdio.test",
                             O_WRONLY | O_CREAT,
                             S_IRWXU | S_IRWXG | S_IRWXO);

        if (fildes < 0) {
            perror("open");
            abort_tx();
        }

        int fildes2 = dup_tx(fildes);

        if (fildes2 < 0) {
            perror("dup");
            abort_tx();
        }

        if (lseek_tx(fildes, 0, SEEK_END) == (off_t)-1) {
            perror("lseek1");
            abort_tx();
        }

        if (lseek_tx(fildes2, 0, SEEK_END) == (off_t)-1) {
            perror("lseek2");
            abort_tx();
        }

        if (write_tx(fildes, str[0], strlen_tx(str[0])) < 0) {
            perror("write1");
            abort_tx();
        }

        if (write_tx(fildes2, str[1], strlen_tx(str[1])) < 0) {
            perror("write2");
            abort_tx();
        }

        if (close_tx(fildes2) < 0) {
            perror("close2");
            abort_tx();
        }

        if (close_tx(fildes) < 0) {
            perror("close1");
            abort_tx();
        }

    systx_commit
    systx_end
}

/**
 * Open, dup, write, and close a file descriptor.
 */
void
fdio_test_18(unsigned int tid)
{
    extern enum systx_libc_cc_mode g_cc_mode;
    systx_libc_set_file_type_cc_mode(SYSTX_LIBC_FILE_TYPE_REGULAR, g_cc_mode);

    static volatile int fildes = -1;
    static pthread_mutex_t fd_lock = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&fd_lock);

    if (fildes < 0) {
        fildes = TEMP_FAILURE_RETRY(open("/tmp/fdio.test",
                                         O_WRONLY | O_CREAT,
                                         S_IRWXU | S_IRWXG | S_IRWXO));

        if (fildes < 0) {
            perror("open");
            abort();
        }
    }

    pthread_mutex_unlock(&fd_lock);

    char str[2][128];

    for (char (*s)[128] = str; s < str + arraylen(str); ++s) {
        int res = snprintf(*s, sizeof(str[0]), "%u %lu %s", tid,
                           (unsigned long)(s - str), g_test_str);
        if (res < 0) {
            perror("snprintf");
            abort();
        }
    }

    systx_begin

        /*int fildes = open_tx("/tmp/fdio.test",
                               O_WRONLY | O_CREAT,
                               S_IRWXU | S_IRWXG | S_IRWXO));

        if (fildes < 0) {
            perror("open");
            abort_tx();
        }*/

        int fildes2 = dup_tx(fildes);

        if (fildes2 < 0) {
            perror("dup");
            abort_tx();
        }

        if (lseek_tx(fildes, 0, SEEK_END) == (off_t)-1) {
            perror("lseek1");
            abort_tx();
        }

        if (lseek_tx(fildes2, 0, SEEK_END) == (off_t)-1) {
            perror("lseek2");
            abort_tx();
        }

        if (write_tx(fildes, str[0], strlen_tx(str[0])) < 0) {
            perror("write1");
            abort_tx();
        }

        if (write_tx(fildes2, str[1], strlen_tx(str[1])) < 0) {
            perror("write2");
            abort_tx();
        }

        if (close_tx(fildes2) < 0) {
            perror("close");
            abort_tx();
        }

    systx_commit
    systx_end

    /*if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
        perror("close");
        abort();
    }*/
}


/**
 * Write to stdout to test unbuffered writes.
 */
void
fdio_test_19(unsigned int tid)
{
    static int pfd[2] = {-1, -1};
    static pthread_mutex_t fd_lock = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&fd_lock);

    if (pfd[0] < 0) {

        if (pipe(pfd) < 0) {
            perror("pipe");
            abort();
        }

        /* Fill pipe */

        for (int i = 0; i < 10; ++i) {

            char str[128];

            int res = snprintf(str, sizeof(str), "%u %d %s", tid, i,
                               g_test_str);
            if (res < 0) {
                perror("snprintf");
                abort();
            }

            if (TEMP_FAILURE_RETRY(write(pfd[1], str, strlen(str))) < 0) {
                perror("write");
                abort();
            }
        }

        /* Set pipe's read end to non-blocking mode */

        int fl = TEMP_FAILURE_RETRY(fcntl(pfd[0], F_GETFL));
        if (fl < 0) {
            perror("fcntl");
            abort();
        }

        if (TEMP_FAILURE_RETRY(fcntl(pfd[0], F_SETFL, fl|O_NONBLOCK)) < 0) {
            perror("fcntl");
            abort();
        }
    }

    pthread_mutex_unlock(&fd_lock);

    size_t rlen;

    do {
        systx_begin

            char rbuf[10];
            memset_tm(rbuf, 0, sizeof(rbuf)); /* Work around valgrind */

            /* Read from pipe */
            ssize_t res = read_tx(pfd[0], rbuf, sizeof(rbuf));
            if (res < 0) {
                if (errno == EAGAIN) { /* Pipe empty */
                    res = 0;
                } else {
                    perror("read");
                    abort_tx();
                }
            }
            size_t tx_rlen = res;

            /* Write to stdout */
            res = write_tx(STDOUT_FILENO, rbuf, tx_rlen);
            if (res < 0) {
                perror("write");
                abort_tx();
            }

            store_size_t_tx(&rlen, tx_rlen);

        systx_commit
        systx_end

    } while (rlen);

    /* Close pipe */
    /*for (i = 0; i < sizeof(pfd)/sizeof(pfd[0]); ++i) {
        if (TEMP_FAILURE_RETRY(close(pfd[i])) < 0) {
            perror("close");
            abort();
        }
    }*/
}

static int g_fildes = -1;

void
fdio_test_20_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    extern enum systx_libc_cc_mode g_cc_mode;
    systx_libc_set_file_type_cc_mode(SYSTX_LIBC_FILE_TYPE_REGULAR, g_cc_mode);

    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile.bin",
                                       O_RDWR | O_CREAT,
                                       S_IRWXU | S_IRWXG | S_IRWXO));
    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
fdio_test_20_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    int res = fsync(g_fildes);
    if (res < 0) {
        perror("fsync");
        abort();
    }

    res = TEMP_FAILURE_RETRY(close(g_fildes));
    if (res < 0) {
        perror("close");
        abort();
    }
}

/**
 * Write string, read-back and write again; should output 'Hello Hello!'.
 */
void
fdio_test_20(unsigned int tid)
{
    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    off_t offset = rand_r(&t_seed) % (1024 * 1024);

    systx_begin

        off_t tx_offset = load_off_t_tx(&offset);

        unsigned char buf[24];
        ssize_t res = pread_tx(g_fildes, buf, sizeof(buf), tx_offset);
        if (res < 0) {
            perror("pread");
            abort_tx();
        }
        size_t count = res;

        if (pwrite_tx(g_fildes, buf, count, offset) < 0) {
            perror("pwrite");
            abort_tx();
        }

    systx_commit
    systx_end
}

/*
 * Random read/write
 */

int
tx_random_rw_pre(const char* filename)
{
    extern enum systx_libc_cc_mode g_cc_mode;
    systx_libc_set_file_type_cc_mode(SYSTX_LIBC_FILE_TYPE_REGULAR, g_cc_mode);

    int res = TEMP_FAILURE_RETRY(open(filename, O_RDWR,
                                      S_IRWXU | S_IRWXG | S_IRWXO));
    if (res < 0) {
        perror("open");
        abort();
    }

    return res;
}

void
tx_random_rw(int fildes, unsigned int* seed, off_t size,
             unsigned long ncycles, unsigned long nreads)
{
    systx_begin

        off_t tx_size = load_off_t_tx(&size);
        unsigned long tx_ncycles = load_ulong_tx(&ncycles);
        unsigned long tx_nreads = load_ulong_tx(&nreads);

        for (unsigned long i = 0; i < tx_ncycles; ++i) {

            unsigned char buf[24];
            off_t offset;
            size_t count;

            for (unsigned long j = 0; j < tx_nreads; ++j) {

                offset = rand_r_tx(seed) % tx_size;

                ssize_t res = pread_tx(fildes, buf, sizeof(buf), offset);
                if (res < 0) {
                    perror("pread");
                    abort();
                }
                count = res;
            }

            ssize_t res = pwrite_tx(fildes, buf, count, offset);
            if (res < 0) {
                perror("pwrite");
                abort();
            }
        }

    systx_commit
    systx_end
}

int
locked_random_rw_pre(const char* filename)
{
    int res = TEMP_FAILURE_RETRY(open(filename, O_RDWR,
                                      S_IRWXU | S_IRWXG | S_IRWXO));
    if (res < 0) {
        perror("open");
        abort();
    }

    return res;
}

void
locked_random_rw(pthread_mutex_t* lock, int fildes, unsigned int* seed,
                 off_t size, unsigned long ncycles, unsigned long nreads)
{
    pthread_mutex_lock(lock);

    for (unsigned long i = 0; i < ncycles; ++i) {

        unsigned char buf[24];
        off_t offset;
        size_t count;

        for (unsigned long j = 0; j < nreads; ++j) {

            offset = rand_r(seed) % size;

            ssize_t res = TEMP_FAILURE_RETRY(pread(g_fildes,
                                             buf,
                                             sizeof(buf),
                                             offset));
            if (res < 0) {
                perror("pread");
                abort();
            }
            count = res;
        }

        ssize_t res = TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, count, offset));
        if (res < 0) {
            perror("pwrite");
            abort();
        }
    }

    pthread_mutex_unlock(lock);
}

static void
random_rw_post(int fildes)
{
    int res = fsync(fildes);
    if (res < 0) {
        perror("fsync");
        abort();
    }

    res = TEMP_FAILURE_RETRY(close(fildes));
    if (res < 0) {
        perror("close");
        abort();
    }
}

/*
 * Random read/write, ratio 1:1
 */

void
fdio_test_21_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = tx_random_rw_pre("./testdir/testfile.bin");
}

void
fdio_test_21_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    random_rw_post(g_fildes);
}

void
fdio_test_21(unsigned int tid)
{
    extern size_t g_txcycles;

    static __thread unsigned int t_seed = 0; /* thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    tx_random_rw(g_fildes, &t_seed, 1024 * 1024, g_txcycles, 1);
}

void
fdio_test_22_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = locked_random_rw_pre("./testdir/testfile.bin");
}

void
fdio_test_22_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    random_rw_post(g_fildes);
}

void
fdio_test_22(unsigned int tid)
{
    extern size_t g_txcycles;

    static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;
    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    locked_random_rw(&s_lock, g_fildes, &t_seed, 1024 * 1024, g_txcycles, 1);
}

/*
 * Random read/write, ratio 2:1
 */

void
fdio_test_23_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = tx_random_rw_pre("./testdir/testfile.bin");
}

void
fdio_test_23_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    random_rw_post(g_fildes);
}

void
fdio_test_23(unsigned int tid)
{
    extern size_t g_txcycles;

    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    tx_random_rw(g_fildes, &t_seed, 1024 * 1024, g_txcycles, 2);
}

void
fdio_test_24_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = locked_random_rw_pre("./testdir/testfile.bin");
}

void
fdio_test_24_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    random_rw_post(g_fildes);
}

void
fdio_test_24(unsigned int tid)
{
    extern size_t g_txcycles;

    static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;
    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    locked_random_rw(&s_lock, g_fildes, &t_seed, 1024 * 1024, g_txcycles, 2);
}

/*
 * Random read/write, ratio 4:1
 */

void
fdio_test_25_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = tx_random_rw_pre("./testdir/testfile.bin");
}

void
fdio_test_25_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    random_rw_post(g_fildes);
}

void
fdio_test_25(unsigned int tid)
{
    extern size_t g_txcycles;

    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    tx_random_rw(g_fildes, &t_seed, 1024 * 1024, g_txcycles, 4);
}

void
fdio_test_26_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = locked_random_rw_pre("./testdir/testfile.bin");
}

void
fdio_test_26_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    random_rw_post(g_fildes);
}

void
fdio_test_26(unsigned int tid)
{
    extern size_t g_txcycles;

    static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    locked_random_rw(&g_lock, g_fildes, &t_seed, 1024 * 1024, g_txcycles, 4);
}

/*
 * Random read/write, ratio 8:1
 */

void
fdio_test_27_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = tx_random_rw_pre("./testdir/testfile.bin");
}

void
fdio_test_27_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    random_rw_post(g_fildes);
}

void
fdio_test_27(unsigned int tid)
{
    extern size_t g_txcycles;

    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    tx_random_rw(g_fildes, &t_seed, 1024 * 1024, g_txcycles, 8);
}

void
fdio_test_28_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = locked_random_rw_pre("./testdir/testfile.bin");
}

void
fdio_test_28_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    random_rw_post(g_fildes);
}

void
fdio_test_28(unsigned int tid)
{
    extern size_t g_txcycles;

    static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;
    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    locked_random_rw(&s_lock, g_fildes, &t_seed, 1024 * 1024, g_txcycles, 8);
}

/*
 * Random reads
 */

static int
tx_random_read_pre(const char* filename)
{
    extern enum systx_libc_cc_mode g_cc_mode;
    systx_libc_set_file_type_cc_mode(SYSTX_LIBC_FILE_TYPE_REGULAR, g_cc_mode);

    int res = TEMP_FAILURE_RETRY(open(filename,
                                      O_RDONLY | O_CREAT,
                                      S_IRWXU | S_IRWXG | S_IRWXO));
    if (res < 0) {
        perror("open");
        abort();
    }
    return res;
}

static void
tx_random_read(int fildes, unsigned int* seed, off_t size,
               unsigned long ncycles)
{
    systx_begin

        size_t tx_cycles = load_ulong_tx(&ncycles);

        for (size_t i = 0; i < tx_cycles; ++i) {

            off_t offset = rand_r_tx(seed) % size;

            unsigned char buf[24];
            ssize_t res = pread_tx(fildes, buf, sizeof(buf), offset);
            if (res < 0) {
                perror("pread");
                abort_tx();
            }
        }

    systx_commit
    systx_end
}

static int
locked_random_read_pre(const char* filename)
{
    int res = TEMP_FAILURE_RETRY(open(filename,
                                      O_RDONLY | O_CREAT,
                                      S_IRWXU | S_IRWXG | S_IRWXO));
    if (res < 0) {
        perror("open");
        abort();
    }
    return res;
}

static void
locked_random_read(pthread_mutex_t* lock, int fildes, unsigned int* seed,
                   off_t size, unsigned long ncycles)
{
    pthread_mutex_lock(lock);

    for (unsigned long i = 0; i < ncycles; ++i) {

        off_t offset = rand_r(seed) % size;

        unsigned char buf[24];
        ssize_t res = TEMP_FAILURE_RETRY(pread(fildes,
                                               buf,
                                               sizeof(buf),
                                               offset));
        if (res < 0) {
            perror("pread");
            abort();
        }
    }

    pthread_mutex_unlock(lock);
}

static void
random_read_post(int fildes)
{
    int res = fsync(fildes);
    if (res < 0) {
        perror("fsync");
        abort();
    }

    res = TEMP_FAILURE_RETRY(close(fildes));
    if (res < 0) {
        perror("close");
        abort();
    }
}

void
fdio_test_29_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = tx_random_read_pre("./testdir/testfile.bin");
}

void
fdio_test_29_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    random_read_post(g_fildes);
}

void
fdio_test_29(unsigned int tid)
{
    extern size_t g_txcycles;

    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    tx_random_read(g_fildes, &t_seed, 1024 * 1024, g_txcycles);
}

void
fdio_test_30_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = locked_random_read_pre("./testdir/testfile.bin");
}

void
fdio_test_30_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    random_read_post(g_fildes);
}

void
fdio_test_30(unsigned int tid)
{
    extern size_t g_txcycles;

    static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;
    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    locked_random_read(&s_lock, g_fildes, &t_seed, 1024 * 1024, g_txcycles);
}

/*
 * Random writes
 */

static int
tx_random_write_pre(const char* filename)
{
    extern enum systx_libc_cc_mode g_cc_mode;
    systx_libc_set_file_type_cc_mode(SYSTX_LIBC_FILE_TYPE_REGULAR, g_cc_mode);

    int res = TEMP_FAILURE_RETRY(open(filename,
                                      O_WRONLY | O_CREAT,
                                      S_IRWXU | S_IRWXG | S_IRWXO));
    if (res < 0) {
        perror("open");
        abort();
    }
    return res;
}

static void
tx_random_write(int fildes, unsigned int* seed, off_t size, unsigned long ncycles)
{
    unsigned char buf[24];
    memset(buf, 0, sizeof(buf));

    systx_begin

        unsigned long tx_cycles = load_ulong_tx(&ncycles);

        for (unsigned long i = 0; i < tx_cycles; ++i) {

            off_t offset = rand_r_tx(seed) % size;

            ssize_t res = pwrite_tx(fildes, buf, sizeof(buf), offset);
            if (res < 0) {
                perror("pwrite");
                abort_tx();
            }
        }

    systx_commit
    systx_end
}

static int
locked_random_write_pre(const char* filename)
{
    int res = TEMP_FAILURE_RETRY(open(filename,
                                      O_WRONLY | O_CREAT,
                                      S_IRWXU | S_IRWXG | S_IRWXO));
    if (res < 0) {
        perror("open");
        abort();
    }
    return res;
}

static void
locked_random_write(pthread_mutex_t* lock, int fildes, unsigned int* seed,
                    off_t size, unsigned long ncycles)
{
    unsigned char buf[24];
    memset(buf, 0, sizeof(buf));

    pthread_mutex_lock(lock);

    for (unsigned long i = 0; i < ncycles; ++i) {

        off_t offset = rand_r(seed) % size;

        ssize_t res = TEMP_FAILURE_RETRY(pwrite(fildes, buf, sizeof(buf),
                                                offset));
        if (res < 0) {
            perror("pwrite");
            abort();
        }
    }

    pthread_mutex_unlock(lock);
}


static void
random_write_post(int fildes)
{
    int res = fsync(fildes);
    if (res < 0) {
        perror("fsync");
        abort();
    }

    res = TEMP_FAILURE_RETRY(close(fildes));
    if (res < 0) {
        perror("close");
        abort();
    }
}

void
fdio_test_31_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = tx_random_write_pre("./testdir/testfile2.bin");
}

void
fdio_test_31(unsigned int tid)
{
    extern size_t g_txcycles;

    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    tx_random_read(g_fildes, &t_seed, 1024 * 1024, g_txcycles);
}

void
fdio_test_31_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    random_read_post(g_fildes);
}

void
fdio_test_32_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = locked_random_read_pre("./testdir/testfile2.bin");
}

void
fdio_test_32(unsigned int tid)
{
    extern size_t g_txcycles;

    static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;
    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    locked_random_read(&s_lock, g_fildes, &t_seed, 1024 * 1024, g_txcycles);
}

void
fdio_test_32_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    random_read_post(g_fildes);
}

/*
 * Sequentiell reads
 */

static int
tx_seq_read_pre(const char* filename)
{
    extern enum systx_libc_cc_mode g_cc_mode;
    systx_libc_set_file_type_cc_mode(SYSTX_LIBC_FILE_TYPE_REGULAR, g_cc_mode);

    int res = TEMP_FAILURE_RETRY(open(filename,
                                      O_RDONLY | O_CREAT,
                                      S_IRWXU | S_IRWXG | S_IRWXO));
    if (res < 0) {
        perror("open");
        abort();
    }
    return res;
}

static void
tx_seq_read(int fildes, unsigned int* seed, off_t size, unsigned long ncycles)
{
    off_t offset = rand_r(seed) % size;

    systx_begin

        off_t pos = load_off_t_tx(&offset);
        unsigned long cycles = load_ulong_tx(&ncycles);

        for (unsigned long i = 0; i < cycles; ++i) {

            unsigned char buf[24];
            ssize_t res = pread_tx(fildes, buf, sizeof(buf), pos);
            if (res < 0) {
                perror("pread");
                abort_tx();
            }

            pos += res;
        }

    systx_commit
    systx_end
}

static int
locked_seq_read_pre(const char* filename)
{
    int res = TEMP_FAILURE_RETRY(open(filename,
                                      O_RDONLY | O_CREAT,
                                      S_IRWXU | S_IRWXG | S_IRWXO));
    if (res < 0) {
        perror("open");
        abort();
    }
    return res;
}

static void
locked_seq_read(pthread_mutex_t* lock, int fildes, unsigned int* seed,
                off_t size, unsigned long ncycles)
{
    off_t offset = rand_r(seed) % size;

    pthread_mutex_lock(lock);

    for (unsigned long i = 0; i < ncycles; ++i) {

        unsigned char buf[24];
        ssize_t res = TEMP_FAILURE_RETRY(pread(fildes, buf, sizeof(buf),
                                               offset));
        if (res < 0) {
            perror("pread");
            abort();
        }

        offset += res;
    }

    pthread_mutex_unlock(lock);
}

static void
seq_read_post(int fildes)
{
    int res = fsync(fildes);
    if (res < 0) {
        perror("fsync");
        abort();
    }

    res = TEMP_FAILURE_RETRY(close(fildes));
    if (res < 0) {
        perror("close");
        abort();
    }
}

void
fdio_test_33_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = tx_seq_read_pre("./testdir/testfile.bin");
}

void
fdio_test_33(unsigned int tid)
{
    extern size_t g_txcycles;

    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    tx_seq_read(g_fildes, &t_seed, 1024 * 1024, g_txcycles);
}

void
fdio_test_33_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    seq_read_post(g_fildes);
}

void
fdio_test_34_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = locked_seq_read_pre("./testdir/testfile.bin");
}

void
fdio_test_34(unsigned int tid)
{
    extern size_t g_txcycles;

    static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;
    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    locked_seq_read(&s_lock, g_fildes, &t_seed, 1024 * 1024, g_txcycles);
}

void
fdio_test_34_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    seq_read_post(g_fildes);
}

/*
 * Sequentiell writes
 */

static int
tx_seq_write_pre(const char* filename)
{
    extern enum systx_libc_cc_mode g_cc_mode;
    systx_libc_set_file_type_cc_mode(SYSTX_LIBC_FILE_TYPE_REGULAR, g_cc_mode);

    int res = TEMP_FAILURE_RETRY(open(filename,
                                      O_WRONLY | O_CREAT,
                                      S_IRWXU | S_IRWXG | S_IRWXO));
    if (res < 0) {
        perror("open");
        abort();
    }
    return res;
}

static void
tx_seq_write(int fildes, unsigned int* seed, off_t size, unsigned long ncycles)
{
    unsigned char buf[24];
    memset(buf, 0, sizeof(buf));

    off_t offset = rand_r(seed) % size;

    systx_begin

        off_t pos = load_off_t_tx(&offset);
        unsigned long tx_ncycles = load_ulong_tx(&ncycles);

        for (unsigned long i = 0; i < tx_ncycles; ++i) {

            ssize_t res = pwrite_tx(fildes, buf, sizeof(buf), pos);
            if (res < 0) {
                perror("pwrite");
                abort_tx();
            }

            pos += res;
        }

    systx_commit
    systx_end
}

static int
locked_seq_write_pre(const char* filename)
{
    int res = TEMP_FAILURE_RETRY(open(filename,
                                      O_WRONLY | O_CREAT,
                                      S_IRWXU | S_IRWXG | S_IRWXO));
    if (res < 0) {
        perror("open");
        abort();
    }
    return res;
}

static void
locked_seq_write(pthread_mutex_t* lock, int fildes, unsigned int* seed,
                 off_t size, unsigned long ncycles)
{
    unsigned char buf[24];
    memset(buf, 0, sizeof(buf));

    off_t offset = rand_r(seed) % size;

    pthread_mutex_lock(lock);

    for (unsigned long i = 0; i < ncycles; ++i) {

        ssize_t res = TEMP_FAILURE_RETRY(pwrite(fildes, buf, sizeof(buf),
                                                offset));
        if (res < 0) {
            perror("pwrite");
            abort();
        }

        offset += res;
    }

    pthread_mutex_unlock(lock);
}

static void
seq_write_post(int fildes)
{
    int res = fsync(fildes);
    if (res < 0) {
        perror("fsync");
        abort();
    }

    res = TEMP_FAILURE_RETRY(close(fildes));
    if (res < 0) {
        perror("close");
        abort();
    }
}

void
fdio_test_35_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = tx_seq_write_pre("./testdir/testfile2.bin");
}

void
fdio_test_35(unsigned int tid)
{
    extern size_t g_txcycles;

    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    tx_seq_write(g_fildes, &t_seed, 1024 * 1024, g_txcycles);
}

void
fdio_test_35_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    seq_write_post(g_fildes);
}

void
fdio_test_36_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    locked_seq_write_pre("./testdir/testfile2.bin");
}

void
fdio_test_36(unsigned int tid)
{
    extern size_t g_txcycles;

    static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;
    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    locked_seq_write(&s_lock, g_fildes, &t_seed, 1024 * 1024, g_txcycles);
}

void
fdio_test_36_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    seq_write_post(g_fildes);
}

/*
 * 100 MB file
 */


/*
 * Random read/write, ratio 1:1
 */

long long fdio_test_37_ticks = 0;
long long fdio_test_37_count = 0;

void
fdio_test_37_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = tx_random_rw_pre("./testdir/testfile-100.bin");
}

void
fdio_test_37_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    random_rw_post(g_fildes);
}

void
fdio_test_37(unsigned int tid)
{
    extern size_t g_txcycles;

    static __thread unsigned int t_seed = 0; /* thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    tx_random_rw(g_fildes, &t_seed, 100 * 1024 * 1024, g_txcycles, 1);
}

void
fdio_test_38_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = locked_random_rw_pre("./testdir/testfile-100.bin");
}

void
fdio_test_38_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    random_rw_post(g_fildes);
}

void
fdio_test_38(unsigned int tid)
{
    extern size_t g_txcycles;

    static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;
    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    locked_random_rw(&s_lock, g_fildes, &t_seed, 100 * 1024 * 1024,
                     g_txcycles, 1);
}

/*
 * Random read/write, ratio 2:1
 */

void
fdio_test_39_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = tx_random_rw_pre("./testdir/testfile-100.bin");
}

void
fdio_test_39_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    random_rw_post(g_fildes);
}

void
fdio_test_39(unsigned int tid)
{
    extern size_t g_txcycles;

    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    tx_random_rw(g_fildes, &t_seed, 100 * 1024 * 1024, g_txcycles, 2);
}

void
fdio_test_40_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = locked_random_rw_pre("./testdir/testfile-100.bin");
}

void
fdio_test_40_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    random_rw_post(g_fildes);
}

void
fdio_test_40(unsigned int tid)
{
    extern size_t g_txcycles;

    static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;
    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    locked_random_rw(&s_lock, g_fildes, &t_seed, 100 * 1024 * 1024,
                     g_txcycles, 2);
}

/*
 * Random read/write, ratio 4:1
 */

void
fdio_test_41_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = tx_random_rw_pre("./testdir/testfile-100.bin");
}

void
fdio_test_41_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    random_rw_post(g_fildes);
}

void
fdio_test_41(unsigned int tid)
{
    extern size_t g_txcycles;

    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    tx_random_rw(g_fildes, &t_seed, 100 * 1024 * 1024, g_txcycles, 4);
}

void
fdio_test_42_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = locked_random_rw_pre("./testdir/testfile-100.bin");
}

void
fdio_test_42_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    random_rw_post(g_fildes);
}

void
fdio_test_42(unsigned int tid)
{
    extern size_t g_txcycles;

    static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;
    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    locked_random_rw(&s_lock, g_fildes, &t_seed, 100 * 1024 * 1024,
                     g_txcycles, 4);
}

/*
 * Random read/write, ratio 8:1
 */

void
fdio_test_43_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = tx_random_rw_pre("./testdir/testfile-100.bin");
}

void
fdio_test_43_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    random_rw_post(g_fildes);
}

void
fdio_test_43(unsigned int tid)
{
    extern size_t g_txcycles;

    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    tx_random_rw(g_fildes, &t_seed, 100 * 1024 * 1024, g_txcycles, 8);
}

void
fdio_test_44_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = locked_random_rw_pre("./testdir/testfile-100.bin");
}

void
fdio_test_44_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    random_rw_post(g_fildes);
}

void
fdio_test_44(unsigned int tid)
{
    extern size_t g_txcycles;

    static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;
    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    locked_random_rw(&s_lock, g_fildes, &t_seed, 100 * 1024 * 1024,
                     g_txcycles, 8);
}

/*
 * Random reads
 */

void
fdio_test_45_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = tx_random_read_pre("./testdir/testfile-100.bin");
}

void
fdio_test_45_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    random_read_post(g_fildes);
}

void
fdio_test_45(unsigned int tid)
{
    extern size_t g_txcycles;

    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    tx_random_read(g_fildes, &t_seed, 100 * 1024 * 1024, g_txcycles);
}

void
fdio_test_46_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = locked_random_read_pre("./testdir/testfile-100.bin");
}

void
fdio_test_46_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    random_read_post(g_fildes);
}

void
fdio_test_46(unsigned int tid)
{
    extern size_t g_txcycles;

    static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;
    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    locked_random_read(&s_lock, g_fildes, &t_seed, 100 * 1024 * 1024,
                       g_txcycles);
}

/*
 * Random writes
 */

void
fdio_test_47_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = tx_random_write_pre("./testdir/testfile2-100.bin");
}

void
fdio_test_47(unsigned int tid)
{
    extern size_t g_txcycles;

    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    tx_random_write(g_fildes, &t_seed, 100 * 1024 * 1024, g_txcycles);
}

void
fdio_test_47_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    random_write_post(g_fildes);
}

void
fdio_test_48_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = locked_random_write_pre("./testdir/testfile2-100.bin");
}

void
fdio_test_48(unsigned int tid)
{
    extern size_t g_txcycles;

    static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;
    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    locked_random_write(&s_lock, g_fildes, &t_seed, 100 * 1024 * 1024, g_txcycles);
}

void
fdio_test_48_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    random_write_post(g_fildes);
}

/*
 * Sequentiell reads
 */

void
fdio_test_49_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = tx_seq_read_pre("./testdir/testfile-100.bin");
}

void
fdio_test_49(unsigned int tid)
{
    extern size_t g_txcycles;

    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    tx_seq_read(g_fildes, &t_seed, 100 * 1024 * 1024, g_txcycles);
}

void
fdio_test_49_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    seq_read_post(g_fildes);
}

void
fdio_test_50_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = locked_seq_read_pre("./testdir/testfile-100.bin");
}

void
fdio_test_50(unsigned int tid)
{
    extern size_t g_txcycles;

    static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;
    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    locked_seq_read(&s_lock, g_fildes, &t_seed, 100 * 1024 * 1024, g_txcycles);
}

void
fdio_test_50_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    seq_read_post(g_fildes);
}

/*
 * Sequentiell writes
 */

void
fdio_test_51_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = tx_seq_write_pre("./testdir/testfile2-100.bin");
}

void
fdio_test_51(unsigned int tid)
{
    extern size_t g_txcycles;

    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    tx_seq_write(g_fildes, &t_seed, 100 * 1024 * 1024, g_txcycles);
}

void
fdio_test_51_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    seq_write_post(g_fildes);
}

void
fdio_test_52_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = locked_seq_write_pre("./testdir/testfile2-100.bin");
}

void
fdio_test_52(unsigned int tid)
{
    extern size_t g_txcycles;

    static pthread_mutex_t s_lock = PTHREAD_MUTEX_INITIALIZER;
    static __thread unsigned int t_seed = 0; /* Thread-local seed value */

    if (!t_seed) {
        t_seed = tid + 1;
    }

    locked_seq_write(&s_lock, g_fildes, &t_seed, 100 * 1024 * 1024, g_txcycles);
}

void
fdio_test_52_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    seq_write_post(g_fildes);
}
