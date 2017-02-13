
.PHONY = all clean

all : server

server : main.o server.o
	gcc -g3 -o $@ $^  -lc

%.o: %.c
	gcc -Wall -D_GNU_SOURCE -c -g3 -o $@ $<

clean:
	rm -fr *.o
	rm -fr server

