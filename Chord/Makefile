CC=gcc
CFLAGS=-Wall -Iincludes -Wextra -std=gnu99 -ggdb
LDLIBS=-lprotobuf-c -lcrypto

all: rpc hashtest

rpc: rpc.c rpc.pb-c.c

rpc.pb-c.c: rpc.proto
	protoc-c --c_out=. $<

hashtest: hash.o hashtest.o

clean:
	rm -rf *~ *.o rpc rpc.pb-c.c rpc.pb-c.h hashtest

.PHONY : clean all
