# Variables
CC = gcc                                    # Compiler to use
CFLAGS = -Wall -g -I"C:/Program Files/OpenSSL-Win64/include"  # Include path for OpenSSL headers
LDFLAGS = -L"C:/Program Files/OpenSSL-Win64/lib" -lssl -lcrypto   # Linking flags for OpenSSL
SRC = src/main.c src/deduplication.c src/file_handler.c src/backup_manager.c src/network.c # Source files
OBJ = $(SRC:.c=.o)                          # Object files (replace .c with .o)
TARGET = backup_tool                        # Name of the final executable

# Default target to build
all: $(TARGET)

# Rule to create the executable
$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)              # Link object files to create executable

# Rule to compile source files to object files
src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and the executable
clean:
	rm -f $(OBJ) $(TARGET)

# Optional rule to run the program
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
