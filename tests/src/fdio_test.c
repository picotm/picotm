/* Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "fdio_test.h"
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <picotm/fcntl.h>
#include <picotm/stddef.h>
#include <picotm/stdlib.h>
#include <picotm/string.h>
#include <picotm/string-tm.h>
#include <picotm/sys/types.h>
#include <picotm/picotm.h>
#include <picotm/picotm-libc.h>
#include <picotm/picotm-tm.h>
#include <picotm/unistd.h>
#include <unistd.h>
#include "ptr.h"
#include "testhlp.h"

static size_t
MiB_to_bytes(unsigned short MiB)
{
    return 1024 * 1024 * MiB;
}

static const char g_test_str[] = "Hello world!\n";

static char g_filename[PATH_MAX];
static int  g_fildes = -1;

static const char*
temp_filename(unsigned long testnum, unsigned long filenum)
{
    int res = snprintf(g_filename, sizeof(g_filename),
                       "/tmp/fdio-test-%lu-%lu-XXXXXX",
                       testnum, filenum);
    if (res < 0) {
        perror("snprintf");
        abort();
    }
    const char* filename = mktemp(g_filename);
    if (!filename) {
        perror("mktemp");
        abort();
    }
    return filename;
}

static void
set_flags(int fildes, int flags, int get_cmd, int set_cmd)
{
    int current_flags = TEMP_FAILURE_RETRY(fcntl(fildes, get_cmd));
    if (current_flags < 0) {
        perror("fcntl");
        abort();
    }
    if (current_flags == flags) {
        return;
    }
    int res = TEMP_FAILURE_RETRY(fcntl(fildes, set_cmd, flags));
    if (res < 0) {
        perror("fcntl");
        abort();
    }
}

static void
set_fildes_flags(int fildes, int flags)
{
    set_flags(fildes, flags &= O_CLOEXEC, F_GETFD, F_SETFD);
}

static void
set_file_status_flags(int fildes, int flags)
{
    set_flags(fildes, flags &= ~O_CLOEXEC, F_GETFL, F_SETFL);
}

static int
temp_fildes(unsigned long testnum, unsigned long filenum, int flags)
{
    int res = snprintf(g_filename, sizeof(g_filename),
                       "/tmp/fdio-test-%lu-%lu-XXXXXX",
                       testnum, filenum);
    if (res < 0) {
        perror("snprintf");
        abort();
    }
    int fildes = mkstemp(g_filename);
    if (fildes < 0) {
        perror("mkstemp");
        abort();
    }
    if (flags) {
        set_fildes_flags(fildes, flags);
        set_file_status_flags(fildes, flags);
    }
    return fildes;
}

static void
close_fildes(int fildes)
{
    int res = TEMP_FAILURE_RETRY(close(fildes));
    if (res < 0) {
        perror("close");
        abort();
    }
}

static int
temp_fildes_gen(unsigned long testnum, unsigned long filenum, int flags,
                size_t filsiz, int32_t (*gen_i32)(void))
{
    int fildes = temp_fildes(testnum, filenum, 0);

    while (filsiz) {
        int32_t val = gen_i32();
        size_t count = filsiz > sizeof(val) ? sizeof(val) : filsiz;

        ssize_t res = TEMP_FAILURE_RETRY(write(fildes, &val, count));
        if (res < 0) {
            perror("write");
            abort();
        }
        filsiz -= res;
    }

    if (flags) {
        set_fildes_flags(fildes, flags);
        set_file_status_flags(fildes, flags);
    }

    return fildes;
}

static int32_t
gen_mrand48_i32(void)
{
    return mrand48();
}

static int
temp_fildes_rand(unsigned long testnum, unsigned long filenum, int flags,
                 size_t filsiz)
{
    srand48(0);
    return temp_fildes_gen(testnum, filenum, flags, filsiz, gen_mrand48_i32);
}

static int32_t
gen_zero_i32(void)
{
    return 0;
}

static int
temp_fildes_zero(unsigned long testnum, unsigned long filenum, int flags,
                 size_t filsiz)
{
    return temp_fildes_gen(testnum, filenum, flags, filsiz, gen_zero_i32);
}

static void
remove_file(const char* filename)
{
    int res = unlink(filename);
    if (res < 0) {
        perror("unlink");
        abort();
    }
}

/**
 * Open and close a file.
 */
