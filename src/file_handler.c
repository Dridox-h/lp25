#include <stdio.h>
#include "file_handler.h"

void list_files(const char *path) {
    // Implémenter la logique pour lister les fichiers dans le répertoire donné
    DIR *dir;
    struct dirent *entry;

    dir = opendir(path);
    if (dir == NULL) {
        perror("Erreur lors de l'ouverture du répertoire");
        return;
    }

    printf("Fichiers dans le répertoire %s :\n", path);
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // Vérifie si c'est un fichier régulier
            printf("  %s\n", entry->d_name);
        }
    }

    closedir(dir);
}

void read_file(const char *filepath) {
    // Implémenter la logique pour lire un fichier
    FILE *file;
    char *buffer;
    long file_size;

    file = fopen(filepath, "rb");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return;
    }

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    rewind(file);

    buffer = malloc(file_size + 1);
    if (buffer == NULL) {
        perror("Erreur d'allocation mémoire");
        fclose(file);
        return;
    }

    fread(buffer, 1, file_size, file);
    buffer[file_size] = '\0';

    printf("Contenu du fichier %s :\n%s\n", filepath, buffer);

    free(buffer);
    fclose(file);
}

void write_file(const char *filepath, const void *data, size_t size) {
    // Implémenter la logique pour écrire des données dans un fichier
    FILE *file;

    file = fopen(filepath, "wb");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return;
    }

    if (fwrite(data, 1, size, file) != size) {
        perror("Erreur lors de l'écriture dans le fichier");
    } else {
        printf("Les données ont été écrites dans le fichier %s avec succès.\n", filepath);
    }

    fclose(file);
}