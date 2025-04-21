CC       = gcc
CFLAGS   = -Wall -g -Iparser -IShared_Memory_Setup -Ilogger -IBasic_IPC_Workflow
LDFLAGS  = -pthread -lrt

# Object files
PARSER_OBJ      = parser/parser.o
MEMORY_OBJ      = Shared_Memory_Setup/Memory_Segments.o
LOCKS_OBJ       = Basic_IPC_Workflow/intersection_locks.o
IPC_OBJ         = Basic_IPC_Workflow/ipc.o
LOG_OBJ         = logger/logger.o logger/csv_logger.o
RAG_OBJ         = Basic_IPC_Workflow/resource_allocation_graph.o
FAKESEC_OBJ     = Basic_IPC_Workflow/fake_sec.o

# Main binaries
MAIN_OBJ        = Railway_System.o
MAIN_TARGET     = iLikeTrains   

TRAIN_OBJ       = Basic_IPC_Workflow/Train_Movement_Simulation.o
TRAIN_TARGET    = train_sim

.PHONY: all clean

all: $(MAIN_TARGET) $(TRAIN_TARGET)

# Object file rules
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Main binary
$(MAIN_TARGET): $(MAIN_OBJ) $(PARSER_OBJ) $(MEMORY_OBJ) $(LOCKS_OBJ) $(LOG_OBJ) $(IPC_OBJ) $(RAG_OBJ) $(FAKESEC_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Train simulator binary
$(TRAIN_TARGET): $(TRAIN_OBJ) $(PARSER_OBJ) $(LOCKS_OBJ) $(LOG_OBJ) $(IPC_OBJ) $(RAG_OBJ) $(FAKESEC_OBJ) $(MEMORY_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	find . -type f -name "*.o" -delete
	rm -f $(MAIN_TARGET) $(TRAIN_TARGET)
