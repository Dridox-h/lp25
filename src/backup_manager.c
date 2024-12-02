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

    // Ouvrir le répertoire source
    dir = opendir(source_dir);
    if (dir == NULL) {
        printf("Erreur lors de l'ouverture du répertoire source");
    }

    // Créer le répertoire de sauvegarde s'il n'existe pas
    if (mkdir(backup_dir) == -1 ) {
        printf("Erreur lors de la création du répertoire de sauvegarde");
        closedir(dir);
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

    closedir(dir);

    printf("Sauvegarde terminée avec succès dans : %s\n", backup_dir);
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
    if (output_file == NULL) {
        perror("Erreur lors de l'ouverture du fichier de sauvegarde");
        return;
    }

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

    fclose(output_file);
}


// Fonction implémentant la logique pour la sauvegarde d'un fichier
void backup_file(const char *filename) {
    /*
    */
}


// Fonction permettant la restauration du fichier backup via le tableau de chunk
void write_restored_file(const char *output_filename, Chunk *chunks, int chunk_count) {
    /*
    */
}

// Fonction pour restaurer une sauvegarde
void restore_backup(const char *backup_id, const char *restore_dir) {
    /* @param: backup_id est le chemin vers le répertoire de la sauvegarde que l'on veut restaurer
    *          restore_dir est le répertoire de destination de la restauration
    */
}