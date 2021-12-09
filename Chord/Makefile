CC=g++
CFLAGS=-Wall -Iincludes -Wextra -std=gnu99 -ggdb
LDLIBS=-lprotobuf -lcrypto

all: chord hashtest

# rpc: rpc.c rpc.pb-c.c

chord: ./chord.o ./chord.pb.cc ./hash.o

chord.pb.cc: chord.proto
	protoc -I./ --cpp_out=. $<

hashtest: hash.o hashtest.o

clean:
	rm -rf *~ *.o rpc rpc.pb-c.c rpc.pb-c.h hashtest

.PHONY : clean all
