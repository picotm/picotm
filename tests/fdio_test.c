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

#define _XOPEN_SOURCE 500 /* Needed for pwrite */

#include <string.h>
#include <stdio.h>
#include <tanger-stm.h>
#include <tanger-stm-internal.h>
#include <tanger-stm-ext-actions.h>
#include <tanger-stm-std-errno.h>
#include <tanger-stm-std-unistd.h>
#include <tanger-stm-std-fcntl.h>
#include <tanger-stm-std-stdlib.h>
#include <tanger-stm-std-stdio.h>
#include <tanger-stm-std-pthread.h>
#include "fdio_test.h"

static const char * const g_test_str = "Hello world!\n";

static volatile pthread_t g_curself;

static int tanger_stm_tidcmp(pthread_t a, pthread_t b)
{
    return a-b;
}

static int tanger_stm_spin_tid(pthread_t self)
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

/* Open and close a file.
 */
void
tanger_stm_fdio_test_1(unsigned int tid)
{
    tanger_begin();

    int fildes = TEMP_FAILURE_RETRY(open("/tmp/fdio.test",
                                         O_WRONLY|O_CREAT,
                                         S_IRWXU|S_IRWXG|S_IRWXO));

    if (fildes < 0) {
        perror("open");
        abort();
    }

    if (tanger_stm_spin_tid(pthread_self()) < 0) {
        tanger_commit();
        abort();
    }

    if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
        perror("close");
        abort();
    }

    tanger_commit();
}

/* Append test: Each thread appends its output to the file, should result
 * in as many lines as threads run.
 */
void
tanger_stm_fdio_test_2(unsigned int tid)
{
    tanger_begin();

    int fildes = TEMP_FAILURE_RETRY(open("/tmp/fdio.test",
                                         O_WRONLY|O_CREAT|O_APPEND,
                                         S_IRWXU|S_IRWXG|S_IRWXO));

    if (fildes < 0) {
        perror("open");
        abort();
    }

    if (TEMP_FAILURE_RETRY(write(fildes, g_test_str, strlen(g_test_str))) < 0) {
        perror("write");
        abort();
    }

    if (tanger_stm_spin_tid(pthread_self()) < 0) {
        tanger_commit();
        abort();
    }

    if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
        perror("close");
        abort();
    }

    tanger_commit();
}

/* Each process writes to the beginning of a file. Only the last written
 * line should survive.
 */
void
tanger_stm_fdio_test_3(unsigned int tid)
{
    tanger_begin();

    int fildes = TEMP_FAILURE_RETRY(open("/tmp/fdio.test",
                                         O_WRONLY|O_CREAT,
                                         S_IRWXU|S_IRWXG|S_IRWXO));

    if (fildes < 0) {
        perror("open");
        abort();
    }

    if (lseek(fildes, 0, SEEK_SET) == (off_t)-1) {
        perror("lseek");
        abort();
    }

    if (TEMP_FAILURE_RETRY(write(fildes, g_test_str, strlen(g_test_str))) < 0) {
        perror("write");
        abort();
    }

    /*if (tanger_stm_spin_tid(pthread_self()) < 0) {
        tanger_commit();
        abort();
    }*/

    if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
        perror("close");
        abort();
    }

    tanger_commit();
}

/* Use pwrite to start writing at byte no. 2.
 */
void
tanger_stm_fdio_test_4(unsigned int tid)
{
    tanger_begin();

    int fildes = TEMP_FAILURE_RETRY(open("/tmp/fdio.test",
                                         O_WRONLY|O_CREAT,
                                        S_IRWXU|S_IRWXG|S_IRWXO));

    if (fildes < 0) {
        perror("open");
        abort();
    }

    if (TEMP_FAILURE_RETRY(pwrite(fildes,
                                  g_test_str, strlen(g_test_str), 2)) < 0) {
        perror("pwrite");
        abort();
    }

    /*if (tanger_stm_spin_tid(pthread_self()) < 0) {
        tanger_commit();
        abort();
    }*/

    if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
        perror("close");
        abort();
    }

    tanger_commit();
}

/* Write string, read-back and write again; should output 'Hello Hello!'.
 */
void
tanger_stm_fdio_test_5(unsigned int tid)
{
    char str[32];
    char tidstr[32];

    memcpy(str, g_test_str, strlen(g_test_str)+1);
    snprintf(tidstr, sizeof(tidstr), "%d", (int)tid);
    memcpy(str, tidstr, strlen(tidstr));

    tanger_begin();

    int fildes = TEMP_FAILURE_RETRY(open("/tmp/fdio.test",
                                         O_RDWR|O_CREAT,
                                         S_IRWXU|S_IRWXG|S_IRWXO));

    if (fildes < 0) {
        perror("open");
        abort();
    }

    if (TEMP_FAILURE_RETRY(pwrite(fildes, str, strlen(g_test_str), 0)) < 0) {
        perror("pwrite");
        abort();
    }

    char rbuf[5];
    memset(rbuf, 0, sizeof(rbuf)); /* Workaround valgrind */

    if (TEMP_FAILURE_RETRY(pread(fildes, rbuf, sizeof(rbuf), 0)) < 0) {
        perror("pread");
        abort();
    }

    if (TEMP_FAILURE_RETRY(pwrite(fildes, rbuf, sizeof(rbuf), 6)) < 0) {
        perror("pwrite");
        abort();
    }

    if (tanger_stm_spin_tid(pthread_self()) < 0) {
        tanger_commit();
        abort();
    }

    if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
        perror("close");
        abort();
    }

    tanger_commit();
}

/* Write string at position 2 using lseek; should output '  Hello world!'.
 */
void
tanger_stm_fdio_test_6(unsigned int tid)
{
    char teststr[16];
    snprintf(teststr, 15, "%d\n", (int)tid);

    tanger_begin();

    /* Open file */

    int fildes = TEMP_FAILURE_RETRY(open("/tmp/fdio.test",
                                         O_WRONLY|O_CREAT,
                                         S_IRWXU|S_IRWXG|S_IRWXO));

    if (fildes < 0) {
        perror("open");
        abort();
    }

    /* Do IO */

    if (lseek(fildes, 2, SEEK_CUR) == (off_t)-1) {
        perror("lseek");
        abort();
    }

    if (TEMP_FAILURE_RETRY(write(fildes, g_test_str, strlen(g_test_str))) < 0) {
        perror("write");
        abort();
    }

    /* Write TID to the EOF */

    if (lseek(fildes, 0, SEEK_END) == (off_t)-1) {
        perror("lseek");
        abort();
    }

    if (TEMP_FAILURE_RETRY(write(fildes, teststr, strlen(teststr))) < 0) {
        perror("write");
        abort();
    }

    /* Close */

    if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
        perror("close");
        abort();
    }

    tanger_commit();
}

