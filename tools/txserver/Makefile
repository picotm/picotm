
.PHONY = all clean proto

txserver : base64.o connection.o lex.txproto.c main.o server.o txproto.tab.o
	gcc -g3 -o $@ $^ -lc

%.o: %.c
	gcc -Wall -D_GNU_SOURCE -c -g3 -o $@ $<

all : txserver

clean:
	rm -fr *.tab.?
	rm -fr lex.*.?
	rm -fr *.o
	rm -fr txserver

proto: txproto.tab.c lex.txproto.c

%.tab.c: %.y
	bison -t --defines --file-prefix=txproto --name-prefix=txproto $<

lex.%.c: %.l
	flex --prefix=txproto --bison-bridge $<

