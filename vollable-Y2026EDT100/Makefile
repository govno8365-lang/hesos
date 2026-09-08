CC = gcc
CFLAGS = -Wall -Wextra -O2 -Iparcers -Iinterpretator -Ibungc
OBJ_DIR = obj
TARGET = vollable

SRC = main.c \
      parcers/lex.c parcers/utils.c \
      parcers/parser.c parcers/ast.c parcers/checker.c \
      interpretator/interpretator.c interpretator/executor.c \
      interpretator/environment.c interpretator/labels.c interpretator/syscall.c \
      bungc/gc.c bungc/bar.c bungc/alloc.c bungc/roots.c

OBJ = $(SRC:%.c=$(OBJ_DIR)/%.o)

all: $(OBJ_DIR) $(TARGET)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)
	mkdir -p $(OBJ_DIR)/parcers
	mkdir -p $(OBJ_DIR)/interpretator
	mkdir -p $(OBJ_DIR)/bungc

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean