#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include <sys/stat.h>
#include "../src/file_handler.h"
#include "../src/deduplication.h"
#include "../src/backup_manager.h"

#define TEST_SRC_DIR "./test_src"
#define TEST_BACKUP_DIR "./backup_dir"
#define RESTORED_DIR "./restored_dir"

// Préparation de l'environnement de test
void setup_test_environment() {
    system("mkdir -p ./test_src");
    system("mkdir -p ./backup_dir");
    system("echo 'File 1 content' > ./test_src/file1.txt");
    system("echo 'File 2 content' > ./test_src/file2.txt");
    system("touch ./test_src/empty.txt");
    system("mkdir -p ./test_src/subdir");
    system("echo 'Subdir file content' > ./test_src/subdir/file3.txt");
}

// Nettoyage de l'environnement de test
void clean_test_environment() {
    system("rm -rf ./test_src ./backup_dir ./restored_dir");
}

// Test de la sauvegarde et de la restauration
void test_backup_and_restore() {
    printf("\n==== Test de sauvegarde et restauration ====\n");

    create_backup(TEST_SRC_DIR, TEST_BACKUP_DIR);

    printf("Sauvegarde terminée dans %s\n", TEST_BACKUP_DIR);

    system("mkdir -p ./restored_dir");
    restore_backup(TEST_BACKUP_DIR, RESTORED_DIR);

    printf("Restauration terminée dans %s\n", RESTORED_DIR);
}

// Test de la déduplication
void test_deduplication() {
    printf("\n==== Test de déduplication ====\n");

    log_t log = read_backup_log(TEST_BACKUP_DIR);
    if (log.head == NULL) {
        fprintf(stderr, "Erreur : le log est vide.\n");
        return;
    }

    log_element *current = log.head;
    size_t count = 0;

    while (current) {
        printf("Fichier : %s\n", current->path);
        printf("MD5 : ");
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
            printf("%02x", current->md5[i]);
        }
        printf("\nDate de modification : %s\n", current->date);
        current = current->next;
        count++;
    }

    printf("Nombre total d'éléments dans le log : %zu\n", count);
}

// Test des fonctions de gestion des fichiers
void test_file_handler() {
    printf("\n==== Test des fonctionnalités de gestion des fichiers ====\n");

    printf("Liste des fichiers dans %s :\n", TEST_SRC_DIR);
    list_files(TEST_SRC_DIR);

    struct stat statbuf;
    const char *file_path = "./test_src/file1.txt";

    if (stat(file_path, &statbuf) == 0) {
        printf("Le fichier %s existe et a une taille de %ld octets.\n", file_path, statbuf.st_size);
    } else {
        fprintf(stderr, "Erreur : impossible de récupérer les informations pour %s\n", file_path);
    }
}

// Test global du système
void test_complete_system() {
    printf("\n==== Test complet du système ====\n");

    setup_test_environment();

    test_backup_and_restore();
    test_deduplication();
    test_file_handler();

    clean_test_environment();

    printf("\n==== Tous les tests sont terminés avec succès ====\n");
}

int main() {
    test_complete_system();
    return 0;
}
