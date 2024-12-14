#include "backup_manager.h"
#include "deduplication.h"
#include "file_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>

// Fonction pour créer une nouvelle sauvegarde complète puis incrémentale
void create_backup(const char *source_dir, const char *backup_dir) {
    /*
     * @param: source_dir est le chemin vers le répertoire à sauvegarder
     * @param: backup_dir est le chemin vers le répertoire de sauvegarde
     */

    DIR *dir;
    struct dirent *entry;
    dir = opendir(source_dir);
    if (dir != NULL) {
        // Créer le répertoire de sauvegarde s'il n'existe pas
        if (mkdir(backup_dir, 0755) == -1) {
            // Si le répertoire existe déjà, on n'affiche pas d'erreur
            if (errno != EEXIST) {
                printf("Erreur lors de la création du répertoire de sauvegarde : %s\n", strerror(errno));
                closedir(dir);
                return;
            }
        }

        // Initialiser la table de hachage pour gérer la déduplication
        Md5Entry hash_table[HASH_TABLE_SIZE] = {0};
        Chunk chunks[10000]; // Un tableau pour les chunks
        int chunk_count = 0;

        // Parcourir tous les fichiers dans le répertoire source
        while ((entry = readdir(dir)) != NULL) {
            // Ignorer les fichiers "." et ".."
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            // Construire les chemins complets des fichiers source et destination
            char source_path[1024];
            snprintf(source_path, sizeof(source_path), "%s/%s", source_dir, entry->d_name);

            char backup_path[1024];
            snprintf(backup_path, sizeof(backup_path), "%s/%s", backup_dir, entry->d_name);

            // Vérifier si c'est un fichier régulier
            struct stat statbuf;
            if (stat(source_path, &statbuf) == 0 && S_ISREG(statbuf.st_mode)) {
                // Ouvrir le fichier source
                FILE *source_file = fopen(source_path, "rb");
                if (source_file == NULL) {
                    printf("Erreur lors de l'ouverture du fichier source : %s\n", source_path);
                    continue;
                }

                // Effectuer la déduplication du fichier et ajouter les chunks
                deduplicate_file(source_file, chunks, hash_table, &chunk_count); // Ajout du &chunk_count

                fclose(source_file);

                // Sauvegarder les informations de chunks dans le répertoire de sauvegarde
                char backup_metadata_path[2048];
                snprintf(backup_metadata_path, sizeof(backup_metadata_path), "%s.metadata", backup_path);
                write_backup_file(backup_metadata_path, chunks, chunk_count);
            }
        }
        closedir(dir);

        printf("Sauvegarde terminée avec succès dans : %s\n", backup_dir);
    }
    else {
        printf("Erreur lors de l'ouverture du répertoire source : %s\n", source_dir);
    }
}


// Fonction permettant d'enregistrer dans fichier le tableau de chunk dédupliqué
void write_backup_file(const char *output_filename, Chunk *chunks, int chunk_count) {
    FILE *output_file = fopen(output_filename, "w");
    if (output_file != NULL) {
        printf("Écriture du fichier de sauvegarde: %s\n", output_filename);  // Ajouter un log pour vérifier
        for (int i = 0; i < chunk_count; i++) {
            char md5_str[MD5_DIGEST_LENGTH * 2 + 1];
            for (int j = 0; j < MD5_DIGEST_LENGTH; j++) {
                sprintf(&md5_str[j * 2], "%02x", chunks[i].md5[j]);
            }
            fprintf(output_file, "Chunk %d: MD5: %s\n", i, md5_str);
        }
        fclose(output_file);
        printf("Sauvegarde écrite dans le fichier : %s\n", output_filename);  // Ajouter un log pour confirmer la fin
    }
    else {
        printf("Erreur lors de l'ouverture du fichier de sauvegarde : %s\n", output_filename);
    }
}


void write_restored_file(const char *output_filename, Chunk *chunks, int chunk_count) {
    FILE *output_file = fopen(output_filename, "wb");
    if (output_file) {
        // Écrire chaque chunk dans le fichier de sortie
        for (int i = 0; i < chunk_count; ++i) {
            if (fwrite(chunks[i].data, 1, CHUNK_SIZE, output_file) != CHUNK_SIZE) {
                printf("Erreur lors de l'écriture dans le fichier de sortie\n");
                fclose(output_file);
                return;
            }
        }

        fclose(output_file);
        printf("Fichier restauré avec succès : %s\n", output_filename);
    }
    else {
        printf("Erreur lors de l'ouverture du fichier de sortie\n");
    }
}

// Fonction de restauration modifiée
void restore_backup(const char *backup_file_path, const char *restore_dir, const char *backup_dir) {
    // Vérifier si le fichier de sauvegarde existe
    struct stat statbuf;
    if (stat(backup_file_path, &statbuf) != 0) {
        fprintf(stderr, "Erreur lors de l'ouverture du fichier de sauvegarde : %s\n", strerror(errno));
        return;
    }

    // Ici, vous pouvez ajouter la logique pour restaurer le fichier
    printf("Fichier de sauvegarde trouvé : %s\n", backup_file_path);

    // Exemple de restauration, ici il est simplement copié dans le répertoire de restauration
    // Vous pouvez remplacer par votre propre logique de restauration
    char restored_file_path[256];
    snprintf(restored_file_path, sizeof(restored_file_path), "%s/restored_file.dat", restore_dir);

    FILE *backup_file = fopen(backup_file_path, "rb");
    if (!backup_file) {
        fprintf(stderr, "Impossible d'ouvrir le fichier de sauvegarde pour la lecture\n");
        return;
    }

    FILE *restored_file = fopen(restored_file_path, "wb");
    if (!restored_file) {
        fprintf(stderr, "Impossible d'ouvrir le fichier de restauration pour l'écriture\n");
        fclose(backup_file);
        return;
    }

    // Copie des données du fichier de sauvegarde dans le fichier restauré
    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), backup_file)) > 0) {
        fwrite(buffer, 1, bytes_read, restored_file);
    }

    fclose(backup_file);
    fclose(restored_file);
}




// Fonction permettant de lister les différentes sauvegardes présentes dans la destination
void list_backups(const char *backup_dir) {
    DIR *dir = opendir(backup_dir);
    if (dir) {
        printf("Liste des sauvegardes dans %s :\n", backup_dir);
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_REG) { // Utilisation de DT_REG pour les fichiers
                // Ignore les fichiers spéciaux "." et ".."
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                    continue;
                }
                printf(" - %s\n", entry->d_name);
            }
        }
        closedir(dir);
    }
    else {
        printf("Erreur lors de l'ouverture du répertoire de sauvegarde : %s\n", backup_dir);
    }
}
