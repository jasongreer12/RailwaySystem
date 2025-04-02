CC = gcc
CFLAGS = -Wall -g
SRC = parser.c parse_tester.c
OBJ = $(SRC:.c=.o)
TARGET = ptest

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(OBJ) $(TARGET)