/* Write string, read-back and write again; should output 'world world!'.
 */
void
tanger_stm_fdio_test_7(unsigned int tid)
{
    char teststr[16];

    if (snprintf(teststr, sizeof(teststr), "%d\n", (int)tid) < 0) {
        perror("snprintf");
        abort();
    }

    tanger_begin();

    /* Open */

    int fildes = TEMP_FAILURE_RETRY(open("/tmp/fdio.test",
                                         O_RDWR|O_CREAT,
                                         S_IRWXU|S_IRWXG|S_IRWXO));

    if (fildes < 0) {
        perror("open");
        abort();
    }

    /* Do IO */

    if (TEMP_FAILURE_RETRY(write(fildes, g_test_str, strlen(g_test_str))) < 0) {
        perror("write");
        abort();
    }

    if (lseek(fildes, 6, SEEK_SET) == (off_t)-1) {
        perror("lseek");
        abort();
    }

    char buf[5];
    memset(buf, 0, sizeof(buf)); /* Workaround valgrind */

    if (TEMP_FAILURE_RETRY(read(fildes, buf, sizeof(buf))) < 0) {
        perror("read");
        abort();
    }

    if (lseek(fildes, 0, SEEK_SET) == (off_t)-1) {
        perror("lseek");
        abort();
    }

    if (TEMP_FAILURE_RETRY(write(fildes, buf, sizeof(buf))) < 0) {
        perror("write");
        abort();
    }

    /* Write TID to the EOF */

    if (lseek(fildes, 0, SEEK_END) == (off_t)-1) {
        perror("lseek");
        abort();
    }

    if (TEMP_FAILURE_RETRY(write(fildes, teststr, strlen(teststr))) < 0) {
        perror("write");
        abort();
    }

    /* Close */

    if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
        perror("close");
        abort();
    }

    tanger_commit();
}

/* Read through pipe to test buffered reads.
 */
void
tanger_stm_fdio_test_8(unsigned int tid)
{
    char filename[32];
    
    if (snprintf(filename, sizeof(filename), "/tmp/fdio-%u.test", tid) < 0) {
        perror("snprintf");
        abort();
    }

    int pfd[2];

    if (pipe(pfd) < 0) {
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

        if (TEMP_FAILURE_RETRY(fcntl(pfd[0], F_SETFL, fl|O_NONBLOCK)) < 0) {
            perror("fcntl");
            abort();
        }
    }

    /* Fill pipe */

    int i;

    for (i = 0; i < 1000; ++i) {

        char str[128];

        if (snprintf(str, sizeof(str), "%d %s", i, g_test_str) < 0) {
            perror("snprintf");
            abort();
        }

        if (TEMP_FAILURE_RETRY(write(pfd[1], str, strlen(str))) < 0) {
            perror("write");
            abort();
        }
    }

    tanger_begin();

    /* Open file */

    int fildes = TEMP_FAILURE_RETRY(open(filename,
                                         O_WRONLY|O_CREAT|O_TRUNC,
                                         S_IRWXU|S_IRWXG|S_IRWXO));

    if (fildes < 0) {
        perror("open");
        abort();
    }

    ssize_t rlen;

    do {
        /* Read from pipe */

        char rbuf[1024];
        memset(rbuf, 0, sizeof(rbuf)); /* Work around valgrind */

        rlen = TEMP_FAILURE_RETRY(read(pfd[0], rbuf, sizeof(rbuf)));

        if (rlen < 0) {
            if (errno == EAGAIN) { /* Pipe empty */
                rlen = 0;
            } else {
                perror("read");
                abort();
            }
        }

        /* Write to file */
        if (TEMP_FAILURE_RETRY(write(fildes, rbuf, rlen)) < 0) {
            perror("write");
            abort();
        }
    } while (rlen);

    if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
        perror("close");
        abort();
    }

    /* Close pipe */

    for (i = 0; i < sizeof(pfd)/sizeof(pfd[0]); ++i) {
        if (TEMP_FAILURE_RETRY(close(pfd[i])) < 0) {
            perror("close");
            abort();
        }
    }

    tanger_commit();
}

/* Write some lines at end of file. Should result in an integer record
 * for each thread. The order of records can vary.
 */
void
tanger_stm_fdio_test_9(unsigned int tid)
{
    static int fildes = -1;
    static pthread_mutex_t fd_lock = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&fd_lock);

    if (fildes < 0) {
        fildes = TEMP_FAILURE_RETRY(open("/tmp/fdio.test",
                                         O_WRONLY|O_CREAT,
                                         S_IRWXU|S_IRWXG|S_IRWXO));

        if (fildes < 0) {
            perror("open");
            abort();
        }
    }

    pthread_mutex_unlock(&fd_lock);

    char str[100][256];
    char (*s)[256];

    for (s = str; s < str+sizeof(str)/sizeof(str[0]); ++s) {
        if (sprintf(*s, "%p line %d\n", (void*)pthread_self(), (int)(s-str)) < 0) {
            perror("snprintf");
            abort();
        }
    }

    tanger_begin();

    if (lseek(fildes, 0, SEEK_END) == (off_t)-1) {
        perror("lseek");
        abort();
    }

    for (s = str; s < str+sizeof(str)/sizeof(str[0]); ++s) {
        if (TEMP_FAILURE_RETRY(write(fildes, *s, strlen(*s))) < 0) {
            perror("write");
            abort();
        }
    }

    tanger_commit();

    /*if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
        perror("close");
        abort();
    }*/
}

/* Write string, read-back and write again; should output 'Hello Hello!'.
 */
void
tanger_stm_fdio_test_10(unsigned int tid)
{
    static int fildes = -1;
    static pthread_mutex_t fd_lock = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&fd_lock);

    if (fildes < 0) {
        fildes = TEMP_FAILURE_RETRY(open("/tmp/fdio.test",
                                         O_RDWR|O_CREAT,
                                         S_IRWXU|S_IRWXG|S_IRWXO));

        if (fildes < 0) {
            perror("open");
            abort();
        }
    }

    pthread_mutex_unlock(&fd_lock);

    tanger_begin();

    if (TEMP_FAILURE_RETRY(pwrite(fildes,
                                  g_test_str, strlen(g_test_str), 0)) < 0) {
        perror("pwrite");
        abort();
    }

    char rbuf[5];
    memset(rbuf, 0, sizeof(rbuf)); /* Workaround valgrind */

    if (TEMP_FAILURE_RETRY(pread(fildes, rbuf, sizeof(rbuf), 0)) < 0) {
        perror("pread");
        abort();
    }

    if (TEMP_FAILURE_RETRY(pwrite(fildes, rbuf, sizeof(rbuf), 6)) < 0) {
        perror("pwrite");
        abort();
    }

    tanger_commit();
}

/* Write string at position 2 using lseek; should output '  Hello world!'.
 */
