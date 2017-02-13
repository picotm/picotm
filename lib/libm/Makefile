
include ../Makefile.common

.PHONY += ceuta ceuta-clean

TANGERINC ?= -I../../../../tanger/tdz/include
LIBTAMINC ?= -I../libta -I../../include

CFLAGS += $(LIBTAMINC) $(TANGERINC)

SRCS = $(wildcard *.c)

CEUTASRCS = ceuta-std-complex.c \
			ceuta-std-math.c

all: libtam.bc

libtam.bc: $(SRCS:.c=.bc)
	$(LLVMLD) $(LLVMLDFLAGS) -o $@ $^

ceuta: $(CEUTASRCS)

ceuta-%.c: %.cu
	../../tools/ceuta $< > $@

ceuta-clean:
	rm -f ceuta-*.c

