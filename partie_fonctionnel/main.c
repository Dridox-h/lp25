#include "deduplication.h"
#include "file_handler.h"
#include "backup_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    const char *source_dir = "source_directory";  // Répertoire source pour la sauvegarde
    const char *backup_dir = "backup_directory";  // Répertoire où la sauvegarde sera stockée
    const char *restore_dir = "restore_directory"; // Répertoire de restauration
    const char *backup_id = "backup_id_example";   // Identifiant de sauvegarde pour la restauration

    // Test 1: Déduplication d'un fichier
    printf("\n=== Test de déduplication ===\n");
    FILE *file_to_deduplicate = fopen("source_directory/test_file.txt", "rb");
    if (file_to_deduplicate) {
        Chunk chunks[10000];  // Tableau pour stocker les chunks
        int chunk_count = 0;
        Md5Entry hash_table[HASH_TABLE_SIZE] = {0};
        deduplicate_file(file_to_deduplicate, chunks, hash_table, &chunk_count); // Appel à la déduplication
        fclose(file_to_deduplicate);
        printf("Déduplication terminée. Nombre de chunks : %d\n", chunk_count);
    } else {
        printf("Erreur lors de l'ouverture du fichier pour la déduplication.\n");
    }

    // Test 2: Sauvegarde complète et incrémentale
    printf("\n=== Test de sauvegarde ===\n");
    create_backup(source_dir, backup_dir); // Créer une sauvegarde

    // Test 3: Liste des sauvegardes existantes
    printf("\n=== Liste des sauvegardes ===\n");
    list_backups(backup_dir); // Lister les sauvegardes existantes

    // Test 4: Restauration d'une sauvegarde
    printf("\n=== Test de restauration ===\n");
    restore_backup(backup_id, restore_dir, backup_dir);  // Restaurer une sauvegarde

    return 0;
}