void
tanger_stm_fdio_test_11(unsigned int tid)
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
                                         O_WRONLY|O_CREAT,
                                         S_IRWXU|S_IRWXG|S_IRWXO));

        if (fildes < 0) {
            perror("open");
            abort();
        }
    }

    pthread_mutex_unlock(&fd_lock);

    tanger_begin();

    if (lseek(fildes, 0, SEEK_SET) == (off_t)-1) {
        perror("lseek");
        abort();
    }

    /* Do IO */

    if (lseek(fildes, 2, SEEK_CUR) == (off_t)-1) {
        perror("lseek");
        abort();
    }

    if (TEMP_FAILURE_RETRY(write(fildes, g_test_str, strlen(g_test_str))) < 0) {
        perror("write");
        abort();
    }

    /* Write TID to the EOF */

    if (lseek(fildes, 0, SEEK_END) == (off_t)-1) {
        perror("lseek");
        abort();
    }

    if (TEMP_FAILURE_RETRY(write(fildes, teststr, strlen(teststr))) < 0) {
        perror("write");
        abort();
    }

    tanger_commit();
}

/* Write string, read-back and write again; should output 'world world!'.
 */
void
tanger_stm_fdio_test_12(unsigned int tid)
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
                                         O_RDWR|O_CREAT,
                                         S_IRWXU|S_IRWXG|S_IRWXO));

        if (fildes < 0) {
            perror("open");
            abort();
        }
    }

    pthread_mutex_unlock(&fd_lock);

    tanger_begin();

    /* Do IO */

    if (lseek(fildes, 0, SEEK_SET) == (off_t)-1) {
        perror("lseek");
        abort();
    }

    if (TEMP_FAILURE_RETRY(write(fildes, g_test_str, strlen(g_test_str))) < 0) {
        perror("write");
        abort();
    }

    if (lseek(fildes, 6, SEEK_SET) == (off_t)-1) {
        perror("lseek");
        abort();
    }

    char buf[5];
    memset(buf, 0, sizeof(buf)); /* Workaround valgrind */

    if (TEMP_FAILURE_RETRY(read(fildes, buf, sizeof(buf))) < 0) {
        perror("read");
        abort();
    }

    if (lseek(fildes, 0, SEEK_SET) == (off_t)-1) {
        perror("lseek");
        abort();
    }

    if (TEMP_FAILURE_RETRY(write(fildes, buf, sizeof(buf))) < 0) {
        perror("write");
        abort();
    }

    /* Write TID to the EOF */

    if (lseek(fildes, 0, SEEK_END) == (off_t)-1) {
        perror("lseek");
        abort();
    }

    if (TEMP_FAILURE_RETRY(write(fildes, teststr, strlen(teststr))) < 0) {
        perror("write");
        abort();
    }

    tanger_commit();
}

/* Write to stdout to test unbuffered writes.
 */
void
tanger_stm_fdio_test_13(unsigned int tid)
{
    int i;

    char str[20][128];

    for (i = 0; i < 20; ++i) {
        if (snprintf(str[i], sizeof(str[i]), "%u %d %s", tid, i, g_test_str) < 0) {
            perror("snprintf");
            abort();
        }
    }

    tanger_begin();

    for (i = 0; i < 20; ++i) {
        if (TEMP_FAILURE_RETRY(write(STDOUT_FILENO, str[i], strlen(str[i]))) < 0) {
            perror("write");
            abort();
        }
    }

    tanger_commit();
}

/* Write some lines at end of file. Should result in an integer record
 * for each thread. The order of records can vary. This tests the
 * optimization for non-depending accesses.
 */
void
tanger_stm_fdio_test_14(unsigned int tid)
{
    static volatile int fildes = -1;
    static pthread_mutex_t fd_lock = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&fd_lock);

    if (fildes < 0) {
        fildes = TEMP_FAILURE_RETRY(open("/tmp/fdio.test",
                                         O_WRONLY|O_CREAT,
                                         S_IRWXU|S_IRWXG|S_IRWXO));

        if (fildes < 0) {
            perror("open");
            abort();
        }
    }

    pthread_mutex_unlock(&fd_lock);

    char str[100][256];
    char (*s)[256];

    for (s = str; s < str+sizeof(str)/sizeof(str[0]); ++s) {
        if (snprintf(*s, sizeof(*s), "%p line %d\n", (void*)pthread_self(), (int)(s-str)) < 0) {
            perror("snprintf");
            abort();
        }
    }

    tanger_begin();

    /*off_t pos = lseek(fildes, 0, SEEK_END);

    if (pos == (off_t)-1) {
        perror("lseek");
        abort();
    }*/

    for (s = str; s < str+sizeof(str)/sizeof(str[0]); ++s) {
        if (TEMP_FAILURE_RETRY(write(fildes, *s, strlen(*s))) < 0) {
            perror("write");
            abort();
        }
    }

    tanger_commit();

    /*if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
        perror("close");
        abort();
    }*/
}

/* Open, dup and close a file descriptor.
 */
void
tanger_stm_fdio_test_15(unsigned int tid)
{
    int fildes = TEMP_FAILURE_RETRY(open("/tmp/fdio.test",
                                         O_WRONLY|O_CREAT,
                                         S_IRWXU|S_IRWXG|S_IRWXO));

    if (fildes < 0) {
        perror("open");
        abort();
    }

    tanger_begin();

    int fildes2 = TEMP_FAILURE_RETRY(dup(fildes));

    if (fildes2 < 0) {
        perror("dup");
        abort();
    }

    if (tanger_stm_spin_tid(pthread_self()) < 0) {
        tanger_commit();
        abort();
    }

    if (TEMP_FAILURE_RETRY(close(fildes2)) < 0) {
        perror("close");
        abort();
    }

    tanger_commit();

    if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
        perror("close");
        abort();
    }
}

/* Open, dup and close a file descriptor.
 */
void
tanger_stm_fdio_test_16(unsigned int tid)
{
    extern enum ccmode g_ccmode;

    tanger_stm_ofd_type_set_ccmode(TYPE_REGULAR, g_ccmode);

    tanger_begin();

    int fildes = TEMP_FAILURE_RETRY(open("/tmp/fdio.test",
                                         O_WRONLY|O_CREAT,
                                         S_IRWXU|S_IRWXG|S_IRWXO));

    if (fildes < 0) {
        perror("open");
        abort();
    }

    int fildes2 = TEMP_FAILURE_RETRY(dup(fildes));

    if (fildes2 < 0) {
        perror("dup");
        abort();
    }

    if (tanger_stm_spin_tid(pthread_self()) < 0) {
        tanger_commit();
        abort();
    }

    if (TEMP_FAILURE_RETRY(close(fildes2)) < 0) {
        perror("close");
        abort();
    }

    if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
        perror("close");
        abort();
    }

    tanger_commit();
}

