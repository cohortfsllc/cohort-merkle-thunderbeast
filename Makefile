CC=g++
CFLAGS=-I. -std=c++0x
LDFLAGS=-lcrypto

HEADERS=block_reader.h hash_file.h hash_tree.h hasher.h updater.h visitor.h
OBJ=updater.o verifier.o visitor.o

%.o: %.cpp $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: merkle runtests

merkle: merkle.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

runtests: test.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

test: runtests
	./runtests

clean:
	rm -f merkle runtests *.o
