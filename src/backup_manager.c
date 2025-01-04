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
    // Ouvrir le fichier en mode binaire pour écriture
    FILE *file = fopen(output_filename, "wb");

    if (!file) {
        perror("Erreur lors de l'ouverture du fichier de sauvegarde");
        return;
    }

    // Écrire le nombre total de chunks dans le fichier
    if (fwrite(&chunk_count, sizeof(int), 1, file) != 1) {
        perror("Erreur lors de l'écriture du nombre de chunks");
        fclose(file);
        return;
    }

    // Écrire chaque chunk dans le fichier
    for (int i = 0; i < chunk_count; ++i) {
        Chunk *chunk = &chunks[i];

        // Écrire la taille des données du chunk
        if (fwrite(&chunk->size, sizeof(size_t), 1, file) != 1) {
            perror("Erreur lors de l'écriture de la taille du chunk");
            fclose(file);
            return;
        }

        // Écrire le MD5 du chunk
        if (fwrite(chunk->md5, MD5_DIGEST_LENGTH, 1, file) != 1) {
            perror("Erreur lors de l'écriture du MD5 du chunk");
            fclose(file);
            return;
        }

        if (fwrite(chunk->data, 1, chunk->size, file) != chunk->size) {
            perror("Erreur lors de l'écriture des données du chunk");
            fclose(file);
            return;
        }
    }

    fclose(file);
    printf("Sauvegarde écrite avec succès dans %s\n", output_filename);
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