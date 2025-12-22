# Compiler settings
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -std=c11 -g 

# Libraries
LDFLAGS = -lreadline -lpthread -lm

# Directories
BIN_DIR = bin
SRC_DIR = src
INC_DIR = include

# Target Executables
SHELL_TARGET = $(BIN_DIR)/unixsh
SUDOKU_TARGET = $(BIN_DIR)/sudoku_validator

# Shell source files (for unixsh)
SHELL_SRC = $(SRC_DIR)/shell/main.c \
            $(SRC_DIR)/shell/proc_mgmt.c \
            $(SRC_DIR)/shell/parser.c \
            $(SRC_DIR)/shell/executor.c \
            $(SRC_DIR)/shell/builtins.c \
            $(SRC_DIR)/shell/history.c
SHELL_OBJ = $(SHELL_SRC:$(SRC_DIR)/shell/%.c=obj/shell/%.o)

# Sudoku source files (STANDALONE)
SUDOKU_SRC = $(SRC_DIR)/apps/sudoku.c
SUDOKU_OBJ = $(SUDOKU_SRC:$(SRC_DIR)/apps/%.c=obj/apps/%.o)

# Output directory for object files
OBJ_DIR = obj

# Default target: build everything
all: $(SHELL_TARGET) $(SUDOKU_TARGET)

# Rule to compile the main shell executable
$(SHELL_TARGET): $(SHELL_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Rule to compile the sudoku validator (STANDALONE)
$(SUDOKU_TARGET): $(SUDOKU_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ -lpthread

# Rule for shell object files
obj/shell/%.o: $(SRC_DIR)/shell/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Rule for apps object files (STANDALONE)
obj/apps/%.o: $(SRC_DIR)/apps/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INC_DIR) -I$(INC_DIR)/utils -c $< -o $@

# Clean up build artifacts
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Phony targets
.PHONY: all clean