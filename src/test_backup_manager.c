#include "backup_manager.h"
#include "file_handler.h"
#include "deduplication.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <unistd.h>

#define SOURCE_DIR "test_source"
#define BACKUP_DIR "test_backup"
#define RESTORE_DIR "test_restore"

// Fonction pour créer des fichiers de test
void create_test_files() {
    mkdir(SOURCE_DIR, 0755);
    FILE *file1 = fopen(SOURCE_DIR "/file1.txt", "w");
    fprintf(file1, "Contenu de fichier 1\n");
    fclose(file1);

    FILE *file2 = fopen(SOURCE_DIR "/file2.txt", "w");
    fprintf(file2, "Contenu de fichier 2\n");
    fclose(file2);

    printf("Fichiers de test créés dans le répertoire : %s\n", SOURCE_DIR);
}

// Test de la fonction create_backup()
void test_create_backup() {
    printf("\n--- Test: create_backup() ---\n");

    mkdir(BACKUP_DIR, 0755);
    create_backup(SOURCE_DIR, BACKUP_DIR);

    // Vérifie si des fichiers de sauvegarde ont été créés
    struct stat statbuf;
    assert(stat(BACKUP_DIR "/file1.txt.metadata", &statbuf) == 0);
    assert(stat(BACKUP_DIR "/file2.txt.metadata", &statbuf) == 0);

    printf("Test create_backup() : OK\n");
}

// Test de la fonction list_backups()
void test_list_backups() {
    printf("\n--- Test: list_backups() ---\n");

    list_backups(".");

    printf("Test list_backups() : OK\n");
}

// Test de la fonction restore_backup()
void test_restore_backup() {
    printf("\n--- Test: restore_backup() ---\n");

    mkdir(RESTORE_DIR, 0755);
    restore_backup("file1.txt.metadata", RESTORE_DIR);

    // Vérifie si le fichier a été restauré
    struct stat statbuf;
    assert(stat(RESTORE_DIR "/restored_file.dat", &statbuf) == 0);

    printf("Test restore_backup() : OK\n");
}

// Fonction principale pour exécuter tous les tests
int main() {
    printf("=== Début des tests pour backup_manager ===\n");

    // Création des fichiers de test
    create_test_files();

    // Test des différentes fonctionnalités
    test_create_backup();
    test_list_backups();
    test_restore_backup();

    // Nettoyage
    printf("\nNettoyage des répertoires de test...\n");
    char cleanup_cmd[256];
    snprintf(cleanup_cmd, sizeof(cleanup_cmd), "rm -rf %s %s %s", SOURCE_DIR, BACKUP_DIR, RESTORE_DIR);
    system(cleanup_cmd);

    printf("=== Tous les tests ont réussi avec succès ! ===\n");
    return 0;
}
