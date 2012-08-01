CC=gcc
CFLAGS=-I. -Wall -g -ggdb
LDFLAGS=-lcrypto -lm

HEADERS=merkle.h tree.h visitor.h update.h
OBJ=truncate.o update.o verify.o visitor.o

%.o: %.cpp $(HEADERS)
	$(CC) -c -o $@ $< $(CFLAGS)

merkle: merkle.o $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

clean:
	rm -f merkle *.o
