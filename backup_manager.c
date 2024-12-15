#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>
#include <openssl/md5.h>
#include "backup_manager.h"
#include "deduplication.h"

// Fonction pour créer une nouvelle sauvegarde incrémentale
void create_backup(const char *source_dir, const char *backup_dir) {
    DIR *dir = opendir(source_dir);
    if (!dir) {
        perror("Erreur lors de l'ouverture du répertoire source");
        return;
    }

    if (mkdir(backup_dir, 0755) == -1 && errno != EEXIST) {
        perror("Erreur lors de la création du répertoire de sauvegarde");
        closedir(dir);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char source_path[1024];
        snprintf(source_path, sizeof(source_path), "%s/%s", source_dir, entry->d_name);

        char backup_path[1024];
        snprintf(backup_path, sizeof(backup_path), "%s/%s", backup_dir, entry->d_name);

        struct stat statbuf;
        if (stat(source_path, &statbuf) == 0) {
            if (S_ISREG(statbuf.st_mode)) {
                // Sauvegarder les fichiers réguliers
                backup_file(source_path, backup_dir);
            } else if (S_ISDIR(statbuf.st_mode)) {
                // Gestion récursive des sous-répertoires
                create_backup(source_path, backup_path);
            }
        }
    }

    closedir(dir);
    printf("Sauvegarde terminée avec succès dans : %s\n", backup_dir);
}

// Fonction pour sauvegarder un fichier dédupliqué
void backup_file(const char *filename, const char *backup_dir) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier");
        return;
    }

    Chunk chunks[HASH_TABLE_SIZE];
    Md5Entry hash_table[HASH_TABLE_SIZE] = {0};
    int chunk_count = 0;

    deduplicate_file(file, chunks, hash_table);
    fclose(file);

    // Création du chemin du fichier de métadonnées
    char *file_name_only = strrchr(filename, '/');
    if (!file_name_only) file_name_only = (char *)filename; // Si aucun '/' n'est trouvé

    char backup_file_path[1024];
    snprintf(backup_file_path, sizeof(backup_file_path), "%s/%s.metadata", backup_dir, file_name_only);

    write_backup_file(backup_file_path, chunks, chunk_count);

    for (int i = 0; i < chunk_count; i++) {
        free(chunks[i].data);
    }
}

// Fonction permettant d'enregistrer dans un fichier le tableau de chunks dédupliqués
void write_backup_file(const char *output_filename, Chunk *chunks, int chunk_count) {
    FILE *output_file = fopen(output_filename, "w");
    if (!output_file) {
        perror("Erreur lors de l'ouverture du fichier de sauvegarde");
        return;
    }

    for (int i = 0; i < chunk_count; i++) {
        char md5_str[MD5_DIGEST_LENGTH * 2 + 1];
        for (int j = 0; j < MD5_DIGEST_LENGTH; j++) {
            sprintf(&md5_str[j * 2], "%02x", chunks[i].md5[j]);
        }
        fprintf(output_file, "Chunk %d: Size: %zu, MD5: %s\n", i, chunks[i].size, md5_str);
    }

    fclose(output_file);
}

// Fonction pour restaurer une sauvegarde
void restore_backup(const char *backup_id, const char *restore_dir) {
    char backup_file_path[1024];
    snprintf(backup_file_path, sizeof(backup_file_path), "%s/%s.metadata", restore_dir, backup_id);

    FILE *backup_file = fopen(backup_file_path, "r");
    if (!backup_file) {
        perror("Erreur lors de l'ouverture du fichier de sauvegarde");
        return;
    }

    Chunk *chunks = malloc(HASH_TABLE_SIZE * sizeof(Chunk));
    int chunk_count = 0;

    char line[256];
    while (fgets(line, sizeof(line), backup_file)) {
        size_t chunk_size;
        char md5_str[MD5_DIGEST_LENGTH * 2 + 1];
        sscanf(line, "Chunk %*d: Size: %zu, MD5: %s", &chunk_size, md5_str);

        chunks[chunk_count].data = malloc(chunk_size);
        chunks[chunk_count].size = chunk_size;
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
            sscanf(&md5_str[i * 2], "%2hhx", &chunks[chunk_count].md5[i]);
        }

        chunk_count++;
    }

    fclose(backup_file);

    char restored_file_path[1024];
    snprintf(restored_file_path, sizeof(restored_file_path), "%s/%s_restored", restore_dir, backup_id);
    write_restored_file(restored_file_path, chunks, chunk_count);

    for (int i = 0; i < chunk_count; i++) {
        free(chunks[i].data);
    }
    free(chunks);
}

// Fonction permettant la restauration du fichier depuis le tableau de chunks
void write_restored_file(const char *output_filename, Chunk *chunks, int chunk_count) {
    FILE *output_file = fopen(output_filename, "wb");
    if (!output_file) {
        perror("Erreur lors de l'ouverture du fichier restauré");
        return;
    }

    for (int i = 0; i < chunk_count; i++) {
        fwrite(chunks[i].data, chunks[i].size, 1, output_file);
    }

    fclose(output_file);
    printf("Restauration terminée dans : %s\n", output_filename);
}

// Fonction pour lister les sauvegardes dans le répertoire cible
void list_backups(const char *backup_dir) {
    DIR *dir = opendir(backup_dir);
    if (!dir) {
        perror("Erreur lors de l'ouverture du répertoire de sauvegarde");
        return;
    }

    struct dirent *entry;
    printf("Liste des sauvegardes dans %s :\n", backup_dir);
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            printf(" - %s\n", entry->d_name);
        }
    }

    closedir(dir);
}