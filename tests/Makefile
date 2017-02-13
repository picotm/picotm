# Defines the executables to be built
APPLICATIONS = tlctest

#
# Optimization easily breaks the application, so any optimization
# has been disabled
#

MAKEFILEREL     ?= .
#STMLIBDIR       ?= $(MAKEFILEREL)/../lib/stmdummy
STMLIBDIR       ?= ../../../tinySTM/tdz/lib/tanger-wt
SEQUENTIALLIB   ?= ../lib/stmdummy/no-tanger.bc
LIBTAPTHREADDIR ?= ../lib/libpthread
LIBTAMDIR       ?= ../lib/libm
LIBTACDIR       ?= ../lib/libc
LIBTADIR        ?= ../lib/libta
LLVMBUILD       ?= $(shell llvm-config --build-mode)
TANGERDIR       ?= ../../../tanger/tdz
TANGERPASS      ?= $(TANGERDIR)/$(LLVMBUILD)/lib/libtanger.so
TANGERINCLUDE   ?= $(TANGERDIR)/include
TAGLIBCINCLUDE  ?= ../include
INCLUDE         += -I$(TAGLIBCINCLUDE) -I$(TANGERINCLUDE)
EXTRAPASSES     ?=
PASSESTORUN     ?= -tanger
PASSESTORUNPOST ?=
NATIVELIBSPRE   ?=
NATIVELIBS      += -lm -lpthread
BASELINKOPTS    ?=
NATIVELINKOPTS  ?= 
OPTFLAGS         =  \
          -fdefer-pop \
          -fguess-branch-probability \
          -fcprop-registers \
          -fif-conversion \
          -fif-conversion2 \
          -ftree-ccp \
          -ftree-dce \
          -ftree-dominator-opts \
          -ftree-dse \
          -ftree-ter \
          -ftree-lrs \
          -ftree-sra \
          -ftree-copyrename \
          -ftree-fre \
          -ftree-ch \
          -funit-at-a-time \
          -fmerge-constants \
          -fomit-frame-pointer \
          -fthread-jumps \
          -fcrossjumping \
          -foptimize-sibling-calls \
          -fcse-follow-jumps  -fcse-skip-blocks \
          -fgcse  -fgcse-lm  \
          -fexpensive-optimizations \
          -frerun-cse-after-loop  \
          -fcaller-saves \
          -fpeephole2 \
          -fschedule-insns  -fschedule-insns2 \
          -fsched-interblock  -fsched-spec \
          -fregmove \
          -fstrict-aliasing -fstrict-overflow \
          -fdelete-null-pointer-checks \
          -freorder-blocks  -freorder-functions \
          -falign-functions  -falign-jumps \
          -falign-loops  -falign-labels \
          -ftree-vrp \
          -ftree-pre \
          -finline-functions -funswitch-loops -fgcse-after-reload
          
HEADERS         ?= $(wildcard $(INCLUDE)/*.h)
CPPFLAGS        += -D_GNU_SOURCE=1
CFLAGS          += -Wall -O0 -pthread -g -pg -fno-builtin -fnested-functions -std=c99
CXXFLAGS        += -Wall -O0 -pthread -g -pg -fno-builtin -fnested-functions
#CPPFLAGS        += -D_GNU_SOURCE=1 -DNDEBUG=1
#CFLAGS          += -Wall $(OPTFLAGS) -pthread -fno-inline -fno-builtin -fnested-functions -std=c99
#CXXFLAGS        += -Wall $(OPTFLAGS) -pthread -fno-inline -fno-builtin -fnested-functions
LINKOPTS        ?=
OPT             ?= opt
LLVMLD          ?= llvm-ld
LLVMGCC         ?= llvm-gcc
LLVMGXX         ?= llvm-g++
LLVMDIS         ?= llvm-dis
LLC             ?= llc
GCC             ?= gcc
# Set this to enable debug output and statistics for the LLVM pass
#TANGERPASSOPTS ?= -debug -stats

.PHONY: all clean

all: $(APPLICATIONS)

# native binary from .bc:
$(APPLICATIONS): %: %-tangerstm.bc $(STMLIBDIR)/libtanger-stm.a
	$(LLC) -f -o $(patsubst %.bc, %.s, $<) $<
	$(GCC) $(CFLAGS) -o $@ $(patsubst %.bc, %.s, $<) $(NATIVELIBSPRE) $(NATIVELIBS) $(NATIVELINKOPTS) 

# link in .bc parts of STM
tlctest-tangerstm.bc: tlctest-tanger.bc $(STMLIBDIR)/libtanger-stm.bc
	$(LLVMLD) -disable-opt -link-as-library -o $@ $< $(STMLIBDIR)/libtanger-stm.bc $(LIBTAPTHREADDIR)/libtapthread.bc $(LIBTAMDIR)/libtam.bc $(LIBTACDIR)/libtac.bc $(LIBTADIR)/libta.bc
#	$(LLVMLD) -link-as-library -o $@ $< $(STMLIBDIR)/libtanger-stm.bc $(LIBTAPTHREADDIR)/libtapthread.bc $(LIBTAMDIR)/libtam.bc $(LIBTACDIR)/libtac.bc $(LIBTADIR)/libta.bc

# run pass over combined
tlctest-tanger.bc: tlctest-tangerbase.bc $(TANGERPASS)
	$(OPT) $(EXTRAPASSES) -load $(TANGERPASS) -lowerallocs -disable-opt -disable-inlining \
		$(PASSESTORUN) $< -o $@ -f $(TANGERPASSOPTS)
#	$(OPT) $(EXTRAPASSES) -load $(TANGERPASS) -mem2reg -lowerallocs -globaldce -deadargelim \
#		$(PASSESTORUN) $< -o $@ -f $(TANGERPASSOPTS)

# .bc from sources
%.bc: %.c $(HEADERS)
	$(LLVMGCC) $(INCLUDE) $(CPPFLAGS) $(CFLAGS) -emit-llvm -c -o $@ $<
%.bc: %.cpp $(HEADERS)
	$(LLVMGXX) $(INCLUDE) $(CPPFLAGS) $(CXXFLAGS) -emit-llvm -c -o $@ $<

# disassemble all .bc
dis: $(patsubst %.bc, %.ll, $(wildcard *.bc)) \
    $(patsubst %-tangerbase.bc, %-tanger.ll, $(wildcard *.bc))
%.ll: %.bc
	$(LLVMDIS) $< -f -o $@

# create sequential versions (use with care!)
seq: $(patsubst %, %-sequential, $(APPLICATIONS))

$(patsubst %, %-sequential, $(APPLICATIONS)): %-sequential: %-tangerbase.bc $(SEQUENTIALLIB)
	$(LLVMLD) -disable-opt -link-as-library -o $@.bc $^
#	$(LLVMLD) -link-as-library -o $@.bc $^
	$(LLC) -f -o $@.s $@.bc
	$(GCC) $(CFLAGS) -o $@ $@.s $(NATIVELIBS) $(NATIVELINKOPTS)

clean:
	rm -f *.bc *.ll *.s $(APPLICATIONS)

tlctest-tangerbase.bc: error_test.bc fdio_test.bc fs_test.bc main.bc malloc_test.bc socket_test.bc
	$(LLVMLD) -link-as-library -disable-opt -disable-inlining -o $@ $^
#	$(LLVMLD) -link-as-library -o $@ $^

