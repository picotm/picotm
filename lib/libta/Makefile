
include ../Makefile.common

TANGERINC ?= -I../../../../tanger/tdz/include
LIBTAINC ?= -I../../include

CFLAGS += $(LIBTAINC) $(TANGERINC)

SRCS = $(wildcard *.c)

all: libta.bc

libta.bc: $(SRCS:.c=.bc)
	$(LLVMLD) $(LLVMLDFLAGS) -o $@ $^

