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


void restore_backup(const char *backup_id, const char *restore_dir, const char *backup_dir) {
    // Vérifier si le fichier de sauvegarde existe
    struct stat statbuf;
    if (stat(backup_id, &statbuf) != 0) {
        fprintf(stderr, "Erreur lors de l'ouverture du fichier de sauvegarde '%s': %s\n", backup_id, strerror(errno));
        return;
    }

    // Logique pour lire la metadata et obtenir la liste des chunks
    // Supposons que la metadata contient les noms des chunks et leurs positions
    char metadata_path[2048];  // Augmenter la taille du tampon pour les chemins longs
    snprintf(metadata_path, sizeof(metadata_path), "%s.metadata", backup_id); // Ajouter .metadata au fichier de sauvegarde

    FILE *metadata_file = fopen(metadata_path, "r");
    if (!metadata_file) {
        fprintf(stderr, "Erreur lors de l'ouverture du fichier de métadonnées '%s': %s\n", metadata_path, strerror(errno));
        return;
    }

    // Créer ou ouvrir le fichier de restauration
    char restored_file_path[2048];  // Augmenter la taille pour des chemins potentiellement longs
    snprintf(restored_file_path, sizeof(restored_file_path), "%s/restored_file.dat", restore_dir);
    
    FILE *restored_file = fopen(restored_file_path, "wb");
    if (!restored_file) {
        fprintf(stderr, "Impossible d'ouvrir le fichier de restauration pour l'écriture '%s': %s\n", restored_file_path, strerror(errno));
        fclose(metadata_file);
        return;
    }

    char chunk_filename[512];  // Assurez-vous que cette taille est suffisante pour les noms de chunk
    // Lire chaque chunk et l'écrire dans le fichier restauré
    while (fgets(chunk_filename, sizeof(chunk_filename), metadata_file)) {
        // Enlever le caractère de nouvelle ligne à la fin (si présent)
        chunk_filename[strcspn(chunk_filename, "\n")] = 0;

        // Vérifier que la longueur combinée de backup_dir et chunk_filename ne dépasse pas la taille de chunk_file_path
        char chunk_file_path[2048];  // Augmenter la taille du tampon à 2048 pour éviter la troncature
        if (strlen(backup_dir) + strlen(chunk_filename) + 1 > sizeof(chunk_file_path)) {
            fprintf(stderr, "Le chemin est trop long pour le fichier chunk '%s/%s'.\n", backup_dir, chunk_filename);
            continue;  // Passer au chunk suivant si le chemin est trop long
        }

        // Construire le chemin complet du fichier chunk
        snprintf(chunk_file_path, sizeof(chunk_file_path), "%s/%s", backup_dir, chunk_filename);

        FILE *chunk_file = fopen(chunk_file_path, "rb");
        if (!chunk_file) {
            fprintf(stderr, "Impossible d'ouvrir le fichier chunk '%s': %s\n", chunk_file_path, strerror(errno));
            continue;  // Passer au chunk suivant
        }

        // Lire les données du chunk et les écrire dans le fichier restauré
        char buffer[1024];
        size_t bytes_read;
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), chunk_file)) > 0) {
            size_t bytes_written = fwrite(buffer, 1, bytes_read, restored_file);
            if (bytes_written < bytes_read) {
                fprintf(stderr, "Erreur lors de l'écriture dans le fichier restauré '%s'\n", restored_file_path);
                fclose(chunk_file);
                fclose(restored_file);
                fclose(metadata_file);
                return;
            }
        }

        fclose(chunk_file);  // Fermer chaque fichier chunk après utilisation
    }

    // Fermer les fichiers une fois le processus terminé
    fclose(metadata_file);
    fclose(restored_file);

    printf("Restauration du fichier terminée avec succès : '%s'\n", restored_file_path);
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
