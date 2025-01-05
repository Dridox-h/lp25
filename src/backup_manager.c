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
void create_backup(const char *source_dir, const char *backup_dir)
{
    /* @param: source_dir est le chemin vers le répertoire à sauvegarder
    *          backup_dir est le chemin vers le répertoire de sauvegarde
    */
    DIR *dir;
    struct dirent *entry;
    struct stat entry_stat;
    char source_path[PATH_MAX], backup_path[PATH_MAX];
    FILE *source_file, *backup_file;
    char buffer[CHUNK_SIZE];
    size_t bytes_read;

    if (!(dir = opendir(source_dir)))
    {
        perror("Erreur d'ouverture du répertoire source");
        return;
    }
    if (mkdir(backup_dir, 0777) == -1 && errno != EEXIST)
    {
        perror("Erreur de création du répertoire de sauvegarde");
        closedir(dir);
        return;
    }
    while (entry = readdir(dir))
    {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;
        snprintf(source_path, sizeof(source_path), "%s/%s", source_dir, entry->d_name);
        snprintf(backup_path, sizeof(backup_path), "%s/%s", backup_dir, entry->d_name);
        if (stat(source_path, &entry_stat) == -1)
        {
            fprintf(stderr, "Erreur lors de stat : %s\n", source_path);
            continue;
        }
        if (S_ISDIR(entry_stat.st_mode))
            create_backup(source_path, backup_path);
        else if (S_ISREG(entry_stat.st_mode))
        {
            source_file = fopen(source_path, "rb");
            backup_file = fopen(backup_path, "wb");
            if (!source_file || !backup_file)
            {
                perror("Erreur lors de l'ouverture des fichiers");
                if (source_file)
		                fclose(source_file);
                if (backup_file)
		                fclose(backup_file);
                continue;
            }
            while ((bytes_read = fread(buffer, 1, CHUNK_SIZE, source_file)) > 0)
                fwrite(buffer, 1, bytes_read, backup_file);
            fclose(source_file);
            fclose(backup_file);
        }
    }
		if (closedir(dir) == -1)
        perror("Erreur lors de la fermeture du répertoire source");
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

void write_restored_file(const char *output_filename, Chunk *chunks, int chunk_count)
{
    FILE *output_file;
    size_t written;

	if (!chunks || chunk_count <= 0)
    {
        fprintf(stderr, "Chunks invalides ou nombre de chunks invalide\n");
        return;
    }
    output_file = fopen(output_filename, "wb");
    if (!output_file)
    {
        fprintf(stderr, "Erreur lors de l'ouverture du fichier de sortie : %s\n", output_filename);
        return;
    }
    for (int i = 0; i < chunk_count; ++i)
    {
        written = fwrite(chunks[i].data, 1, chunks[i].size, output_file);
        if (written != chunks[i].size)
        {
            fprintf(stderr, "Erreur lors de l'écriture du chunk %d dans le fichier\n", i);
            fclose(output_file);
            return;
        }
    }
    if (fclose(output_file))
        perror("Erreur lors de la fermeture du fichier de sortie");
    else
        printf("Fichier restauré avec succès : %s\n", output_filename);
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