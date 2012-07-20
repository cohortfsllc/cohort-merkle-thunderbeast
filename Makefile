CC=g++
CFLAGS=-I. -std=c++0x
LDFLAGS=-lcrypto

HEADERS=hashtree.h hash_file.h hasher.h updater.h

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
