
.PHONY = all clean

all : rdtime wrtime

rdtime : rdtime.o
	gcc -g3 -o $@ $^  -lc

wrtime : wrtime.o
	gcc -g3 -o $@ $^  -lc

%.o: %.c
	gcc -Wall -D_GNU_SOURCE -c -g3 -O3 -o $@ $<

clean:
	rm -fr *.o
	rm -fr rdtime
	rm -fr wrtime