/* Open, dup, write, and close a file descriptor.
 */
void
tanger_stm_fdio_test_17(unsigned int tid)
{
    extern enum ccmode g_ccmode;

    tanger_stm_ofd_type_set_ccmode(TYPE_REGULAR, g_ccmode);

    char str[2][128];
    char (*s)[128];

    for (s = str; s < str+sizeof(str)/sizeof(str[0]); ++s) {
        if (snprintf(*s, sizeof(str[0]), "%d %d %s", tid, (int)(s-str), g_test_str) < 0) {
            perror("snprintf");
            abort();
        }
    }

    tanger_begin();

    int fildes = TEMP_FAILURE_RETRY(open("/tmp/fdio.test",
                                         O_WRONLY|O_CREAT,
                                         S_IRWXU|S_IRWXG|S_IRWXO));

    if (fildes < 0) {
        perror("open");
        abort();
    }

    int fildes2 = TEMP_FAILURE_RETRY(dup(fildes));

    if (fildes2 < 0) {
        perror("dup");
        abort();
    }

    if (lseek(fildes, 0, SEEK_END) == (off_t)-1) {
        perror("lseek1");
        abort();
    }

    if (lseek(fildes2, 0, SEEK_END) == (off_t)-1) {
        perror("lseek2");
        abort();
    }

    if (TEMP_FAILURE_RETRY(write(fildes, str[0], strlen(str[0]))) < 0) {
        perror("write1");
        abort();
    }

    if (TEMP_FAILURE_RETRY(write(fildes2, str[1], strlen(str[1]))) < 0) {
        perror("write2");
        abort();
    }

    if (TEMP_FAILURE_RETRY(close(fildes2)) < 0) {
        perror("close2");
        abort();
    }

    if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
        perror("close1");
        abort();
    }

    tanger_commit();
}

/* Open, dup, write, and close a file descriptor.
 */
void
tanger_stm_fdio_test_18(unsigned int tid)
{
    extern enum ccmode g_ccmode;
    tanger_stm_ofd_type_set_ccmode(TYPE_REGULAR, g_ccmode);

    static volatile int fildes = -1;
    static pthread_mutex_t fd_lock = PTHREAD_MUTEX_INITIALIZER;

    pthread_mutex_lock(&fd_lock);

    if (fildes < 0) {
        fildes = TEMP_FAILURE_RETRY(open("/tmp/fdio.test",
                                         O_WRONLY|O_CREAT,
                                         S_IRWXU|S_IRWXG|S_IRWXO));

        if (fildes < 0) {
            perror("open");
            abort();
        }
    }

    pthread_mutex_unlock(&fd_lock);

    char str[2][128];
    char (*s)[128];

    for (s = str; s < str+sizeof(str)/sizeof(str[0]); ++s) {
        if (snprintf(*s, sizeof(str[0]), "%d %d %s", tid, (int)(s-str), g_test_str) < 0) {
            perror("snprintf");
            abort();
        }
    }

    tanger_begin();

    /*int fildes = TEMP_FAILURE_RETRY(open("/tmp/fdio.test",
                                         O_WRONLY|O_CREAT,
                                         S_IRWXU|S_IRWXG|S_IRWXO));

    if (fildes < 0) {
        perror("open");
        abort();
    }*/

    int fildes2 = TEMP_FAILURE_RETRY(dup(fildes));

    if (fildes2 < 0) {
        perror("dup");
        abort();
    }

    if (lseek(fildes, 0, SEEK_END) == (off_t)-1) {
        perror("lseek1");
        abort();
    }

    if (lseek(fildes2, 0, SEEK_END) == (off_t)-1) {
        perror("lseek2");
        abort();
    }

    if (TEMP_FAILURE_RETRY(write(fildes, str[0], strlen(str[0]))) < 0) {
        perror("write1");
        abort();
    }

    if (TEMP_FAILURE_RETRY(write(fildes2, str[1], strlen(str[1]))) < 0) {
        perror("write2");
        abort();
    }

    if (TEMP_FAILURE_RETRY(close(fildes2)) < 0) {
        perror("close");
        abort();
    }

    tanger_commit();

    /*if (TEMP_FAILURE_RETRY(close(fildes)) < 0) {
        perror("close");
        abort();
    }*/
}


/* Write to stdout to test unbuffered writes.
 */
void
tanger_stm_fdio_test_19(unsigned int tid)
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

        int i;

        for (i = 0; i < 10; ++i) {

            char str[128];

            if (snprintf(str, sizeof(str), "%u %d %s", tid, i, g_test_str) < 0) {
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

    ssize_t rlen;

    do {
        tanger_begin();

        char rbuf[10];
        memset(rbuf, 0, sizeof(rbuf)); /* Work around valgrind */

        /* Read from pipe */
        rlen = TEMP_FAILURE_RETRY(read(pfd[0], rbuf, sizeof(rbuf)));

        if (rlen < 0) {
            if (errno == EAGAIN) { /* Pipe empty */
                rlen = 0;
            } else {
                perror("read");
                abort();
            }
        }

        /* Write to stdout */
        if (TEMP_FAILURE_RETRY(write(STDOUT_FILENO, rbuf, rlen)) < 0) {
            perror("write");
            abort();
        }

        tanger_commit();

    } while (rlen);

    /* Close pipe */
    /*for (i = 0; i < sizeof(pfd)/sizeof(pfd[0]); ++i) {
        if (TEMP_FAILURE_RETRY(close(pfd[i])) < 0) {
            perror("close");
            abort();
        }
    }*/
}

/* Write string, read-back and write again; should output 'Hello Hello!'.
 */

static int g_fildes = -1;

void
tanger_stm_fdio_test_20_pre()
{
    extern enum ccmode g_ccmode;

    tanger_stm_ofd_type_set_ccmode(TYPE_REGULAR, g_ccmode);

    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile.bin",
                                       O_RDWR|O_CREAT,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_20_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

/*typedef unsigned long long ticks;
static __inline__ ticks getticks(void)
{
     unsigned a, d; 
     __asm__ volatile("rdtsc" : "=a" (a), "=d" (d)); 
     return ((ticks)a) | (((ticks)d) << 32); 
}*/

void
tanger_stm_fdio_test_20(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    unsigned char buf[24];

    if (!seed) {
        seed = tid+1;
    }
    off_t offset = rand_r(&seed)%(1024*1024);

    tanger_begin();

    ssize_t count = TEMP_FAILURE_RETRY(pread(g_fildes,
                                             buf,
                                             sizeof(buf),
                                             offset));

    if (count < 0) {
        perror("pread");
        abort();
    }
    if (TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, count, offset)) < 0) {
        perror("pwrite");
        abort();
    }

    tanger_commit();
}

/*
 * Random read/write, ratio 1:1
 */

