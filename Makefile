CC=g++
CFLAGS=-I. -std=c++0x
LDFLAGS=-lcrypto

HEADERS=hashtree.h hash_file.h hasher.h updater.h
OBJS=test.o

%.o: %.cpp $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

runtests: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

test: runtests
	./runtests

clean:
	rm runtests *.o
