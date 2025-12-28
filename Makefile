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
PI_ESTIMATOR_TARGET = $(BIN_DIR)/pi_estimator
MERGE_SORT_TARGET = $(BIN_DIR)/merge_sort
QUICK_SORT_TARGET = $(BIN_DIR)/quick_sort

# Shell source files (for unixsh)
SHELL_SRC = $(SRC_DIR)/shell/main.c \
            $(SRC_DIR)/shell/proc_mgmt.c \
            $(SRC_DIR)/shell/parser.c \
            $(SRC_DIR)/shell/executor.c \
            $(SRC_DIR)/shell/builtins.c \
            $(SRC_DIR)/shell/history.c
SHELL_OBJ = $(SHELL_SRC:$(SRC_DIR)/shell/%.c=obj/shell/%.o)

# Utils source files
UTILS_SRC = $(SRC_DIR)/utils/prng.c \
			$(SRC_DIR)/utils/shared_mem.c \
			$(SRC_DIR)/utils/atomic_ops.c
UTILS_OBJ = $(UTILS_SRC:$(SRC_DIR)/utils/%.c=obj/utils/%.o)

# Sudoku source files (STANDALONE)
SUDOKU_SRC = $(SRC_DIR)/apps/sudoku.c
SUDOKU_OBJ = obj/apps/sudoku.o obj/utils/atomic_ops.o

# Pi Estimator source files
PI_ESTIMATOR_SRC = $(SRC_DIR)/apps/pi_estimator.c
PI_ESTIMATOR_OBJ = obj/apps/pi_estimator.o $(UTILS_OBJ)

# Merge Sort source files
MERGE_SORT_SRC = $(SRC_DIR)/apps/merge_sort.c \
					$(SRC_DIR)/utils/shared_mem.c
MERGE_SORT_OBJ = $(MERGE_SORT_SRC:$(SRC_DIR)/%.c=obj/%.o)

# Quick Sort source files
QUICK_SORT_SRC = $(SRC_DIR)/apps/quick_sort.c \
                	$(SRC_DIR)/utils/shared_mem.c
QUICK_SORT_OBJ = $(QUICK_SORT_SRC:$(SRC_DIR)/%.c=obj/%.o)

# Output directory for object files
OBJ_DIR = obj

# Default target: build everything
all: $(SHELL_TARGET) $(SUDOKU_TARGET) $(PI_ESTIMATOR_TARGET) $(MERGE_SORT_TARGET) $(QUICK_SORT_TARGET)

# Rule to compile the main shell executable
$(SHELL_TARGET): $(SHELL_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Rule to compile the sudoku validator (STANDALONE)
$(SUDOKU_TARGET): $(SUDOKU_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ -lpthread

# Rule to compile the pi estimator
$(PI_ESTIMATOR_TARGET): $(PI_ESTIMATOR_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ -lpthread -lm

# Rule to compile the merge sort
$(MERGE_SORT_TARGET): $(MERGE_SORT_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Rule to compile the quick sort
$(QUICK_SORT_TARGET): $(QUICK_SORT_OBJ)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

# Rule for shell object files
obj/shell/%.o: $(SRC_DIR)/shell/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Rule for apps object files (STANDALONE)
obj/apps/%.o: $(SRC_DIR)/apps/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INC_DIR) -I$(INC_DIR)/utils -c $< -o $@

# Rule for utils object files
obj/utils/%.o: $(SRC_DIR)/utils/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INC_DIR) -I$(INC_DIR)/utils -c $< -o $@

# Clean up build artifacts
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Test commands
test: all
	@echo "Testing Unix Shell..."
	@echo "echo 'Hello from shell'" | ./$(BIN_DIR)/unixsh 2>/dev/null || true
	@echo ""
	@echo ""
	@echo "Testing Sudoku Validator..."
	@./$(BIN_DIR)/sudoku_validator 2>/dev/null | head -20 || true
	@echo ""
	@echo "Testing Pi Estimator..."
	@./$(BIN_DIR)/pi_estimator -p 2 -i 10000 -q 2>/dev/null || true
	@echo ""
	@echo "Testing Merge Sort..."
	@./$(BIN_DIR)/merge_sort 5 2 8 1 9
	@echo ""
	@echo "Testing Quick Sort..."
	@./$(BIN_DIR)/quick_sort 10 7 8 9 1 5
# Phony targets
.PHONY: all clean test