/*long long fdio_test_21_ticks = 0;
long long fdio_test_21_count = 0;*/

void
tanger_stm_fdio_test_21_pre()
{
    extern enum ccmode g_ccmode;

    tanger_stm_ofd_type_set_ccmode(TYPE_REGULAR, g_ccmode);

    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile.bin",
                                       O_RDWR,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_21_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_21(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* thread-local seed value */
    extern size_t g_txcycles;
    size_t i;

    if (!seed) {
        seed = tid+1;
    }

/*    ticks t0 = getticks();*/

    tanger_begin();

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char buf[24];

        off_t offset = rand_r(&seed)%(1024*1024);

        ssize_t count = TEMP_FAILURE_RETRY(pread(g_fildes,
                                                 buf,
                                                 sizeof(buf),
                                                 offset));

        if (count < 0) {
            perror("pread");
            abort();
        }
        if (TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, count, offset)) < 0) {
            perror("pwrite");
            abort();
        }
    }

    tanger_commit();

/*    ticks t1 = getticks();
    fdio_test_21_ticks += t1-t0;
    ++fdio_test_21_count;*/
}

void
tanger_stm_fdio_test_22_pre()
{
    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile.bin",
                                       O_RDWR,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_22_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_22(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    extern size_t g_txcycles;
    size_t i;

    if (!seed) {
        seed = tid+1;
    }

    pthread_mutex_lock(&lock);

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char buf[24];

        off_t offset = rand_r(&seed)%(1024*1024);

        ssize_t count = TEMP_FAILURE_RETRY(pread(g_fildes,
                                                 buf,
                                                 sizeof(buf),
                                                 offset));

        if (count < 0) {
            perror("pread");
            abort();
        }
        if (TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, count, offset)) < 0) {
            perror("pwrite");
            abort();
        }
    }

    pthread_mutex_unlock(&lock);
}

/*
 * Random read/write, ratio 2:1
 */

void
tanger_stm_fdio_test_23_pre()
{
    extern enum ccmode g_ccmode;

    tanger_stm_ofd_type_set_ccmode(TYPE_REGULAR, g_ccmode);

    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile.bin",
                                       O_RDWR,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_23_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_23(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    extern size_t g_txcycles;
    size_t i;

    if (!seed) {
        seed = tid+1;
    }

    tanger_begin();

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char buf[24];

        off_t offset;
        int j;
        ssize_t count;

        for (j = 0; j < 2; ++j) {
            offset = rand_r(&seed)%(1024*1024);

            count = TEMP_FAILURE_RETRY(pread(g_fildes,
                                             buf,
                                             sizeof(buf),
                                             offset));

            if (count < 0) {
                perror("pread");
                abort();
            }
        }
        if (TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, count, offset)) < 0) {
            perror("pwrite");
            abort();
        }
    }

    tanger_commit();
}

void
tanger_stm_fdio_test_24_pre()
{
    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile.bin",
                                       O_RDWR,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_24_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_24(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    extern size_t g_txcycles;
    size_t i;

    if (!seed) {
        seed = tid+1;
    }

    pthread_mutex_lock(&lock);

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char buf[24];

        ssize_t count;
        off_t offset;
        int j;

        for (j = 0; j < 2; ++j) {
            offset = rand_r(&seed)%(1024*1024);

            count = TEMP_FAILURE_RETRY(pread(g_fildes,
                                             buf,
                                             sizeof(buf),
                                             offset));

            if (count < 0) {
                perror("pread");
                abort();
            }
        }

        if (TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, count, offset)) < 0) {
            perror("pwrite");
            abort();
        }
    }

    pthread_mutex_unlock(&lock);
}

/*
 * Random read/write, ratio 4:1
 */

void
tanger_stm_fdio_test_25_pre()
{
    extern enum ccmode g_ccmode;

    tanger_stm_ofd_type_set_ccmode(TYPE_REGULAR, g_ccmode);

    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile.bin",
                                       O_RDWR,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_25_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_25(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    extern size_t g_txcycles;
    size_t i;

    if (!seed) {
        seed = tid+1;
    }

    tanger_begin();

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char buf[24];

        ssize_t count;
        off_t offset;
        int j;

        for (j = 0; j < 4; ++j) {
            offset = rand_r(&seed)%(1024*1024);

            count = TEMP_FAILURE_RETRY(pread(g_fildes,
                                             buf,
                                             sizeof(buf),
                                             offset));

            if (count < 0) {
                perror("pread");
                abort();
            }
        }

        if (TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, count, offset)) < 0) {
            perror("pwrite");
            abort();
        }
    }

    tanger_commit();
}

void
tanger_stm_fdio_test_26_pre()
{
    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile.bin",
                                       O_RDWR,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_26_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_26(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    extern size_t g_txcycles;
    size_t i;

    if (!seed) {
        seed = tid+1;
    }

    pthread_mutex_lock(&lock);

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char buf[24];

        ssize_t count;
        off_t offset;
        int j;

        for (j = 0; j < 4; ++j) {
            offset = rand_r(&seed)%(1024*1024);

            count = TEMP_FAILURE_RETRY(pread(g_fildes,
                                             buf,
                                             sizeof(buf),
                                             offset));

            if (count < 0) {
                perror("pread");
                abort();
            }
        }
        if (TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, count, offset)) < 0) {
            perror("pwrite");
            abort();
        }
    }

    pthread_mutex_unlock(&lock);
}

/*
 * Random read/write, ratio 8:1
 */

void
tanger_stm_fdio_test_27_pre()
{
    extern enum ccmode g_ccmode;

    tanger_stm_ofd_type_set_ccmode(TYPE_REGULAR, g_ccmode);

    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile.bin",
                                       O_RDWR,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_27_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_27(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    extern size_t g_txcycles;
    size_t i;

    if (!seed) {
        seed = tid+1;
    }

    tanger_begin();

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char buf[24];

        ssize_t count;
        off_t offset;
        int j;

        for (j = 0; j < 8; ++j) {
            offset = rand_r(&seed)%(1024*1024);

            count = TEMP_FAILURE_RETRY(pread(g_fildes,
                                             buf,
                                             sizeof(buf),
                                             offset));

            if (count < 0) {
                perror("pread");
                abort();
            }
        }
        if (TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, count, offset)) < 0) {
            perror("pwrite");
            abort();
        }
    }

    tanger_commit();
}

void
tanger_stm_fdio_test_28_pre()
{
    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile.bin",
                                       O_RDWR,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_28_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_28(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    extern size_t g_txcycles;
    size_t i;

    if (!seed) {
        seed = tid+1;
    }

    pthread_mutex_lock(&lock);

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char buf[24];

        ssize_t count;
        off_t offset;
        int j;

        for (j = 0; j < 8; ++j) {
            offset = rand_r(&seed)%(1024*1024);

            count = TEMP_FAILURE_RETRY(pread(g_fildes,
                                             buf,
                                             sizeof(buf),
                                             offset));

            if (count < 0) {
                perror("pread");
                abort();
            }
        }
        if (TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, count, offset)) < 0) {
            perror("pwrite");
            abort();
        }
    }

    pthread_mutex_unlock(&lock);
}

