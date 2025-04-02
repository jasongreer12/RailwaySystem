CC = gcc
CFLAGS = -Wall -g -Iparser
MAIN_SRC = main.c
MAIN_OBJ = $(MAIN_SRC:.c=.o)
MAIN_TARGET = iLikeTrains
PARSER_DIR = parser

all: $(MAIN_TARGET)

$(MAIN_TARGET): $(MAIN_OBJ)
	$(MAKE) -C $(PARSER_DIR)
	$(CC) $(CFLAGS) -o $@ $(MAIN_OBJ) $(PARSER_DIR)/parser.o

clean:
	rm -f $(MAIN_OBJ) $(MAIN_TARGET)
	$(MAKE) -C $(PARSER_DIR) clean
