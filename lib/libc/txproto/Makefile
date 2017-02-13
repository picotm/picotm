
include ../../Makefile.common

.PHONY += proto

TANGERINC ?= -I../../../../../tanger/tdz/include
LIBTACINC ?= -I.. -I../../libta -I../../../include

CFLAGS += $(LIBTACINC) $(TANGERINC)

SRCS = $(wildcard *.c)

all: libtac_txproto.bc

libtac_txproto.bc: lex.txproto.c txproto.tab.c $(SRCS:.c=.bc)
	$(LLVMLD) $(LLVMLDFLAGS) -o $@ $(SRCS:.c=.bc)

proto: txproto.tab.c lex.txproto.c

%.tab.c: %.y
	bison -t --defines --file-prefix=txproto --name-prefix=txproto $<

lex.%.c: %.l
	flex --prefix=txproto --bison-bridge $<

