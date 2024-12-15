#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <openssl/md5.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <libgen.h>
#include <pwd.h>
#include <grp.h>
#include <sys/xattr.h>
#include "backup_manager.h"
#include "deduplication.h"
#include "file_handler.h"

// Prototypes des fonctions nécessaires
void create_backup(const char *source_dir, const char *backup_dir);
void list_directory(const char *dir);
int compare_directories_verbose(const char *dir1, const char *dir2);

// =========================
// Fonctions Utilitaires pour Tests
// =========================

void setup_complex_test_environment() {
    // Nettoyer les dossiers précédents
    system("rm -rf test_source test_backup");

    // Création des dossiers de test
    system("mkdir -p test_source/level1/level2/level3 test_backup");

    // Fichiers de petite taille
    FILE *file1 = fopen("test_source/file1.txt", "w");
    fprintf(file1, "Contenu du fichier 1");
    fclose(file1);

    FILE *file2 = fopen("test_source/file2.bin", "w");
    for (int i = 0; i < 1024; ++i) { // 1 Ko de données binaires
        fputc((i % 256), file2);
    }
    fclose(file2);

    // Fichiers dans des sous-répertoires
    FILE *file3 = fopen("test_source/level1/level2/level3/nested_file.txt", "w");
    fprintf(file3, "Fichier dans un sous-dossier");
    fclose(file3);

    // Fichiers vides
    FILE *empty_file = fopen("test_source/empty_file.txt", "w");
    fclose(empty_file);

    // Fichiers avec des permissions restreintes
    FILE *restricted_file = fopen("test_source/restricted_file.txt", "w");
    fprintf(restricted_file, "Fichier avec des permissions restreintes");
    fclose(restricted_file);
    chmod("test_source/restricted_file.txt", 0400); // Lecture seule

    printf("Environnement de test complexe créé.\n");
}

void run_advanced_backup_test() {
    printf("\nTest : Sauvegarde initiale de l'environnement avancé\n");
    create_backup("test_source", "test_backup");

    // Modifiez un fichier existant
    FILE *file_modified = fopen("test_source/file1.txt", "a");
    fprintf(file_modified, "\nContenu modifié dans file1.txt");
    fclose(file_modified);

    // Supprimez un fichier
    remove("test_source/empty_file.txt");

    // Ajoutez un nouveau fichier
    FILE *new_file = fopen("test_source/new_file.txt", "w");
    fprintf(new_file, "Nouveau fichier ajouté");
    fclose(new_file);

    printf("\nTest : Sauvegarde incrémentale après modifications\n");
    create_backup("test_source", "test_backup");
}

int compare_directories_verbose(const char *dir1, const char *dir2) {
    char command[1024];
    snprintf(command, sizeof(command), "diff -r %s %s", dir1, dir2);
    int result = system(command);

    if (result == 0) {
        printf("\nLes répertoires sont identiques.\n");
    } else {
        printf("\nDifférences détectées entre les répertoires ! Voici le détail :\n");
        system("diff -qr test_source test_backup");
    }
    return result;
}

void list_directory(const char *dir) {
    char command[1024];
    snprintf(command, sizeof(command), "tree %s", dir);
    system(command);
}

// =========================
// Main : Tests Complets
// =========================

int main() {
    printf("\n=== Tests de Sauvegarde : Environnement Basique ===\n");
    setup_complex_test_environment();
    printf("\nTest : Sauvegarde initiale\n");
    create_backup("test_source", "test_backup");

    // Affichage des résultats de base
    printf("\nListe des fichiers dans la source après sauvegarde initiale :\n");
    list_directory("test_source");

    printf("\nListe des fichiers dans la sauvegarde après sauvegarde initiale :\n");
    list_directory("test_backup");

    printf("\nComparaison des répertoires après sauvegarde initiale :\n");
    compare_directories_verbose("test_source", "test_backup");

    printf("\n=== Tests de Sauvegarde : Environnement Avancé ===\n");
    run_advanced_backup_test();

    // Affichage des résultats après modifications
    printf("\nListe des fichiers dans la source après sauvegarde incrémentale :\n");
    list_directory("test_source");

    printf("\nListe des fichiers dans la sauvegarde après sauvegarde incrémentale :\n");
    list_directory("test_backup");

    printf("\nComparaison des répertoires après sauvegarde incrémentale :\n");
    compare_directories_verbose("test_source", "test_backup");

    return 0;
}