void
fdio_test_1(unsigned int tid)
{
    picotm_begin

        int fildes = open_tx(g_filename,
                             O_WRONLY | O_CREAT,
                             S_IRWXU | S_IRWXG | S_IRWXO);

        delay_transaction(tid);

        close_tx(fildes);

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

void
fdio_test_1_pre(unsigned long nthreads, enum loop_mode loop,
                enum boundary_type btype, unsigned long long bound,
                int (*logmsg)(const char*, ...))
{
    temp_filename(1, 0);
}

void
fdio_test_1_post(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    remove_file(g_filename);
}

/**
 * Open-and-write test: Each thread open the file and writes a string.
 * Calling open_tx() with O_APPEND creates a load  dependency on the
 * file position, which is at the end of the file.
 */
void
fdio_test_2(unsigned int tid)
{
    picotm_begin

        int fildes = open_tx(g_filename,
                             O_WRONLY | O_CREAT | O_APPEND,
                             S_IRWXU | S_IRWXG | S_IRWXO);

        write_tx(fildes, g_test_str, strlen_tm(g_test_str));

        delay_transaction(tid);

        close_tx(fildes);

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

void
fdio_test_2_pre(unsigned long nthreads, enum loop_mode loop,
                enum boundary_type btype, unsigned long long bound,
                int (*logmsg)(const char*, ...))
{
    temp_filename(2, 0);
}

void
fdio_test_2_post(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    remove_file(g_filename);
}

/**
 * Each process writes to the beginning of a file. Only the last written
 * line should survive.
 */
void
fdio_test_3(unsigned int tid)
{
    picotm_begin

        int fildes = open_tx(g_filename,
                             O_WRONLY | O_CREAT,
                             S_IRWXU | S_IRWXG | S_IRWXO);

        lseek_tx(fildes, 0, SEEK_SET);
        write_tx(fildes, g_test_str, strlen_tm(g_test_str));

        /*delay_transaction(tid)*/

        close_tx(fildes);

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

void
fdio_test_3_pre(unsigned long nthreads, enum loop_mode loop,
                enum boundary_type btype, unsigned long long bound,
                int (*logmsg)(const char*, ...))
{
    temp_filename(3, 0);
}

void
fdio_test_3_post(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    remove_file(g_filename);
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
    picotm_begin

        int fildes = open_tx(g_filename,
                             O_WRONLY | O_CREAT,
                             S_IRWXU | S_IRWXG | S_IRWXO);

        pwrite_tx(fildes, g_test_str, strlen_tm(g_test_str), 2);

        /*delay_transaction(tid)*/

        close_tx(fildes);

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

void
fdio_test_4_pre(unsigned long nthreads, enum loop_mode loop,
                enum boundary_type btype, unsigned long long bound,
                int (*logmsg)(const char*, ...))
{
    temp_filename(4, 0);
}

void
fdio_test_4_post(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    remove_file(g_filename);
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

    picotm_begin

        int fildes = open_tx(g_filename,
                             O_RDWR | O_CREAT,
                             S_IRWXU | S_IRWXG | S_IRWXO);

        pwrite_tx(fildes, str, strlen_tm(g_test_str), 0);

        char rbuf[5]; /* 5 characters for reading 'Hello' */
        memset_tm(rbuf, 0, sizeof(rbuf)); /* Workaround valgrind */

        pread_tx(fildes, rbuf, sizeof(rbuf), 0);
        pwrite_tx(fildes, rbuf, sizeof(rbuf), 6);

        delay_transaction(tid);

        close_tx(fildes);

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

void
fdio_test_5_pre(unsigned long nthreads, enum loop_mode loop,
                enum boundary_type btype, unsigned long long bound,
                int (*logmsg)(const char*, ...))
{
    temp_filename(5, 0);
}

void
fdio_test_5_post(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    remove_file(g_filename);
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

    picotm_begin

        /* Open file */
        int fildes = open_tx(g_filename,
                             O_WRONLY | O_CREAT,
                             S_IRWXU | S_IRWXG | S_IRWXO);

        /* Do I/O */
        lseek_tx(fildes, 2, SEEK_CUR);
        write_tx(fildes, g_test_str, strlen(g_test_str));

        /* Write TID to the EOF */
        lseek_tx(fildes, 0, SEEK_END);
        write_tx(fildes, teststr, strlen_tm(teststr));

        /* Close */
        close_tx(fildes);

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

void
fdio_test_6_pre(unsigned long nthreads, enum loop_mode loop,
                enum boundary_type btype, unsigned long long bound,
                int (*logmsg)(const char*, ...))
{
    temp_filename(6, 0);
}

void
fdio_test_6_post(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    remove_file(g_filename);
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

    picotm_begin

        /* Open */
        int fildes = open_tx(g_filename,
                             O_RDWR | O_CREAT,
                             S_IRWXU | S_IRWXG | S_IRWXO);

        /* Do I/O */
        write_tx(fildes, g_test_str, strlen_tm(g_test_str));
        lseek_tx(fildes, 6, SEEK_SET);

        char buf[5];
        memset_tm(buf, 0, sizeof(buf)); /* Workaround valgrind */

        read_tx(fildes, buf, sizeof(buf));
        lseek_tx(fildes, 0, SEEK_SET);
        write_tx(fildes, buf, sizeof(buf));

        /* Write TID to the EOF */
        lseek_tx(fildes, 0, SEEK_END);
        write_tx(fildes, teststr, strlen_tm(teststr));

        /* Close */
        close_tx(fildes);

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

void
fdio_test_7_pre(unsigned long nthreads, enum loop_mode loop,
                enum boundary_type btype, unsigned long long bound,
                int (*logmsg)(const char*, ...))
{
    temp_filename(7, 0);
}

void
fdio_test_7_post(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    remove_file(g_filename);
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

    picotm_begin

        /* Open file */

        int fildes = open_tx(filename,
                             O_WRONLY | O_CREAT | O_TRUNC,
                             S_IRWXU | S_IRWXG | S_IRWXO);

        while (true) {

            char rbuf[1024];
            memset_tm(rbuf, 0, sizeof(rbuf)); /* Work around valgrind */

            /* Read from pipe; non-blocking I/O can signal error! */
            ssize_t res = read_tx(pfd[0], rbuf, sizeof(rbuf));
            if (res < 0) {
                if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                    /* Pipe empty; leave loop */
                    break;
                }
            }
            size_t rlen = res;

            /* Write to file */
            write_tx(fildes, rbuf, rlen);
        }

        close_tx(fildes);

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end

    /* Close pipe */

    for (size_t i = 0; i < arraylen(pfd); ++i) {
        close(pfd[i]);
    }
}

void
fdio_test_8_pre(unsigned long nthreads, enum loop_mode loop,
                enum boundary_type btype, unsigned long long bound,
                int (*logmsg)(const char*, ...))
{
    return;
}

void
fdio_test_8_post(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    for (unsigned long tid = 0; tid < nthreads; ++tid) {

        char filename[32];
        int res = snprintf(filename, sizeof(filename),
                           "/tmp/fdio-%lu.test", tid);
        if (res < 0) {
            perror("snprintf");
            abort();
        }
        remove_file(filename);
    }
}

/**
 * Write some lines at end of file. Should result in an integer record
 * for each thread. The order of records can vary.
 */
void
fdio_test_9(unsigned int tid)
{
    char str[100][256];

    for (char (*s)[256] = str; s < str + arraylen(str); ++s) {
        if (sprintf(*s, "%d line %d\n", tid, (int)(s - str)) < 0) {
            perror("snprintf");
            abort();
        }
    }

    picotm_begin

        lseek_tx(g_fildes, 0, SEEK_END);

        for (const char (*s)[256] = str; s < str + arraylen(str); ++s) {
            write_tx(g_fildes, *s, strlen_tm(*s));
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

void
fdio_test_9_pre(unsigned long nthreads, enum loop_mode loop,
                enum boundary_type btype, unsigned long long bound,
                int (*logmsg)(const char*, ...))
{
    g_fildes = temp_fildes(9, 0, O_CLOEXEC | O_WRONLY);
}

void
fdio_test_9_post(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    close_fildes(g_fildes);
    remove_file(g_filename);
}

/**
 * Write string, read-back and write again; should output 'Hello Hello!'.
 *
 * \todo Did not return.
 */
void
fdio_test_10(unsigned int tid)
{
    picotm_begin

        pwrite_tx(g_fildes, g_test_str, strlen_tm(g_test_str), 0);

        char rbuf[5];
        memset_tm(rbuf, 0, sizeof(rbuf)); /* Workaround valgrind */

        pread_tx(g_fildes, rbuf, sizeof(rbuf), 0);
        pwrite_tx(g_fildes, rbuf, sizeof(rbuf), 6);

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

void
fdio_test_10_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = temp_fildes(10, 0, O_CLOEXEC | O_RDWR);
}

void
fdio_test_10_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    close_fildes(g_fildes);
    remove_file(g_filename);
}

/**
 * Write string at position 2 using lseek; should output '  Hello world!'.
 */
void
fdio_test_11(unsigned int tid)
{
    char teststr[16];
    int res = snprintf(teststr, 15, "%d\n", (int)tid);
    if (res < 0) {
        perror("snprintf");
        abort();
    }

    picotm_begin

        lseek_tx(g_fildes, 0, SEEK_SET);

        /* Do I/O */
        lseek_tx(g_fildes, 2, SEEK_CUR);
        write_tx(g_fildes, g_test_str, strlen_tm(g_test_str));

        /* Write TID to the EOF */
        lseek_tx(g_fildes, 0, SEEK_END);
        write_tx(g_fildes, teststr, strlen_tx(teststr));

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

void
fdio_test_11_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = temp_fildes(11, 0, O_CLOEXEC | O_WRONLY);
}

void
fdio_test_11_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    close_fildes(g_fildes);
    remove_file(g_filename);
}

/**
 * Write string, read-back and write again; should output 'world world!'.
 */
void
fdio_test_12(unsigned int tid)
{
    char teststr[16];
    int res = snprintf(teststr, 15, "%d\n", (int)tid);
    if (res < 0) {
        perror("snprintf");
        abort();
    }

    picotm_begin

        /* Do I/O */
        lseek_tx(g_fildes, 0, SEEK_SET);
        write_tx(g_fildes, g_test_str, strlen_tm(g_test_str));
        lseek_tx(g_fildes, 6, SEEK_SET);

        char buf[5];
        memset_tm(buf, 0, sizeof(buf)); /* Workaround valgrind */

        read_tx(g_fildes, buf, sizeof(buf));
        lseek_tx(g_fildes, 0, SEEK_SET);
        write_tx(g_fildes, buf, sizeof(buf));

        /* Write TID to the EOF */
        lseek_tx(g_fildes, 0, SEEK_END);
        write_tx(g_fildes, teststr, strlen_tx(teststr));

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

void
fdio_test_12_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = temp_fildes(12, 0, O_CLOEXEC | O_RDWR);
}

void
fdio_test_12_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    close_fildes(g_fildes);
    remove_file(g_filename);
}

/**
 * Write to stdout to test unbuffered writes.
 */
void
fdio_test_13(unsigned int tid)
{
    char str[20][128];

    for (size_t i = 0; i < arraylen(str); ++i) {
        int res = snprintf(str[i], sizeof(str[i]), "%u %zu %s",
                           tid, i, g_test_str);
        if (res < 0) {
            perror("snprintf");
            abort();
        }
    }

    picotm_begin

        for (size_t i = 0; i < arraylen(str); ++i) {
            write_tx(STDOUT_FILENO, str[i], strlen_tx(str[i]));
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

/**
 * Write some lines at end of file. Should result in an integer record
 * for each thread. The order of records can vary. This tests the
 * optimization for non-depending accesses.
 */
void
fdio_test_14(unsigned int tid)
{
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

    picotm_begin

        /*lseek_tx(fildes, 0, SEEK_END);*/

        for (s = str; s < str+sizeof(str)/sizeof(str[0]); ++s) {
            write_tx(g_fildes, *s, strlen(*s));
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

void
fdio_test_14_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = temp_fildes(14, 0, O_CLOEXEC | O_WRONLY);
}

void
fdio_test_14_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    close_fildes(g_fildes);
    remove_file(g_filename);
}

/**
 * Open, dup and close a file descriptor.
 */
void
fdio_test_15(unsigned int tid)
{
    picotm_begin

        int fildes2 = dup_tx(g_fildes);

        delay_transaction(tid);

        close_tx(fildes2);

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

void
fdio_test_15_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = temp_fildes(14, 0, O_CLOEXEC | O_WRONLY);
}

void
fdio_test_15_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    close_fildes(g_fildes);
    remove_file(g_filename);
}

/**
 * Open, dup and close a file descriptor.
 */
void
fdio_test_16(unsigned int tid)
{
    extern enum picotm_libc_cc_mode g_cc_mode;
    picotm_libc_set_file_type_cc_mode(PICOTM_LIBC_FILE_TYPE_REGULAR, g_cc_mode);

    picotm_begin

        int fildes = open_tx(g_filename,
                             O_WRONLY | O_CREAT,
                             S_IRWXU | S_IRWXG | S_IRWXO);

        int fildes2 = dup_tx(fildes);

        delay_transaction(tid);

        close_tx(fildes2);
        close_tx(fildes);

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

void
fdio_test_16_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    temp_filename(16, 0);
}

void
fdio_test_16_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    remove_file(g_filename);
}

/**
 * Open, dup, write, and close a file descriptor.
 */
void
fdio_test_17(unsigned int tid)
{
    extern enum picotm_libc_cc_mode g_cc_mode;
    picotm_libc_set_file_type_cc_mode(PICOTM_LIBC_FILE_TYPE_REGULAR, g_cc_mode);

    char str[2][128];

    for (char (*s)[128] = str; s < str + arraylen(str); ++s) {
        int res = snprintf(*s, sizeof(str[0]), "%u %lu %s", tid,
                           (unsigned long)(s - str), g_test_str);
        if (res < 0) {
            perror("snprintf");
            abort();
        }
    }

    picotm_begin

        int fildes = open_tx(g_filename,
                             O_WRONLY | O_CREAT,
                             S_IRWXU | S_IRWXG | S_IRWXO);

        int fildes2 = dup_tx(fildes);

        lseek_tx(fildes, 0, SEEK_END);
        lseek_tx(fildes2, 0, SEEK_END);
        write_tx(fildes, str[0], strlen_tx(str[0]));
        write_tx(fildes2, str[1], strlen_tx(str[1]));

        close_tx(fildes2);
        close_tx(fildes);

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

void
fdio_test_17_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    temp_filename(17, 0);
}

void
fdio_test_17_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    remove_file(g_filename);
}

/**
 * Open, dup, write, and close a file descriptor.
 */
void
fdio_test_18(unsigned int tid)
{
    extern enum picotm_libc_cc_mode g_cc_mode;
    picotm_libc_set_file_type_cc_mode(PICOTM_LIBC_FILE_TYPE_REGULAR, g_cc_mode);

    char str[2][128];

    for (char (*s)[128] = str; s < str + arraylen(str); ++s) {
        int res = snprintf(*s, sizeof(str[0]), "%u %lu %s", tid,
                           (unsigned long)(s - str), g_test_str);
        if (res < 0) {
            perror("snprintf");
            abort();
        }
    }

    picotm_begin

        int fildes2 = dup_tx(g_fildes);

        lseek_tx(g_fildes, 0, SEEK_END);
        lseek_tx(fildes2, 0, SEEK_END);
        write_tx(g_fildes, str[0], strlen_tx(str[0]));
        write_tx(fildes2, str[1], strlen_tx(str[1]));

        close_tx(fildes2);

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

void
fdio_test_18_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = temp_fildes(18, 0, O_CLOEXEC);
}

void
fdio_test_18_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    close_fildes(g_fildes);
    remove_file(g_filename);
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
        picotm_begin

            char rbuf[10];
            memset_tm(rbuf, 0, sizeof(rbuf)); /* Work around valgrind */

            /* Read from pipe */
            ssize_t res = read_tx(pfd[0], rbuf, sizeof(rbuf));
            if (res < 0) {
                if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                    /* Pipe empty */
                    res = 0;
                }
            }
            size_t tx_rlen = res;

            /* Write to stdout */
            write_tx(STDOUT_FILENO, rbuf, tx_rlen);

            store_size_t_tx(&rlen, tx_rlen);

        picotm_commit

            abort_transaction_on_error(__func__);

        picotm_end

    } while (rlen);

    /* Close pipe */
    /*for (i = 0; i < sizeof(pfd)/sizeof(pfd[0]); ++i) {
        if (TEMP_FAILURE_RETRY(close(pfd[i])) < 0) {
            perror("close");
            abort();
        }
    }*/
}

void
fdio_test_20_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    extern enum picotm_libc_cc_mode g_cc_mode;
    picotm_libc_set_file_type_cc_mode(PICOTM_LIBC_FILE_TYPE_REGULAR, g_cc_mode);

    g_fildes = temp_fildes_rand(20, 0, O_CLOEXEC | O_RDWR, MiB_to_bytes(1));
}

