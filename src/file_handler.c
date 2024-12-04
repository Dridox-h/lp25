#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "file_handler.h"
#include "deduplication.h"

// Fonction permettant de lire le fichier .backup_log et de charger son contenu dans une structure log_t
log_t read_backup_log(const char *logfile) {
    log_t logs = { .head = NULL, .tail = NULL };
    char line[512];
    FILE *file = fopen(logfile, "r");

    if (!file) {
        perror("Erreur d'ouverture du fichier .backup_log");
        return logs; // Retourne une liste vide en cas d'erreur
    }

    while (fgets(line, sizeof(line), file)) {
        // Crée un nouvel élément log_element
        log_element *new_elt = malloc(sizeof(log_element));
        if (!new_elt) {
            perror("Erreur d'allocation mémoire pour log_element");
            fclose(file);
            return logs;
        }

        // Découpe la ligne lue en composants : chemin, hachage MD5, date
        char *token = strtok(line, " ");
        new_elt->path = strdup(token); // Chemin

        token = strtok(NULL, " ");
        for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
            sscanf(token + i * 2, "%2hhx", &new_elt->md5[i]); // Hachage MD5
        }

        token = strtok(NULL, "\n");
        new_elt->date = strdup(token); // Date

        // Ajoute le nouvel élément à la liste chaînée
        new_elt->next = NULL;
        new_elt->prev = logs.tail;
        if (logs.tail) {
            logs.tail->next = new_elt;
        } else {
            logs.head = new_elt; // Premier élément
        }
        logs.tail = new_elt; // Met à jour le dernier élément
    }

    fclose(file);
    return logs;
}

// Fonction permettant de mettre à jour le fichier .backup_log avec le contenu de log_t
void update_backup_log(const char *logfile, log_t *logs) {
    FILE *file = fopen(logfile, "w");
    if (!file) {
        perror("Erreur d'ouverture du fichier .backup_log pour mise à jour");
        return;
    }

    // Parcourt la liste chaînée et écrit chaque élément dans le fichier
    log_element *current = logs->head;
    while (current) {
        write_log_element(current, file);
        current = current->next;
    }

    fclose(file);
}

// Fonction permettant d'écrire un élément log_element dans le fichier
void write_log_element(log_element *elt, FILE *file) {
    char md5_hex[MD5_DIGEST_LENGTH * 2 + 1] = NULL;

    if (!elt || !file) {
        return;
    }

    // Convertit le hachage MD5 en chaîne hexadécimale
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        sprintf(md5_hex + i * 2, "%02x", elt->md5[i]);
    }

    // Écrit l'élément dans le fichier
    fprintf(file, "%s %s %s\n", elt->path, md5_hex, elt->date);
}

// Fonction pour lister les fichiers dans un répertoire (non récursif)
void list_files(const char *path) {
    struct dirent *entry;
    DIR *dir = opendir(path);

    if (!dir) {
        perror("Erreur lors de l'ouverture du répertoire");
        return;
    }

    while ((entry = readdir(dir))) {
        // Ignore les entrées spéciales "." et ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        printf("%s/%s\n", path, entry->d_name); // Affiche le chemin complet
    }

    closedir(dir);
}