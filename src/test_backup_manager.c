#include "backup_manager.h"
#include "file_handler.h"
#include "deduplication.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>
#include <assert.h>

// Définir les chemins globalement
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

    // Liste les fichiers de sauvegarde dans BACKUP_DIR
    list_backups(BACKUP_DIR);

    printf("Test list_backups() : OK\n");
}

// Test de la fonction restore_backup()
void test_restore_backup() {
    printf("\n--- Test: restore_backup() ---\n");

    // Créer le répertoire de restauration si nécessaire
    mkdir(RESTORE_DIR, 0755);

    // Construire le chemin complet vers le fichier de sauvegarde
    char backup_file_path[256];
    snprintf(backup_file_path, sizeof(backup_file_path), "%s/file1.txt.metadata", BACKUP_DIR);  // Corrigé ici

    // Appeler restore_backup avec le chemin correct
    restore_backup(backup_file_path, RESTORE_DIR, BACKUP_DIR);

    // Vérifie si le fichier a été restauré
    struct stat statbuf;
    int restore_stat = stat(RESTORE_DIR "/restored_file.dat", &statbuf);

    // Vérifie si le fichier a bien été restauré
    assert(restore_stat == 0 && "Le fichier restauré n'existe pas");

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

    printf("=== Tous les tests ont réussi avec succès ! ===\n");
    return 0;
}
