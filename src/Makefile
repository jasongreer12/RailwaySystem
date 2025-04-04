CC = gcc
CFLAGS = -Wall -g -Iparser
MAIN_SRC = main.c
MAIN_OBJ = $(MAIN_SRC:.c=.o)
MAIN_TARGET = iLikeTrains
PARSER_DIR = parser

# New sources for the server and test client
SERVER_SRC = server.c ipc_utils.c
SERVER_OBJ = $(SERVER_SRC:.c=.o)
SERVER_TARGET = server

TEST_CLIENT_SRC = test_client.c ipc_utils.c
TEST_CLIENT_OBJ = $(TEST_CLIENT_SRC:.c=.o)
TEST_CLIENT_TARGET = test_client

# Build all targets: main project, server, and test client
all: $(MAIN_TARGET) $(SERVER_TARGET) $(TEST_CLIENT_TARGET)

# Main target build (your existing rules)
$(MAIN_TARGET): $(MAIN_OBJ)
	$(MAKE) -C $(PARSER_DIR)
	$(CC) $(CFLAGS) -o $@ $(MAIN_OBJ) $(PARSER_DIR)/parser.o

# Server target build
$(SERVER_TARGET): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJ)

# Test client target build
$(TEST_CLIENT_TARGET): $(TEST_CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $(TEST_CLIENT_OBJ)

clean:
	rm -f $(MAIN_OBJ) $(MAIN_TARGET) $(SERVER_OBJ) $(SERVER_TARGET) $(TEST_CLIENT_OBJ) $(TEST_CLIENT_TARGET)
	$(MAKE) -C $(PARSER_DIR) clean
