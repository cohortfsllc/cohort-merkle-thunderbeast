CC=g++
CFLAGS=-I. -std=c++0x
LDFLAGS=-lcrypto

HEADERS=block_reader.h hash_file.h hash_tree.h hasher.h updater.h visitor.h
OBJ=updater.o visitor.o

%.o: %.cpp $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

merkle: merkle.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

runtests: test.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

test: runtests
	./runtests

clean:
	rm merkle runtests *.o