void
fdio_test_20_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    remove_file(g_filename);
    close_fildes(g_fildes);
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

    picotm_begin

        off_t tx_offset = load_off_t_tx(&offset);

        unsigned char buf[24];
        ssize_t count = pread_tx(g_fildes, buf, sizeof(buf), tx_offset);

        pwrite_tx(g_fildes, buf, count, offset);

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

/*
 * Random read/write
 */

int
tx_random_rw_pre(unsigned long testnum, unsigned long filenum, size_t filsiz)
{
    extern enum picotm_libc_cc_mode g_cc_mode;
    picotm_libc_set_file_type_cc_mode(PICOTM_LIBC_FILE_TYPE_REGULAR, g_cc_mode);

    return temp_fildes_rand(testnum, filenum, O_CLOEXEC | O_RDWR, filsiz);
}

void
tx_random_rw(int fildes, unsigned int* seed, off_t size,
             unsigned long ncycles, unsigned long nreads)
{
    picotm_begin

        off_t tx_size = load_off_t_tx(&size);
        unsigned long tx_ncycles = load_ulong_tx(&ncycles);
        unsigned long tx_nreads = load_ulong_tx(&nreads);

        for (unsigned long i = 0; i < tx_ncycles; ++i) {

            unsigned char buf[24];
            off_t offset = 0;
            size_t count = 0;

            for (unsigned long j = 0; j < tx_nreads; ++j) {
                offset = rand_r_tx(seed) % tx_size;
                count = pread_tx(fildes, buf, sizeof(buf), offset);
            }

            pwrite_tx(fildes, buf, count, offset);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

int
locked_random_rw_pre(unsigned long testnum, unsigned long filenum,
                     size_t filsiz)
{
    return temp_fildes_rand(testnum, filenum, O_CLOEXEC | O_RDWR, filsiz);
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
    remove_file(g_filename);
    close_fildes(fildes);
}

/*
 * Random read/write, ratio 1:1
 */

void
fdio_test_21_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = tx_random_rw_pre(21, 0, MiB_to_bytes(1));
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
    g_fildes = locked_random_rw_pre(22, 0, MiB_to_bytes(1));
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
    g_fildes = tx_random_rw_pre(23, 0, MiB_to_bytes(1));
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
    g_fildes = locked_random_rw_pre(24, 0, MiB_to_bytes(1));
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
    g_fildes = tx_random_rw_pre(25, 0, MiB_to_bytes(1));
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
    g_fildes = locked_random_rw_pre(26, 0, MiB_to_bytes(1));
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
    g_fildes = tx_random_rw_pre(27, 0, MiB_to_bytes(1));
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
    g_fildes = locked_random_rw_pre(28, 0, MiB_to_bytes(1));
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
tx_random_read_pre(unsigned long testnum, unsigned long filenum,
                   size_t filsiz)
{
    extern enum picotm_libc_cc_mode g_cc_mode;
    picotm_libc_set_file_type_cc_mode(PICOTM_LIBC_FILE_TYPE_REGULAR, g_cc_mode);

    return temp_fildes_rand(testnum, filenum, O_CLOEXEC | O_RDONLY, filsiz);
}

static void
tx_random_read(int fildes, unsigned int* seed, off_t size,
               unsigned long ncycles)
{
    picotm_begin

        size_t tx_cycles = load_ulong_tx(&ncycles);

        for (size_t i = 0; i < tx_cycles; ++i) {

            off_t offset = rand_r_tx(seed) % size;

            unsigned char buf[24];
            pread_tx(fildes, buf, sizeof(buf), offset);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

static int
locked_random_read_pre(unsigned long testnum, unsigned long filenum,
                       size_t filsiz)
{
    return temp_fildes_rand(testnum, filenum, O_CLOEXEC | O_RDONLY, filsiz);
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
    close_fildes(fildes);
    remove_file(g_filename);
}

void
fdio_test_29_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = tx_random_read_pre(29, 0, MiB_to_bytes(1));
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
    g_fildes = locked_random_read_pre(30, 0, MiB_to_bytes(1));
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
tx_random_write_pre(unsigned long testnum, unsigned long filenum,
                    size_t filsiz)
{
    extern enum picotm_libc_cc_mode g_cc_mode;
    picotm_libc_set_file_type_cc_mode(PICOTM_LIBC_FILE_TYPE_REGULAR, g_cc_mode);

    return temp_fildes_zero(testnum, filenum, O_CLOEXEC | O_WRONLY, filsiz);
}

static void
tx_random_write(int fildes, unsigned int* seed, off_t size, unsigned long ncycles)
{
    unsigned char buf[24];
    memset(buf, 0, sizeof(buf));

    picotm_begin

        unsigned long tx_cycles = load_ulong_tx(&ncycles);

        for (unsigned long i = 0; i < tx_cycles; ++i) {

            off_t offset = rand_r_tx(seed) % size;

            pwrite_tx(fildes, buf, sizeof(buf), offset);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

static int
locked_random_write_pre(unsigned long testnum, unsigned long filenum,
                        size_t filsiz)
{
    return temp_fildes_zero(testnum, filenum, O_CLOEXEC | O_WRONLY, filsiz);
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
    close_fildes(fildes);
    remove_file(g_filename);
}

void
fdio_test_31_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = tx_random_write_pre(31, 0, MiB_to_bytes(1));
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
    g_fildes = locked_random_write_pre(32, 0, MiB_to_bytes(1));
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

    locked_random_write(&s_lock, g_fildes, &t_seed, 1024 * 1024, g_txcycles);
}

void
fdio_test_32_post(unsigned long nthreads, enum loop_mode loop,
                  enum boundary_type btype, unsigned long long bound,
                  int (*logmsg)(const char*, ...))
{
    random_write_post(g_fildes);
}

/*
 * Sequential reads
 */

static int
tx_seq_read_pre(unsigned long testnum, unsigned long filenum,
                size_t filsiz)
{
    extern enum picotm_libc_cc_mode g_cc_mode;
    picotm_libc_set_file_type_cc_mode(PICOTM_LIBC_FILE_TYPE_REGULAR, g_cc_mode);

    return temp_fildes_rand(testnum, filenum, O_CLOEXEC | O_RDONLY, filsiz);
}

static void
tx_seq_read(int fildes, unsigned int* seed, off_t size, unsigned long ncycles)
{
    off_t offset = rand_r(seed) % size;

    picotm_begin

        off_t pos = load_off_t_tx(&offset);
        unsigned long cycles = load_ulong_tx(&ncycles);

        for (unsigned long i = 0; i < cycles; ++i) {
            unsigned char buf[24];
            pos += pread_tx(fildes, buf, sizeof(buf), pos);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

static int
locked_seq_read_pre(unsigned long testnum, unsigned long filenum,
                    size_t filsiz)
{
    return temp_fildes_rand(testnum, filenum, O_CLOEXEC | O_RDONLY, filsiz);
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
    close_fildes(fildes);
    remove_file(g_filename);
}

void
fdio_test_33_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = tx_seq_read_pre(33, 0, MiB_to_bytes(1));
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
    g_fildes = locked_seq_read_pre(34, 0, MiB_to_bytes(1));
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
 * Sequential writes
 */

static int
tx_seq_write_pre(unsigned long testnum, unsigned long filenum,
                 size_t fildes)
{
    extern enum picotm_libc_cc_mode g_cc_mode;
    picotm_libc_set_file_type_cc_mode(PICOTM_LIBC_FILE_TYPE_REGULAR, g_cc_mode);

    return temp_fildes_zero(testnum, filenum, O_CLOEXEC | O_WRONLY, fildes);
}

static void
tx_seq_write(int fildes, unsigned int* seed, off_t size, unsigned long ncycles)
{
    unsigned char buf[24];
    memset(buf, 0, sizeof(buf));

    off_t offset = rand_r(seed) % size;

    picotm_begin

        off_t pos = load_off_t_tx(&offset);
        unsigned long tx_ncycles = load_ulong_tx(&ncycles);

        for (unsigned long i = 0; i < tx_ncycles; ++i) {
            pos += pwrite_tx(fildes, buf, sizeof(buf), pos);
        }

    picotm_commit

        abort_transaction_on_error(__func__);

    picotm_end
}

static int
locked_seq_write_pre(unsigned long testnum, unsigned long filenum,
                     size_t filsiz)
{
    return temp_fildes_zero(testnum, filenum, O_CLOEXEC | O_WRONLY, filsiz);
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
    close_fildes(fildes);
    remove_file(g_filename);
}

void
fdio_test_35_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = tx_seq_write_pre(35, 0, MiB_to_bytes(1));
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
    locked_seq_write_pre(36, 0, MiB_to_bytes(1));
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
    g_fildes = tx_random_rw_pre(37, 0, MiB_to_bytes(100));
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
    g_fildes = locked_random_rw_pre(38, 0, MiB_to_bytes(100));
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
    g_fildes = tx_random_rw_pre(39, 0, MiB_to_bytes(100));
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
    g_fildes = locked_random_rw_pre(40, 0, MiB_to_bytes(100));
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
    g_fildes = tx_random_rw_pre(41, 0, MiB_to_bytes(100));
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
    g_fildes = locked_random_rw_pre(42, 0, MiB_to_bytes(100));
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
    g_fildes = tx_random_rw_pre(43, 0, MiB_to_bytes(100));
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
    g_fildes = locked_random_rw_pre(43, 0, MiB_to_bytes(100));
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
    g_fildes = tx_random_read_pre(45, 0, MiB_to_bytes(100));
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
    g_fildes = locked_random_read_pre(46, 0, MiB_to_bytes(100));
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
    g_fildes = tx_random_write_pre(47, 0, MiB_to_bytes(100));
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
    g_fildes = locked_random_write_pre(48, 0, MiB_to_bytes(100));
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
 * Sequential reads
 */

void
fdio_test_49_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = tx_seq_read_pre(49, 0, MiB_to_bytes(100));
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
    g_fildes = locked_seq_read_pre(50, 0, MiB_to_bytes(100));
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
 * Sequential writes
 */

void
fdio_test_51_pre(unsigned long nthreads, enum loop_mode loop,
                 enum boundary_type btype, unsigned long long bound,
                 int (*logmsg)(const char*, ...))
{
    g_fildes = tx_seq_write_pre(51, 0, MiB_to_bytes(100));
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
    g_fildes = locked_seq_write_pre(52, 0, MiB_to_bytes(100));
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
