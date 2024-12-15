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
#include <assert.h>

// Fonction pour créer une nouvelle sauvegarde complète puis incrémentale
void create_backup(const char *source_dir, const char *backup_dir) {
    // Crée une nouvelle sauvegarde complète ou incrémentale selon l'existence de logs
    struct tm *tm_info;
    time_t t;
    char backup_name[256];
    
    // Récupérer l'heure actuelle pour nommer le dossier de la sauvegarde
    time(&t);
    tm_info = localtime(&t);
    strftime(backup_name, sizeof(backup_name), "%Y-%m-%d-%H:%M:%S", tm_info);
    
    // Créer un dossier pour la sauvegarde avec la date/heure
    char backup_path[512];
    snprintf(backup_path, sizeof(backup_path), "%s/%s", backup_dir, backup_name);
    mkdir(backup_path, 0777);

    // Vérifier si une sauvegarde précédente existe
    char log_path[512];
    snprintf(log_path, sizeof(log_path), "%s/%s/.backup_log", backup_dir, backup_name);
    
    log_t logs;
    if (access(log_path, F_OK) != -1) {
        // Il existe déjà une sauvegarde précédente
        logs = read_backup_log(log_path);
        // Effectuer une sauvegarde incrémentale en comparant avec les logs
        backup_incremental(source_dir, backup_path, &logs);
    } else {
        // Pas de sauvegarde précédente, créer une nouvelle sauvegarde complète
        backup_complete(source_dir, backup_path);
    }

    // Écrire ou mettre à jour le log de la sauvegarde
    update_backup_log(log_path, &logs);
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
void restore_backup(const char *backup_metadata_path, const char *restore_file_path, Md5Entry *hash_table, int hash_table_size) {
    // Ouvrir le fichier des métadonnées
    FILE *metadata_file = fopen(backup_metadata_path, "rb");
    if (!metadata_file) {
        printf("Erreur lors de l'ouverture du fichier de métadonnées %s : %s\n", backup_metadata_path, strerror(errno));
        return;
    }

    // Lire les informations des métadonnées et reconstruire le fichier restauré
    // Exemple de lecture simplifiée (cela dépend de la structure des métadonnées)
    
    // Extraire le nom du fichier à partir du chemin
    const char *filename = strrchr(backup_metadata_path, '/');
    if (!filename) {
        filename = backup_metadata_path;  // Pas de chemin, juste le nom
    } else {
        filename++;  // Pour ignorer le '/'
    }

    // Créer le nom du fichier restauré en ajoutant le préfixe 'restore_'
    char restore_filename[256];
    snprintf(restore_filename, sizeof(restore_filename), "%s/restore_%s", restore_file_path, filename);

    // Ouvrir le fichier restauré en écriture
    FILE *restore_file = fopen(restore_filename, "wb");
    if (!restore_file) {
        printf("Erreur lors de l'ouverture du fichier restauré %s : %s\n", restore_filename, strerror(errno));
        fclose(metadata_file);
        return;
    }

    // À ce stade, il faut savoir comment les métadonnées sont stockées et utilisées
    // Supposons que les métadonnées contiennent des informations pour accéder aux morceaux du fichier.

    // Exemple simplifié : récupération des morceaux à partir de la table de hachage
    // Lisez les entrées des métadonnées et utilisez-les pour restaurer les morceaux du fichier.
    // Cela dépend de la structure exacte de vos métadonnées, donc cet exemple peut nécessiter des ajustements.

    unsigned char chunk_md5[MD5_DIGEST_LENGTH];
    size_t chunk_size = 1024;  // Exemple de taille de chunk (1024 octets)
    void *chunk_data = malloc(chunk_size);
    if (!chunk_data) {
        printf("Erreur d'allocation mémoire pour le chunk.\n");
        fclose(metadata_file);
        fclose(restore_file);
        return;
    }

    // Lisez les métadonnées et restaurez les morceaux
    while (fread(chunk_md5, sizeof(unsigned char), MD5_DIGEST_LENGTH, metadata_file) == MD5_DIGEST_LENGTH) {
        // Rechercher dans la table de hachage pour trouver l'indice du chunk
        int chunk_index = -1;
        for (int i = 0; i < hash_table_size; i++) {
            if (memcmp(chunk_md5, hash_table[i].md5, MD5_DIGEST_LENGTH) == 0) {
                chunk_index = hash_table[i].index;
                break;
            }
        }

        if (chunk_index == -1) {
            printf("Erreur : Chunk non trouvé dans la table de hachage.\n");
            break;
        }

        // Récupérer le chunk (par exemple, depuis un fichier de sauvegarde ou d'une autre source)
        // Ici, nous devons ajouter la logique pour charger réellement les données du chunk
        // Pour l'exemple, nous générons simplement des données factices.
        memset(chunk_data, 0, chunk_size);  // Remplacer par la récupération réelle du chunk

        // Écrire le chunk restauré dans le fichier de restauration
        fwrite(chunk_data, 1, chunk_size, restore_file);
    }

    // Libérer la mémoire et fermer les fichiers
    free(chunk_data);
    fclose(metadata_file);
    fclose(restore_file);

    printf("Fichier restauré avec succès : %s\n", restore_filename);
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
