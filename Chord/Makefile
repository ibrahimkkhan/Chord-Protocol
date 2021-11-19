CC=g++
CFLAGS=-Wall -Iincludes -Wextra -std=gnu99 -ggdb
LDLIBS=-lprotobuf-c -lcrypto
# all: rpc hashtest chord
all: hashtest chord

# rpc: rpc.c rpc.pb-c.c

# rpc.pb-c.c: rpc.proto
# 	protoc-c --c_out=. $<

hashtest: hash.o hashtest.o

chord: hash.o chord.o
# clean: rm -rf *~ *.o rpc rpc.pb-c.c rpc.pb-c.h hashtest
clean:
	rm -rf *~ *.o hashtest chord

.PHONY : clean all
