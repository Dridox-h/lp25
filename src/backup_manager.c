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
    /* @param: source_dir est le chemin vers le répertoire à sauvegarder
    *          backup_dir est le chemin vers le répertoire de sauvegarde
    */
}

// Fonction permettant d'enregistrer dans fichier le tableau de chunk dédupliqué
void write_backup_file(const char *output_filename, Chunk *chunks, int chunk_count) {
    /*
    */
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
    /*
    */
}

// Fonction pour restaurer une sauvegarde
void restore_backup(const char *backup_id, const char *restore_dir) {
    /* @param: backup_id est le chemin vers le répertoire de la sauvegarde que l'on veut restaurer
    *          restore_dir est le répertoire de destination de la restauration
    */
   // Définir une taille fixe pour le chemin
    #define MAX_PATH_LENGTH 4096

    // Ouvrir le répertoire de sauvegarde 
    DIR *backup_dir = opendir(backup_id);
    if (!backup_dir) {
        fprintf(stderr, "Impossible d'ouvrir le répertoire de sauvegarde : %s\n", backup_id);
        return;
    }

    struct dirent *entry;
    // Parcourir toutes les entrées du répertoire
    while ((entry = readdir(backup_dir)) != NULL) {
        // Ignorer les sous-répertoires et ne traiter que les fichiers réguliers
        if (entry->d_type == DT_REG) {
            // Construire le chemin complet du fichier de sauvegarde
            char backup_file_path[MAX_PATH_LENGTH];
            snprintf(backup_file_path, MAX_PATH_LENGTH, "%s/%s", backup_id, entry->d_name);

            // Ouvrir le fichier de sauvegarde
            FILE *backup_file = fopen(backup_file_path, "rb");
            if (!backup_file) {
                fprintf(stderr, "Erreur lors de l'ouverture du fichier de sauvegarde : %s\n", backup_file_path);
                continue;
            }

            // Lire le nombre de chunks dans le fichier de sauvegarde
            int chunk_count;
            fread(&chunk_count, sizeof(int), 1, backup_file);

            // Restaurer les chunks depuis le fichier
            Chunk *chunks = NULL;
            undeduplicate_file(backup_file, &chunks, &chunk_count);

            // Construire le chemin complet pour le fichier restauré
            char restored_file_path[MAX_PATH_LENGTH];
            snprintf(restored_file_path, MAX_PATH_LENGTH, "%s/%s", restore_dir, entry->d_name);

            // Écrire les données restaurées dans un fichier
            write_restored_file(restored_file_path, chunks, chunk_count);

            for (int i = 0; i < chunk_count; i++) {
                free(chunks[i].data);
            }
            free(chunks);
            fclose(backup_file);
        }
    }

    // Fermer le répertoire de sauvegarde après traitement
    closedir(backup_dir);

    // Afficher un message de confirmation
    printf("Restauration terminée dans le répertoire : %s\n", restore_dir);
}