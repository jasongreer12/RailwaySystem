CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lrt

# Source files
CSV_LOGGER_SRC = csv_logger.c
TEST_SRC = test_csv_logger.c
TEST2_SRC = test_csv_logger2.c

# Object files
CSV_LOGGER_OBJ = $(CSV_LOGGER_SRC:.c=.o)
TEST_OBJ = $(TEST_SRC:.c=.o)
TEST2_OBJ = $(TEST2_SRC:.c=.o)

# Target executables
TARGET = test_csv_logger
TARGET2 = test_csv_logger2

.PHONY: all clean

all: $(TARGET) $(TARGET2)

$(TARGET): $(CSV_LOGGER_OBJ) $(TEST_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TARGET2): $(CSV_LOGGER_OBJ) $(TEST2_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(CSV_LOGGER_OBJ) $(TEST_OBJ) $(TEST2_OBJ) $(TARGET) $(TARGET2) *.csv