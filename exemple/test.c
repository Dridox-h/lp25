#include <stdio.h>
#include <stdlib.h>
#include "../src/backup_manager.h"
#include "../src/deduplication.h"
#include "../src/file_handler.h"

#define TEST_SRC_DIR "./test_src"
#define TEST_BACKUP_DIR "./backup_dir"
#define RESTORED_FILE "restored_file.txt"

//gcc test.c ../src/backup_manager.c ../src/deduplication.c ../src/file_handler.c -lcrypto

int main()
{
    printf("\n==== Début des tests du système de sauvegarde ====\n");

    // Test de la sauvegarde et de la restauration
    printf("\n==== Test de sauvegarde et restauration ====\n");
    // Création de la sauvegarde
    create_backup(TEST_SRC_DIR, TEST_BACKUP_DIR);
    // Restauration à partir de la sauvegarde
    restore_backup(TEST_BACKUP_DIR, RESTORED_FILE);
    

    // Test de la déduplication
    printf("\n==== Test de déduplication ====\n");
    // Lecture du fichier de log
    log_t log = read_backup_log(TEST_BACKUP_DIR);
    // Parcours de la liste de logs
    log_element *current = log.head;
    size_t count = 0;
    while (current)
    {
        printf("Fichier : %s\n", current->path);
        printf("MD5 : ");
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
            printf("%02x", current->md5[i]);
        printf("\nDate de modification : %s\n", current->date);
        current = current->next;
        count++;
    }
    printf("Nombre total d'éléments dans le log : %zu\n", count);

    printf("\n==== Tous les tests sont terminés ====\n");
}
