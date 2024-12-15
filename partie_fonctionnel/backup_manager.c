#include "backup_manager.h"
#include "deduplication.h"
#include "file_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>

// Fonction pour créer une nouvelle sauvegarde complète puis incrémentale
void create_backup(const char *source_dir, const char *backup_dir) {
    // Créer un nom pour le répertoire de sauvegarde
    char timestamp[20];
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H-%M-%S", tm_info);

    char backup_path[256];
    snprintf(backup_path, sizeof(backup_path), "%s/backup_%s", backup_dir, timestamp);

    // Créer le répertoire de sauvegarde
    if (mkdir(backup_path, 0700) == -1) {
        perror("Erreur lors de la création du répertoire de sauvegarde");
        return;
    }

    // Parcourir le répertoire source et sauvegarder les fichiers un par un
    struct dirent *entry;
    DIR *dir = opendir(source_dir);
    if (!dir) {
        perror("Erreur d'ouverture du répertoire source");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {  // Si c'est un fichier régulier
            char file_path[512];
            snprintf(file_path, sizeof(file_path), "%s/%s", source_dir, entry->d_name);

            // Appeler la fonction de sauvegarde pour chaque fichier
            backup_file(file_path);
        }
    }
    closedir(dir);
}

// Fonction permettant d'enregistrer dans fichier le tableau de chunk dédupliqué
void write_backup_file(const char *output_filename, Chunk *chunks, int chunk_count) {
    FILE *output_file = fopen(output_filename, "wb");
    if (!output_file) {
        perror("Erreur d'ouverture du fichier de sauvegarde");
        return;
    }

    for (int i = 0; i < chunk_count; i++) {
        if (chunks[i].data != NULL) {
            fwrite(chunks[i].md5, 1, MD5_DIGEST_LENGTH, output_file);  // Écrire le MD5 du chunk
            fwrite(chunks[i].data, 1, CHUNK_SIZE, output_file);         // Écrire les données du chunk
        }
    }

    fclose(output_file);
}

// Fonction implémentant la logique pour la sauvegarde d'un fichier
void backup_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Erreur d'ouverture du fichier pour la sauvegarde");
        return;
    }

    Chunk chunks[1000];
    Md5Entry hash_table[HASH_TABLE_SIZE] = {0};
    deduplicate_file(file, chunks, hash_table);

    // Générer un nom pour le fichier de sauvegarde
    char output_filename[256];
    snprintf(output_filename, sizeof(output_filename), "%s.bak", filename);

    // Sauvegarder les chunks dédupliqués
    write_backup_file(output_filename, chunks, 1000);

    fclose(file);
}

// Fonction permettant la restauration du fichier backup via le tableau de chunk
void write_restored_file(const char *output_filename, Chunk *chunks, int chunk_count) {
    FILE *output_file = fopen(output_filename, "wb");
    if (!output_file) {
        perror("Erreur d'ouverture du fichier restauré");
        return;
    }

    for (int i = 0; i < chunk_count; i++) {
        if (chunks[i].data != NULL) {
            fwrite(chunks[i].data, 1, CHUNK_SIZE, output_file);  // Écrire les données du chunk
        }
    }

    fclose(output_file);
}

// Fonction pour restaurer une sauvegarde
void restore_backup(const char *backup_id, const char *restore_dir) {
    char backup_file[256];
    snprintf(backup_file, sizeof(backup_file), "%s/%s.bak", backup_id, backup_id);

    FILE *backup = fopen(backup_file, "rb");
    if (!backup) {
        perror("Erreur d'ouverture du fichier de sauvegarde");
        return;
    }

    // Charger les chunks sauvegardés depuis le fichier
    Chunk chunks[1000];
    int chunk_count = 0;
    while (fread(chunks[chunk_count].md5, 1, MD5_DIGEST_LENGTH, backup) == MD5_DIGEST_LENGTH) {
        fread(chunks[chunk_count].data, 1, CHUNK_SIZE, backup);
        chunk_count++;
    }

    fclose(backup);

    // Restaurer les chunks dans le répertoire de restauration
    char restored_file[256];
    snprintf(restored_file, sizeof(restored_file), "%s/restored_file.dat", restore_dir);
    write_restored_file(restored_file, chunks, chunk_count);
}

// Fonction permettant de lister les différentes sauvegardes présentes dans la destination
void list_backups(const char *backup_dir) {
    DIR *dir = opendir(backup_dir);
    if (!dir) {
        perror("Erreur d'ouverture du répertoire de sauvegarde");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {  // Si c'est un répertoire
            printf("Backup trouvé: %s\n", entry->d_name);
        }
    }

    closedir(dir);
}
