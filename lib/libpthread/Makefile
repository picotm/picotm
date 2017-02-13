
include ../Makefile.common

.PHONY += ceuta ceuta-clean

TANGERINC ?= -I../../../../tanger/tdz/include
LIBTAPTHREADINC ?= -I../libta -I../../include

CFLAGS += $(LIBTAPTHREADINC) $(TANGERINC)

SRCS = $(wildcard *.c)

CEUTASRCS = ceuta-std-pthread.c

all: libtapthread.bc

libtapthread.bc: $(SRCS:.c=.bc)
	$(LLVMLD) $(LLVMLDFLAGS) -o $@ $^

ceuta: $(CEUTASRCS)

ceuta-%.c: %.cu
	../../tools/ceuta $< > $@

ceuta-clean:
	rm -f ceuta-*.c

