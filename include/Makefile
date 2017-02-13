
.PHONY = ceuta

CEUTA_HEADERS = tanger-stm-malloc.h \
				tanger-stm-std-complex.h \
				tanger-stm-std-errno.h \
				tanger-stm-std-fcntl.h \
				tanger-stm-std-math.h \
				tanger-stm-std-pthread.h \
				tanger-stm-std-sched.h \
				tanger-stm-std-stdio.h \
				tanger-stm-std-stdlib.h \
				tanger-stm-std-string.h \
				tanger-stm-std-sys-select.h \
				tanger-stm-std-sys-stat.h \
				tanger-stm-std-sys-socket.h \
				tanger-stm-std-unistd.h

ceuta-clean:
	rm -f $(CEUTA_HEADERS)

ceuta: $(CEUTA_HEADERS)

tanger-stm-std-complex.h : ../lib/libm/std-complex.cu
	../tools/ceuta -h $< > $@

tanger-stm-std-math.h : ../lib/libm/std-math.cu
	../tools/ceuta -h $< > $@

tanger-stm-std-pthread.h : ../lib/libpthread/std-pthread.cu
	../tools/ceuta -h $< > $@

tanger-stm-%.h : ../lib/libc/%.cu
	../tools/ceuta -h $< > $@