/*
 * Random reads
 */

void
tanger_stm_fdio_test_29_pre()
{
    extern enum ccmode g_ccmode;

    tanger_stm_ofd_type_set_ccmode(TYPE_REGULAR, g_ccmode);

    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile.bin",
                                       O_RDONLY|O_CREAT,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_29_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_29(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    extern size_t g_txcycles;
    size_t i;

    if (!seed) {
        seed = tid+1;
    }

    tanger_begin();

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char buf[24];

        off_t offset = rand_r(&seed)%(1024*1024);

        ssize_t count = TEMP_FAILURE_RETRY(pread(g_fildes,
                                                 buf,
                                                 sizeof(buf),
                                                 offset));

        if (count < 0) {
            perror("pread");
            abort();
        }
    }

    tanger_commit();
}

void
tanger_stm_fdio_test_30_pre()
{
    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile.bin",
                                       O_RDONLY|O_CREAT,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_30_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_30(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    extern size_t g_txcycles;
    size_t i;

    if (!seed) {
        seed = tid+1;
    }

    pthread_mutex_lock(&lock);

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char buf[24];

        off_t offset = rand_r(&seed)%(1024*1024);

        ssize_t count = TEMP_FAILURE_RETRY(pread(g_fildes,
                                                 buf,
                                                 sizeof(buf),
                                                 offset));

        if (count < 0) {
            perror("pread");
            abort();
        }
    }

    pthread_mutex_unlock(&lock);
}

/*
 * Random writes
 */

void
tanger_stm_fdio_test_31_pre()
{
    extern enum ccmode g_ccmode;

    tanger_stm_ofd_type_set_ccmode(TYPE_REGULAR, g_ccmode);

    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile2.bin",
                                       O_WRONLY|O_CREAT,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_31(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    extern size_t g_txcycles;
    size_t i;
    unsigned char buf[24];

    if (!seed) {
        seed = tid+1;
    }

    memset(buf, 0, sizeof(buf));

    tanger_begin();

    for (i = 0; i < g_txcycles; ++i) {

        off_t offset = rand_r(&seed)%(1024*1024);

        ssize_t count =
            TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, sizeof(buf), offset));

        if (count < 0) {
            perror("pwrite");
            abort();
        }
    }

    tanger_commit();
}

void
tanger_stm_fdio_test_31_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_32_pre()
{
    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile2.bin",
                                       O_WRONLY|O_CREAT,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_32(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    extern size_t g_txcycles;
    size_t i;
    unsigned char buf[24];

    if (!seed) {
        seed = tid+1;
    }

    memset(buf, 0, sizeof(buf));

    pthread_mutex_lock(&lock);

    for (i = 0; i < g_txcycles; ++i) {

        off_t offset = rand_r(&seed)%(1024*1024);

        ssize_t count =
            TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, sizeof(buf), offset));

        if (count < 0) {
            perror("pwrite");
            abort();
        }
    }

    pthread_mutex_unlock(&lock);
}

void
tanger_stm_fdio_test_32_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

/*
 * Sequentiell reads
 */

void
tanger_stm_fdio_test_33_pre()
{
    extern enum ccmode g_ccmode;

    tanger_stm_ofd_type_set_ccmode(TYPE_REGULAR, g_ccmode);

    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile.bin",
                                       O_RDONLY|O_CREAT,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_33(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    extern size_t g_txcycles;
    size_t i;

    if (!seed) {
        seed = tid+1;
    }

    off_t offset = rand_r(&seed)%(1024*1024);

    tanger_begin();

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char buf[24];

        ssize_t count =
             TEMP_FAILURE_RETRY(pread(g_fildes, buf, sizeof(buf), offset));

        if (count < 0) {
            perror("pread");
            abort();
        }

        offset += count;
    }

    tanger_commit();
}

void
tanger_stm_fdio_test_33_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_34_pre()
{
    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile.bin",
                                       O_RDONLY|O_CREAT,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_34(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    extern size_t g_txcycles;
    size_t i;

    if (!seed) {
        seed = tid+1;
    }

    off_t offset = rand_r(&seed)%(1024*1024);

    pthread_mutex_lock(&lock);

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char buf[24];

        ssize_t count =
            TEMP_FAILURE_RETRY(pread(g_fildes, buf, sizeof(buf), offset));

        if (count < 0) {
            perror("pread");
            abort();
        }

        offset += count;
    }

    pthread_mutex_unlock(&lock);
}

void
tanger_stm_fdio_test_34_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

/*
 * Sequentiell writes
 */

void
tanger_stm_fdio_test_35_pre()
{
    extern enum ccmode g_ccmode;

    tanger_stm_ofd_type_set_ccmode(TYPE_REGULAR, g_ccmode);

    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile2.bin",
                                       O_WRONLY|O_CREAT,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_35(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    extern size_t g_txcycles;
    size_t i;
    unsigned char buf[24];
    off_t offset;

    if (!seed) {
        seed = tid+1;
    }

    memset(buf, 0, sizeof(buf));
    offset = rand_r(&seed)%(1024*1024);

    tanger_begin();

    for (i = 0; i < g_txcycles; ++i) {

        ssize_t count =
            TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, sizeof(buf), offset));

        if (count < 0) {
            perror("pwrite");
            abort();
        }

        offset += count;
    }

    tanger_commit();
}

void
tanger_stm_fdio_test_35_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_36_pre()
{
    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile2.bin",
                                       O_WRONLY|O_CREAT,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_36(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    extern size_t g_txcycles;
    size_t i;
    unsigned char buf[24];
    off_t offset;

    if (!seed) {
        seed = tid+1;
    }

    memset(buf, 0, sizeof(buf));
    offset = rand_r(&seed)%(1024*1024);

    pthread_mutex_lock(&lock);

    for (i = 0; i < g_txcycles; ++i) {

        ssize_t count =
            TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, sizeof(buf), offset));

        if (count < 0) {
            perror("pwrite");
            abort();
        }
        
        offset += count;
    }

    pthread_mutex_unlock(&lock);
}

