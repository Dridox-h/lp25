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
    /*
     * @param: source_dir est le chemin vers le répertoire à sauvegarder
     * @param: backup_dir est le chemin vers le répertoire de sauvegarde
     */

    DIR *dir;
    struct dirent *entry;
    dir = opendir(source_dir);
    if (dir != NULL) {
        // Créer le répertoire de sauvegarde s'il n'existe pas
        if (mkdir(backup_dir, 0755) != -1 ) {
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
                        printf("Erreur lors de l'ouverture du fichier source");
                        continue;
                    }

                    // Effectuer la déduplication du fichier et ajouter les chunks
                    deduplicate_file(source_file, chunks, hash_table);

                    fclose(source_file);

                    // Sauvegarder les informations de chunks dans le répertoire de sauvegarde
                    char backup_metadata_path[1024];
                    snprintf(backup_metadata_path, sizeof(backup_metadata_path), "%s.metadata", backup_path);
                    write_backup_file(backup_metadata_path, chunks, chunk_count);
                }

            }
            printf("Sauvegarde terminée avec succès dans : %s\n", backup_dir);

        }
        else{
            printf("Erreur lors de la création du répertoire de sauvegarde");
        }
        
    }
    else{
        printf("Erreur lors de l'ouverture du répertoire source");

    }
    closedir(dir);
}




// Fonction permettant d'enregistrer dans fichier le tableau de chunk dédupliqué
 void write_backup_file(const char *output_filename, Chunk *chunks, int chunk_count) {
    /*
    * Paramètres :
    *   output_filename: fichier où enregistrer les données de la sauvegarde.
    *   chunks: tableau de chunks représentant les blocs dédupliqués des fichiers sauvegardés.
    *   chunk_count: le nombre de chunks dans le tableau.
    */

    FILE *output_file = fopen(output_filename, "w");
    if (output_file != NULL) {        
        // Pour chaque chunk, on écrit son MD5 et le chemin du fichier associé dans le fichier
        for (int i = 0; i < chunk_count; i++) {
            // Convertir le MD5 en chaîne hexadécimale pour l'afficher dans le fichier
            char md5_str[MD5_DIGEST_LENGTH * 2 + 1];
            for (int j = 0; j < MD5_DIGEST_LENGTH; j++) {
                sprintf(&md5_str[j * 2], "%02x", chunks[i].md5[j]);
            }
            // Enregistrer les informations dans le fichier : chemin du fichier et MD5
            fprintf(output_file, "Chunk %d: MD5: %s\n", i, md5_str);
        }
    }
    else{
        printf("Erreur lors de l'ouverture du fichier de sauvegarde");
    }
    fclose(output_file);
}


// Fonction implémentant la logique pour la sauvegarde d'un fichier
void backup_file(const char *filename) {

    struct stat statbuf;
    if (stat(filename, &statbuf) != -1) {
        // Vérifier que c'est un fichier régulier
        if (S_ISREG(statbuf.st_mode)) {
            FILE *file = fopen(filename, "rb");
            if (file) {        
                // Calculer la taille du fichier et le nombre de chunks nécessaires
                size_t file_size = statbuf.st_size;
                size_t chunk_count = (file_size + CHUNK_SIZE - 1) / CHUNK_SIZE;

                // Allouer dynamiquement un tableau de chunks et une table de hachage
                Chunk *chunks = malloc(chunk_count * sizeof(Chunk));
                if (chunks) {

                    Md5Entry hash_table[HASH_TABLE_SIZE] = {0};

                    // Dédupliquer le fichier en remplissant le tableau de chunks
                    deduplicate_file(file, chunks, hash_table);

                    fclose(file);

                    // Générer un nom de fichier de sauvegarde basé sur l'heure actuelle
                    char backup_filename[256];
                    time_t now = time(NULL);
                    strftime(backup_filename, sizeof(backup_filename), "backup_%Y%m%d_%H%M%S.dat", localtime(&now));

                    write_backup_file(backup_filename, chunks, chunk_count);

                    // Libérer la mémoire allouée pour les chunks
                    for (size_t i = 0; i < chunk_count; ++i) {
                        free(chunks[i].data);
                    }
                    free(chunks);

                    printf("Sauvegarde terminée avec succès : %s\n", backup_filename);
                }
                else{
                    printf("Erreur lors de l'allocation mémoire pour les chunks");
                    fclose(file);
                }
            }
            else{
                printf("Erreur lors de l'ouverture du fichier");
            }
        }
        else{
            printf("Le chemin spécifié n'est pas un fichier régulier : %s\n", filename);
        }
    }
    else{
        printf("Erreur lors de l'accès au fichier");
    }
}

// Fonction permettant la restauration du fichier backup via le tableau de chunk
void write_restored_file(const char *output_filename, Chunk *chunks, int chunk_count) {
    FILE *output_file = fopen(output_filename, "wb");
    if (output_file) {
        printf("Erreur lors de l'ouverture du fichier de sortie");

        // Écrire chaque chunk dans le fichier de sortie
        for (int i = 0; i < chunk_count; ++i) {
            if (fwrite(chunks[i].data, 1, CHUNK_SIZE, output_file) != CHUNK_SIZE) {
                printf("Erreur lors de l'écriture dans le fichier de sortie");
                fclose(output_file);
            }
        }

        fclose(output_file);
        printf("Fichier restauré avec succès : %s\n", output_filename);
    }
    else{
        printf("Erreur lors de l'ouverture du fichier de sortie");
    }
}

// Fonction pour restaurer une sauvegarde
void restore_backup(const char *backup_id, const char *restore_dir) {
    // Construire le chemin complet vers le fichier de sauvegarde
    char backup_filepath[256];
    snprintf(backup_filepath, sizeof(backup_filepath), "%s/%s", restore_dir, backup_id);

    FILE *backup_file = fopen(backup_filepath, "rb");
    if (backup_file) {
        Chunk *chunks = NULL;
        int chunk_count = 0;
        undeduplicate_file(backup_file, &chunks, &chunk_count);

        fclose(backup_file);

        // Construire un chemin pour le fichier restauré
        char restored_filepath[256];
        snprintf(restored_filepath, sizeof(restored_filepath), "%s/restored_file.dat", restore_dir);

        // Restaurer le fichier à partir des chunks
        write_restored_file(restored_filepath, chunks, chunk_count);

        // Libérer la mémoire allouée pour les chunks
        for (int i = 0; i < chunk_count; ++i) {
            free(chunks[i].data);
        }
        free(chunks);

        printf("Restauration terminée avec succès dans : %s\n", restore_dir);
    }
    else{
        printf("Erreur lors de l'ouverture du fichier de sauvegarde");
    }
}

// Fonction permettant de lister les différentes sauvegardes présentes dans la destination
void list_backups(const char *backup_dir) {
    DIR *dir = opendir(backup_dir);
    if (dir) {
        printf("Liste des sauvegardes dans %s :\n", backup_dir);
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR) {
                // Ignore les répertoires spéciaux "." et ".."
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                    continue;
                }
                printf(" - %s\n", entry->d_name);
            }
        }
        closedir(dir);
    }
    else{
        printf("Erreur lors de l'ouverture du répertoire de sauvegarde");
    }
}