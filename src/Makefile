CC = gcc
CFLAGS = -Wall -g -Iparser
LDFLAGS = -pthread -lrt
MAIN_SRC = Railway_System.c
MAIN_OBJ = $(MAIN_SRC:.c=.o)
MAIN_TARGET = iLikeTrains
PARSER_DIR = parser

# Add intersection locks files
LOCKS_SRC = Basic_IPC_Workflow/intersection_locks.c
LOCKS_OBJ = $(LOCKS_SRC:.c=.o)

# Shared memory 
MEMORY_SRC = Shared_Memory_Setup/Memory_Segments.c
MEMORY_OBJ = $(MEMORY_SRC:.c=.o)
CFLAGS += -IShared_Memory_Setup

# New sources for the server and test client
SERVER_SRC = server.c $(LOCKS_SRC)
SERVER_OBJ = $(SERVER_SRC:.c=.o)
SERVER_TARGET = server

# Train simulation target
TRAIN_SIM_SRC = Basic_IPC_Workflow/Train_Movement_Simulation.c $(LOCKS_SRC)
TRAIN_SIM_OBJ = $(TRAIN_SIM_SRC:.c=.o)
TRAIN_SIM_TARGET = train_sim

# Build all targets: main project, server, test client, and train simulation
all: $(MAIN_TARGET) $(SERVER_TARGET) $(TRAIN_SIM_TARGET)

# Main target build (your existing rules)
$(MAIN_TARGET): $(MAIN_OBJ) $(MEMORY_OBJ)
	$(MAKE) -C $(PARSER_DIR)
	$(CC) $(CFLAGS) -o $@ $(MAIN_OBJ) $(MEMORY_OBJ) $(PARSER_DIR)/parser.o $(LDFLAGS)

# Server target build - now with locks
$(SERVER_TARGET): $(SERVER_OBJ)
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJ) $(LDFLAGS)

# Train simulation target
$(TRAIN_SIM_TARGET): $(TRAIN_SIM_OBJ)
	$(CC) $(CFLAGS) -o $@ $(TRAIN_SIM_OBJ) $(LDFLAGS)

clean:
	rm -f $(MAIN_OBJ) $(MAIN_TARGET) $(SERVER_OBJ) $(SERVER_TARGET) $(TRAIN_SIM_OBJ) $(TRAIN_SIM_TARGET) $(LOCKS_OBJ) $(MEMORY_OBJ)
	$(MAKE) -C $(PARSER_DIR) clean
