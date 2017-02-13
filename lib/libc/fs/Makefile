
include ../../Makefile.common

TANGERINC ?= -I../../../../../tanger/tdz/include
LIBTACINC ?= -I.. -I../../libta -I../../../include

CFLAGS += $(LIBTACINC) $(TANGERINC)

SRCS = $(wildcard *.c)

all: libtac_fs.bc

libtac_fs.bc: $(SRCS:.c=.bc)
	$(LLVMLD) $(LLVMLDFLAGS) -o $@ $(SRCS:.c=.bc)

