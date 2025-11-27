# Compiler settings
CC = gcc

# -g for debugging, -std=c11 for modern c features
CFLAGS = -Wall -Wextra -std=c11 -g

# -lreadline (advanced input), -lpthread (Sudoko), -lm (Monte Carlo/math functions)
LDFLAGS = -lreadline -lpthread -lm

# Directories
BIN_DIR = bin
SRC_DIR = src
INC_DIR = include

# Target Executable
SHELL_TARGET = $(BIN_DIR)/unixsh

# Source file for the shell core 
SHELL_SRC = $(SRC_DIR)/shell/main.c \
			$(SRC_DIR)/shell/proc_mgmt.c
SHELL_OBJ = $(patsubst $(SRC_DIR)/shell/%.c, obj/%.o, $(SHELL_SRC))

# Output directory for object files
OBJ_DIR = obj

# Default target : build the shell
all: $(SHELL_TARGET)

# Rule to compile the main shell executable
$(SHELL_TARGET): $(SHELL_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Generic rule for compiling C source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/shell/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Clean up build artifacts
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean