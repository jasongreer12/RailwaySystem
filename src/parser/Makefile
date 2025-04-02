CC = gcc
CFLAGS = -Wall -g
OBJ = parser.o

parser.o: parser.c parser.h
	$(CC) $(CFLAGS) -c parser.c -o parser.o

clean:
	rm -f $(OBJ)
