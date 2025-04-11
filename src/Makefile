CC       = gcc
CFLAGS   = -Wall -g -Iparser -IShared_Memory_Setup -Ilogger
LDFLAGS  = -pthread -lrt

PARSER_DIR      = parser
PARSER_OBJ      = $(PARSER_DIR)/parser.o

# Shared memory
MEMORY_SRC      = Shared_Memory_Setup/Memory_Segments.c
MEMORY_OBJ      = $(MEMORY_SRC:.c=.o)

# Intersection locks
LOCKS_SRC       = Basic_IPC_Workflow/intersection_locks.c
LOCKS_OBJ       = $(LOCKS_SRC:.c=.o)

# Logger
LOG_SRC         = logger/logger.c
LOG_OBJ         = $(LOG_SRC:.c=.o)

# Main “server” binary (was Railway_System.c)
MAIN_SRC        = Railway_System.c
MAIN_OBJ        = $(MAIN_SRC:.c=.o)
MAIN_TARGET     = iLikeTrains   

# Train simulator
TRAIN_SRC       = Basic_IPC_Workflow/Train_Movement_Simulation.c
TRAIN_OBJ       = $(TRAIN_SRC:.c=.o)
TRAIN_TARGET    = train_sim

.PHONY: all clean parser

all: parser $(MAIN_TARGET) $(TRAIN_TARGET)

# Build parser.o
parser:
	$(MAKE) -C $(PARSER_DIR)

$(PARSER_OBJ): | parser

# Build the main server binary
$(MAIN_TARGET): $(MAIN_OBJ) \
                $(PARSER_OBJ) \
                $(MEMORY_OBJ) \
                $(LOCKS_OBJ) \
                $(LOG_OBJ)
	$(CC) $(CFLAGS) -o $@ \
	    $(MAIN_OBJ) \
	    $(PARSER_OBJ) \
	    $(MEMORY_OBJ) \
	    $(LOCKS_OBJ) \
	    $(LOG_OBJ) \
	    $(LDFLAGS)

# Build the train simulation binary
$(TRAIN_TARGET): $(TRAIN_OBJ) \
                 $(PARSER_OBJ) \
                 $(LOCKS_OBJ) \
                 $(LOG_OBJ)
	$(CC) $(CFLAGS) -o $@ \
	    $(TRAIN_OBJ) \
	    $(PARSER_OBJ) \
	    $(LOCKS_OBJ) \
	    $(LOG_OBJ) \
	    $(LDFLAGS)

clean:
	rm -f *.o $(MAIN_TARGET) $(TRAIN_TARGET)
	$(MAKE) -C $(PARSER_DIR) clean