void
tanger_stm_fdio_test_36_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
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
tanger_stm_fdio_test_37_pre()
{
    extern enum ccmode g_ccmode;

    tanger_stm_ofd_type_set_ccmode(TYPE_REGULAR, g_ccmode);

    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile-100.bin",
                                       O_RDWR,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_37_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_37(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* thread-local seed value */
    extern size_t g_txcycles;
    size_t i;

    if (!seed) {
        seed = tid+1;
    }

/*    ticks t0 = getticks();*/

    tanger_begin();

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char buf[24];

        off_t offset = rand_r(&seed)%(1024*1024*100);

        ssize_t count = TEMP_FAILURE_RETRY(pread(g_fildes,
                                                 buf,
                                                 sizeof(buf),
                                                 offset));

        if (count < 0) {
            perror("pread");
            abort();
        }
        if (TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, count, offset)) < 0) {
            perror("pwrite");
            abort();
        }
    }

    tanger_commit();

/*    ticks t1 = getticks();

    fdio_test_37_ticks += t1-t0;
    ++fdio_test_37_count;*/
}

void
tanger_stm_fdio_test_38_pre()
{
    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile-100.bin",
                                       O_RDWR,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_38_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_38(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    extern size_t g_txcycles;
    size_t i;

    if (!seed) {
        seed = tid+1;
    }

    pthread_mutex_lock(&lock);

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char buf[24];

        off_t offset = rand_r(&seed)%(1024*1024*100);

        ssize_t count = TEMP_FAILURE_RETRY(pread(g_fildes,
                                                 buf,
                                                 sizeof(buf),
                                                 offset));

        if (count < 0) {
            perror("pread");
            abort();
        }
        if (TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, count, offset)) < 0) {
            perror("pwrite");
            abort();
        }
    }

    pthread_mutex_unlock(&lock);
}

/*
 * Random read/write, ratio 2:1
 */

void
tanger_stm_fdio_test_39_pre()
{
    extern enum ccmode g_ccmode;

    tanger_stm_ofd_type_set_ccmode(TYPE_REGULAR, g_ccmode);

    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile-100.bin",
                                       O_RDWR,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_39_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_39(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    extern size_t g_txcycles;
    size_t i;

    if (!seed) {
        seed = tid+1;
    }

    tanger_begin();

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char buf[24];

        off_t offset;
        int j;
        ssize_t count;

        for (j = 0; j < 2; ++j) {
            offset = rand_r(&seed)%(1024*1024*100);

            count = TEMP_FAILURE_RETRY(pread(g_fildes,
                                             buf,
                                             sizeof(buf),
                                             offset));

            if (count < 0) {
                perror("pread");
                abort();
            }
        }
        if (TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, count, offset)) < 0) {
            perror("pwrite");
            abort();
        }
    }

    tanger_commit();
}

void
tanger_stm_fdio_test_40_pre()
{
    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile-100.bin",
                                       O_RDWR,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_40_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_40(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    extern size_t g_txcycles;
    size_t i;

    if (!seed) {
        seed = tid+1;
    }

    pthread_mutex_lock(&lock);

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char buf[24];

        ssize_t count;
        off_t offset;
        int j;

        for (j = 0; j < 2; ++j) {
            offset = rand_r(&seed)%(1024*1024*100);

            count = TEMP_FAILURE_RETRY(pread(g_fildes,
                                             buf,
                                             sizeof(buf),
                                             offset));

            if (count < 0) {
                perror("pread");
                abort();
            }
        }

        if (TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, count, offset)) < 0) {
            perror("pwrite");
            abort();
        }
    }

    pthread_mutex_unlock(&lock);
}

/*
 * Random read/write, ratio 4:1
 */

void
tanger_stm_fdio_test_41_pre()
{
    extern enum ccmode g_ccmode;

    tanger_stm_ofd_type_set_ccmode(TYPE_REGULAR, g_ccmode);

    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile-100.bin",
                                       O_RDWR,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_41_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_41(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    extern size_t g_txcycles;
    size_t i;

    if (!seed) {
        seed = tid+1;
    }

    tanger_begin();

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char buf[24];

        ssize_t count;
        off_t offset;
        int j;

        for (j = 0; j < 4; ++j) {
            offset = rand_r(&seed)%(1024*1024*100);

            count = TEMP_FAILURE_RETRY(pread(g_fildes,
                                             buf,
                                             sizeof(buf),
                                             offset));

            if (count < 0) {
                perror("pread");
                abort();
            }
        }

        if (TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, count, offset)) < 0) {
            perror("pwrite");
            abort();
        }
    }

    tanger_commit();
}

void
tanger_stm_fdio_test_42_pre()
{
    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile-100.bin",
                                       O_RDWR,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_42_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_42(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    extern size_t g_txcycles;
    size_t i;

    if (!seed) {
        seed = tid+1;
    }

    pthread_mutex_lock(&lock);

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char buf[24];

        ssize_t count;
        off_t offset;
        int j;

        for (j = 0; j < 4; ++j) {
            offset = rand_r(&seed)%(1024*1024*100);

            count = TEMP_FAILURE_RETRY(pread(g_fildes,
                                             buf,
                                             sizeof(buf),
                                             offset));

            if (count < 0) {
                perror("pread");
                abort();
            }
        }
        if (TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, count, offset)) < 0) {
            perror("pwrite");
            abort();
        }
    }

    pthread_mutex_unlock(&lock);
}

/*
 * Random read/write, ratio 8:1
 */

void
tanger_stm_fdio_test_43_pre()
{
    extern enum ccmode g_ccmode;

    tanger_stm_ofd_type_set_ccmode(TYPE_REGULAR, g_ccmode);

    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile-100.bin",
                                       O_RDWR,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_43_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_43(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    extern size_t g_txcycles;
    size_t i;

    if (!seed) {
        seed = tid+1;
    }

    tanger_begin();

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char buf[24];

        ssize_t count;
        off_t offset;
        int j;

        for (j = 0; j < 8; ++j) {
            offset = rand_r(&seed)%(1024*1024*100);

            count = TEMP_FAILURE_RETRY(pread(g_fildes,
                                             buf,
                                             sizeof(buf),
                                             offset));

            if (count < 0) {
                perror("pread");
                abort();
            }
        }
        if (TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, count, offset)) < 0) {
            perror("pwrite");
            abort();
        }
    }

    tanger_commit();
}

void
tanger_stm_fdio_test_44_pre()
{
    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile-100.bin",
                                       O_RDWR,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_44_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_44(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    extern size_t g_txcycles;
    size_t i;

    if (!seed) {
        seed = tid+1;
    }

    pthread_mutex_lock(&lock);

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char buf[24];

        ssize_t count;
        off_t offset;
        int j;

        for (j = 0; j < 8; ++j) {
            offset = rand_r(&seed)%(1024*1024*100);

            count = TEMP_FAILURE_RETRY(pread(g_fildes,
                                             buf,
                                             sizeof(buf),
                                             offset));

            if (count < 0) {
                perror("pread");
                abort();
            }
        }
        if (TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, count, offset)) < 0) {
            perror("pwrite");
            abort();
        }
    }

    pthread_mutex_unlock(&lock);
}

/*
 * Random reads
 */

