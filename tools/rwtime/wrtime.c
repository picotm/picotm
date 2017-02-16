/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

typedef unsigned long long ticks;
static __inline__ ticks getticks(void)
{
     unsigned a, d;
     __asm__ volatile("rdtsc" : "=a" (a), "=d" (d));
     return ((ticks)a) | (((ticks)d) << 32);
}

/* Returns the number of milliseconds since the epoch */
static unsigned long long
getmsofday(void *tzp)
{
    struct timeval t;
    gettimeofday(&t, tzp);

    return t.tv_sec*1000 + t.tv_usec/1000;
}

int
main(int argc, char *argv[])
{
    int fildes;
    char buf[2400];
    int i;

    memset(buf, 0, sizeof(buf));

    fildes = open("/var/tmp/tdz/wrtime.txt", O_WRONLY);

    for (i = 0; i <= 100; i += 10) {

        ticks pread_ticks = 0;
        unsigned long long pread_count = 0;

        unsigned long long begin = getmsofday(NULL);

        do {
            ticks t0 = getticks();
            ssize_t len = pwrite(fildes, buf, 24*i, i*1000);
            ticks t1 = getticks();

            if (len < 0) {
                perror("pwrite");
            }

            pread_ticks += t1-t0;
            ++pread_count;

        } while ((getmsofday(NULL)-begin) < 60000);

        printf("%d %ld\n", (int)(24*i), (long)(pread_ticks/pread_count));
    }

    exit(EXIT_SUCCESS);
}

