# Variables
CC = gcc
CFLAGS = -Wall -g -I. -lssl -lcrypto   # Options de compilation
SRC = main.c deduplication.c file_handler.c backup_manager.c network.c
OBJ = $(SRC:.c=.o)
TARGET = backup_tool

# Règle par défaut
all: $(TARGET)

# Règle pour créer l'exécutable
$(TARGET): $(OBJ)
	$(CC) -o $@ $^

# Règle pour compiler les fichiers source en objets
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Règle pour nettoyer les fichiers objets et l'exécutable
clean:
	rm -f $(OBJ) $(TARGET)

# Règle pour exécuter le programme (optionnel)
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run