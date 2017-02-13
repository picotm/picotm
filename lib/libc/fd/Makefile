
include ../../Makefile.common

TANGERINC ?= -I../../../../../tanger/tdz/include
LIBTACINC ?= -I.. -I../txproto -I../../libta -I../../../include

CFLAGS += $(LIBTACINC) $(TANGERINC)

SRCS = $(wildcard *.c)

all: libtac_fd.bc

libtac_fd.bc: $(SRCS:.c=.bc)
	$(LLVMLD) $(LLVMLDFLAGS) -o $@ $(SRCS:.c=.bc)

