# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

bin_PROGRAMS = tlctest

tlctest_SOURCES = main.c \
                  malloc_test.c \
                  malloc_test.h \
                  ptr.h \
                  test.c \
                  test.h \
                  testhlp.c \
                  testhlp.h \
                  tm_test.c \
                  tm_test.h

CC = $(PTHREAD_CC)

AM_CFLAGS = $(PTHREAD_CFLAGS)

AM_LDFLAGS = $(PTHREAD_LDFLAGS)

LDADD = $(PTHREAD_LIBS)