void
tanger_stm_fdio_test_45_pre()
{
    extern enum ccmode g_ccmode;

    tanger_stm_ofd_type_set_ccmode(TYPE_REGULAR, g_ccmode);

    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile-100.bin",
                                       O_RDONLY|O_CREAT,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_45_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_45(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    extern size_t g_txcycles;
    size_t i;

    if (!seed) {
        seed = tid+1;
    }

    tanger_begin();

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char buf[24];

        off_t offset = rand_r(&seed)%(1024*1024*100);

        ssize_t count = TEMP_FAILURE_RETRY(pread(g_fildes,
                                                 buf,
                                                 sizeof(buf),
                                                 offset));

        if (count < 0) {
            perror("pread");
            abort();
        }
    }

    tanger_commit();
}

void
tanger_stm_fdio_test_46_pre()
{
    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile-100.bin",
                                       O_RDONLY|O_CREAT,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_46_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_46(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    extern size_t g_txcycles;
    size_t i;

    if (!seed) {
        seed = tid+1;
    }

    pthread_mutex_lock(&lock);

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char buf[24];

        off_t offset = rand_r(&seed)%(1024*1024*100);

        ssize_t count = TEMP_FAILURE_RETRY(pread(g_fildes,
                                                 buf,
                                                 sizeof(buf),
                                                 offset));

        if (count < 0) {
            perror("pread");
            abort();
        }
    }

    pthread_mutex_unlock(&lock);
}

/*
 * Random writes
 */

void
tanger_stm_fdio_test_47_pre()
{
    extern enum ccmode g_ccmode;

    tanger_stm_ofd_type_set_ccmode(TYPE_REGULAR, g_ccmode);

    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile2-100.bin",
                                       O_WRONLY|O_CREAT,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_47(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    extern size_t g_txcycles;
    size_t i;
    unsigned char buf[24];

    if (!seed) {
        seed = tid+1;
    }

    memset(buf, 0, sizeof(buf));

    tanger_begin();

    for (i = 0; i < g_txcycles; ++i) {

        off_t offset = rand_r(&seed)%(1024*1024*100);

        ssize_t count =
            TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, sizeof(buf), offset));

        if (count < 0) {
            perror("pwrite");
            abort();
        }
    }

    tanger_commit();
}

void
tanger_stm_fdio_test_47_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_48_pre()
{
    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile2-100.bin",
                                       O_WRONLY|O_CREAT,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_48(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    extern size_t g_txcycles;
    size_t i;
    unsigned char buf[24];

    if (!seed) {
        seed = tid+1;
    }

    memset(buf, 0, sizeof(buf));

    pthread_mutex_lock(&lock);

    for (i = 0; i < g_txcycles; ++i) {

        off_t offset = rand_r(&seed)%(1024*1024*100);

        ssize_t count =
            TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, sizeof(buf), offset));

        if (count < 0) {
            perror("pwrite");
            abort();
        }
    }

    pthread_mutex_unlock(&lock);
}

void
tanger_stm_fdio_test_48_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

/*
 * Sequentiell reads
 */

void
tanger_stm_fdio_test_49_pre()
{
    extern enum ccmode g_ccmode;

    tanger_stm_ofd_type_set_ccmode(TYPE_REGULAR, g_ccmode);

    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile-100.bin",
                                       O_RDONLY|O_CREAT,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_49(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    extern size_t g_txcycles;
    size_t i;

    if (!seed) {
        seed = tid+1;
    }

    off_t offset = rand_r(&seed)%(1024*1024*100);

    tanger_begin();

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char buf[24];

        ssize_t count =
             TEMP_FAILURE_RETRY(pread(g_fildes, buf, sizeof(buf), offset));

        if (count < 0) {
            perror("pread");
            abort();
        }

        offset += count;
    }

    tanger_commit();
}

void
tanger_stm_fdio_test_49_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_50_pre()
{
    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile-100.bin",
                                       O_RDONLY|O_CREAT,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_50(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    extern size_t g_txcycles;
    size_t i;

    if (!seed) {
        seed = tid+1;
    }

    off_t offset = rand_r(&seed)%(1024*1024*100);

    pthread_mutex_lock(&lock);

    for (i = 0; i < g_txcycles; ++i) {
        unsigned char buf[24];

        ssize_t count =
            TEMP_FAILURE_RETRY(pread(g_fildes, buf, sizeof(buf), offset));

        if (count < 0) {
            perror("pread");
            abort();
        }

        offset += count;
    }

    pthread_mutex_unlock(&lock);
}

void
tanger_stm_fdio_test_50_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

/*
 * Sequentiell writes
 */

void
tanger_stm_fdio_test_51_pre()
{
    extern enum ccmode g_ccmode;

    tanger_stm_ofd_type_set_ccmode(TYPE_REGULAR, g_ccmode);

    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile2-100.bin",
                                       O_WRONLY|O_CREAT,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_51(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    extern size_t g_txcycles;
    size_t i;
    unsigned char buf[24];
    off_t offset;

    if (!seed) {
        seed = tid+1;
    }

    memset(buf, 0, sizeof(buf));
    offset = rand_r(&seed)%(1024*1024*100);

    tanger_begin();

    for (i = 0; i < g_txcycles; ++i) {

        ssize_t count =
            TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, sizeof(buf), offset));

        if (count < 0) {
            perror("pwrite");
            abort();
        }

        offset += count;
    }

    tanger_commit();
}

void
tanger_stm_fdio_test_51_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}

void
tanger_stm_fdio_test_52_pre()
{
    g_fildes = TEMP_FAILURE_RETRY(open("./testdir/testfile2-100.bin",
                                       O_WRONLY|O_CREAT,
                                       S_IRWXU|S_IRWXG|S_IRWXO));

    if (g_fildes < 0) {
        perror("open");
        abort();
    }
}

void
tanger_stm_fdio_test_52(unsigned int tid)
{
    static __thread unsigned int seed = 0; /* Thread-local seed value */
    static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
    extern size_t g_txcycles;
    size_t i;
    unsigned char buf[24];
    off_t offset;

    if (!seed) {
        seed = tid+1;
    }

    memset(buf, 0, sizeof(buf));
    offset = rand_r(&seed)%(1024*1024*100);

    pthread_mutex_lock(&lock);

    for (i = 0; i < g_txcycles; ++i) {

        ssize_t count =
            TEMP_FAILURE_RETRY(pwrite(g_fildes, buf, sizeof(buf), offset));

        if (count < 0) {
            perror("pwrite");
            abort();
        }
        
        offset += count;
    }

    pthread_mutex_unlock(&lock);
}

void
tanger_stm_fdio_test_52_post()
{
    fsync(g_fildes);

    if (TEMP_FAILURE_RETRY(close(g_fildes)) < 0) {
        perror("close");
        abort();
    }
}
 
