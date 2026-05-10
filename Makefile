CC = gcc
CFLAGS = -Wall -Wextra -std=c11

ifeq ($(MOCKREG),1)
CFLAGS += -DMOCK_REGISTERS=1
endif

SRC_DIR = source
BIN_DIR = build
TARGET = $(BIN_DIR)/test

# Find all .c files in source/
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Convert source/file.c → source/file.o
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Compile rule: source/file.o from source/file.c
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(SRC_DIR)/*.o
	rm -rf $(BIN_DIR)
