CC = gcc
CFLAGS = -Wall -g -Iparser
LDFLAGS = -pthread -lrt
MAIN_SRC = Railway_System.c
MAIN_OBJ = $(MAIN_SRC:.c=.o)
MAIN_TARGET = iLikeTrains
PARSER_DIR = parser

# Add intersection locks files
LOCKS_SRC = intersection_locks.c
LOCKS_OBJ = $(LOCKS_SRC:.c=.o)

# New sources for the server and test client
SERVER_SRC = server.c ipc_utils.c $(LOCKS_SRC)
SERVER_OBJ = $(SERVER_SRC:.c=.o)
SERVER_TARGET = server

TEST_CLIENT_SRC = test_client.c ipc_utils.c
TEST_CLIENT_OBJ = $(TEST_CLIENT_SRC:.c=.o)
TEST_CLIENT_TARGET = test_client

# Train simulation target
TRAIN_SIM_SRC = Train_Movement_Simulation.c $(LOCKS_SRC)
TRAIN_SIM_OBJ = $(TRAIN_SIM_SRC:.c=.o)
TRAIN_SIM_TARGET = train_sim

# Build all targets: main project, server, test client, and train simulation
all: $(MAIN_TARGET) $(SERVER_TARGET) $(TEST_CLIENT_TARGET) $(TRAIN_SIM_TARGET)

# Main target build (your existing rules)
$(MAIN_TARGET): $(MAIN_OBJ)
	$(MAKE) -C $(PARSER_DIR)
	$(CC) $(CFLAGS) -o $@ $(MAIN_OBJ) $(PARSER_DIR)/parser.o $(LDFLAGS)

# Server target build - now with locks
$(SERVER_TARGET): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJ) $(LDFLAGS)

# Test client target build
$(TEST_CLIENT_TARGET): $(TEST_CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $(TEST_CLIENT_OBJ) $(LDFLAGS)

# Train simulation target
$(TRAIN_SIM_TARGET): $(TRAIN_SIM_OBJ)
	$(CC) $(CFLAGS) -o $@ $(TRAIN_SIM_OBJ) $(LDFLAGS)

clean:
	rm -f $(MAIN_OBJ) $(MAIN_TARGET) $(SERVER_OBJ) $(SERVER_TARGET) $(TEST_CLIENT_OBJ) $(TEST_CLIENT_TARGET) $(TRAIN_SIM_OBJ) $(TRAIN_SIM_TARGET) $(LOCKS_OBJ)
	$(MAKE) -C $(PARSER_DIR) clean
