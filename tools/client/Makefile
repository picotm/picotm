
.PHONY = all clean

all : client

client : client.o main.o
	gcc -g3 -o $@ $^  -lc

%.o: %.c
	gcc -Wall -D_GNU_SOURCE -c -g3 -o $@ $<

clean:
	rm -fr *.o
	rm -fr client

