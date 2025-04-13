CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lrt

# Source files
CSV_LOGGER_SRC = csv_logger.c
TEST_SRC = test_csv_logger.c test_csv_logger2.c

# Object files
CSV_LOGGER_OBJ = $(CSV_LOGGER_SRC:.c=.o)
TEST_OBJS = $(TEST_SRC:.c=.o)

# Target executable
TARGET = test_csv_logger

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(CSV_LOGGER_OBJ) $(TEST_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(CSV_LOGGER_OBJ) $(TEST_OBJS) $(TARGET) *.csv