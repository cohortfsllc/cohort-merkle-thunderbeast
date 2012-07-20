CC=g++
CFLAGS=-I. -std=c++0x
LDFLAGS=-lcrypto

HEADERS=hash_tree.h hash_file.h hasher.h updater.h block_reader.h

%.o: %.cpp $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: merkle runtests

merkle: merkle.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

runtests: test.o
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

test: runtests
	./runtests

clean:
	rm merkle runtests *.o
