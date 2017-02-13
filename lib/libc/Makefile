
include ../Makefile.common

.PHONY += ceuta ceuta-clean

TANGERINC ?= -I../../../../tanger/tdz/include
LIBTACINC ?= -I../libta -I../../include

CFLAGS += $(LIBTACINC) $(TANGERINC)

CEUTASRCS = ceuta-malloc.c \
			ceuta-std-errno.c \
			ceuta-std-fcntl.c \
			ceuta-std-sched.c \
			ceuta-std-stdio.c \
			ceuta-std-stdlib.c \
			ceuta-std-string.c \
			ceuta-std-string.c \
			ceuta-std-sys-select.c \
			ceuta-std-sys-stat.c \
			ceuta-std-sys-socket.c \
			ceuta-std-unistd.c

SRCS = $(wildcard *.c)

all: libtac.bc

libtac.bc: $(SRCS:.c=.bc)
	$(LLVMLD) $(LLVMLDFLAGS) -o $@ $(SRCS:.c=.bc) \
	fd/libtac_fd.bc \
	fs/libtac_fs.bc \
	alloc/libtac_alloc.bc \
	txproto/libtac_txproto.bc

ceuta: $(CEUTASRCS)

ceuta-%.c: %.cu
	../../tools/ceuta $< > $@

ceuta-clean:
	rm -f ceuta-*.c

