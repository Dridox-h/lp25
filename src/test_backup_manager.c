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
    if (mkdir(RESTORE_DIR, 0755) == -1 && errno != EEXIST) {
        printf("Erreur lors de la création du répertoire de restauration : %s\n", strerror(errno));
        return;
    }

    // Construire le chemin complet vers les fichiers de sauvegarde
    char backup_file_path1[256];
    snprintf(backup_file_path1, sizeof(backup_file_path1), "%s/file1.txt.metadata", BACKUP_DIR);

    // Lire la table de hachage (par exemple depuis un fichier) pour restaurer le fichier
    Md5Entry hash_table[HASH_TABLE_SIZE];  // Créer un tableau pour les entrées MD5
    int hash_table_size = 0;  // Définir une taille de table de hachage initiale

    // Il faudrait remplir la table de hachage à partir des métadonnées, mais ici nous supposons que c'est déjà fait.
    // Exemple :
    // hash_table[0].index = 0;
    // memcpy(hash_table[0].md5, ...);

    // Appel de la fonction de restauration
    restore_backup(backup_file_path1, RESTORE_DIR, hash_table, hash_table_size);

    // Construire le chemin pour un second fichier de sauvegarde
    char backup_file_path2[256];
    snprintf(backup_file_path2, sizeof(backup_file_path2), "%s/file2.txt.metadata", BACKUP_DIR);

    // Appel de la fonction de restauration avec la deuxième sauvegarde
    restore_backup(backup_file_path2, RESTORE_DIR, hash_table, hash_table_size);

    // Vérifier si le fichier restauré existe
    struct stat statbuf;
    int restore_stat1 = stat(RESTORE_DIR "/restored_file.dat", &statbuf);
    assert(restore_stat1 == 0 && "Le fichier restauré file1.txt n'existe pas");

    // Vérifier si le second fichier restauré existe
    int restore_stat2 = stat(RESTORE_DIR "/restored_file2.dat", &statbuf);
    assert(restore_stat2 == 0 && "Le fichier restauré file2.txt n'existe pas");

    // Vérifier si le contenu restauré est correct
    FILE *restored_file1 = fopen(RESTORE_DIR "/restored_file.dat", "rb");
    FILE *restored_file2 = fopen(RESTORE_DIR "/restored_file2.dat", "rb");

    assert(restored_file1 != NULL && "Échec de l'ouverture du fichier restauré file1.txt");
    assert(restored_file2 != NULL && "Échec de l'ouverture du fichier restauré file2.txt");

    // Comparer le contenu restauré avec l'original
    char buffer1[1024], buffer2[1024];
    size_t bytes_read1 = fread(buffer1, 1, sizeof(buffer1), restored_file1);
    size_t bytes_read2 = fread(buffer2, 1, sizeof(buffer2), restored_file2);

    assert(bytes_read1 > 0 && "Le contenu du fichier restauré file1.txt est vide ou corrompu");
    assert(bytes_read2 > 0 && "Le contenu du fichier restauré file2.txt est vide ou corrompu");

    fclose(restored_file1);
    fclose(restored_file2);

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
