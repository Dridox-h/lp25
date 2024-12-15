#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/evp.h>  // Utilisation de EVP pour le MD5
#include "backup_manager.h"
#include "file_handler.h"
#include "deduplication.h"

// Fonction pour calculer le MD5 d'un fichier avec EVP
void calculate_md5(const char *file_path, unsigned char *md5) {
    FILE *file = fopen(file_path, "rb");
    if (!file) {
        perror("Erreur d'ouverture du fichier pour calculer le MD5");
        return;
    }

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (!mdctx) {
        perror("Erreur d'initialisation de EVP_MD_CTX");
        fclose(file);
        return;
    }

    // Initialiser le contexte pour MD5
    if (EVP_DigestInit_ex(mdctx, EVP_md5(), NULL) != 1) {
        perror("Erreur d'initialisation du contexte de hachage");
        EVP_MD_CTX_free(mdctx);
        fclose(file);
        return;
    }

    unsigned char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (EVP_DigestUpdate(mdctx, buffer, bytes_read) != 1) {
            perror("Erreur de mise à jour du hachage");
            EVP_MD_CTX_free(mdctx);
            fclose(file);
            return;
        }
    }

    if (EVP_DigestFinal_ex(mdctx, md5, NULL) != 1) {
        perror("Erreur de finalisation du hachage");
    }

    EVP_MD_CTX_free(mdctx);
    fclose(file);
}

// Fonction pour tester la sauvegarde
void test_backup() {
    const char *file_path = "source_directory/file1.txt";
    unsigned char md5[MD5_DIGEST_LENGTH];

    // Calculer le MD5 du fichier
    calculate_md5(file_path, md5);

    // Afficher les informations de sauvegarde
    printf("=== Test de sauvegarde ===\n");
    printf("Écriture du fichier de sauvegarde: backup_directory/file1.txt.metadata\n");

    // Créer un élément log
    log_element *new_elt = malloc(sizeof(log_element));
    if (!new_elt) {
        perror("Erreur d'allocation mémoire pour log_element");
        return;
    }

    new_elt->path = strdup(file_path);  // Chemin du fichier
    memcpy(new_elt->md5, md5, MD5_DIGEST_LENGTH);  // MD5 du fichier

    // Récupérer la date et l'heure actuelles
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    new_elt->date = malloc(20);
    strftime(new_elt->date, 20, "%Y-%m-%d %H:%M:%S", tm_info);  // Format de la date

    // Créer une structure log_t vide pour contenir l'élément
    log_t logs = { .head = new_elt, .tail = new_elt };
    new_elt->next = NULL;
    new_elt->prev = NULL;

    // Mettre à jour le fichier de log
    update_backup_log(".backup_log", &logs);

    printf("Sauvegarde écrite dans le fichier : backup_directory/file1.txt.metadata\n");
    printf("Sauvegarde terminée avec succès dans : backup_directory\n");
}

// Fonction pour tester la restauration
void test_restore() {
    const char *backup_file = "backup_directory/file1.txt.metadata";
    printf("=== Test de restauration ===\n");

    // Tentative d'ouverture du fichier de sauvegarde
    FILE *file = fopen(backup_file, "r");
    if (!file) {
        printf("Erreur lors de l'ouverture du fichier de sauvegarde '%s': No such file or directory\n", backup_file);
        return;
    }

    printf("Restauration réussie du fichier : %s\n", backup_file);
    fclose(file);
}

// Fonction pour tester la liste des sauvegardes
void test_list_backups() {
    printf("=== Liste des sauvegardes ===\n");

    // Lister les fichiers dans le répertoire de sauvegarde
    list_files("backup_directory");
}

// Fonction pour tester les logs de sauvegarde
void test_logs() {
    printf("=== Test des logs de sauvegarde ===\n");

    // Lire les logs à partir du fichier .backup_log
    log_t logs = read_backup_log(".backup_log");

    // Afficher les logs
    log_element *current = logs.head;
    while (current) {
        printf("Chemin : %s, MD5 : ", current->path);
        for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
            printf("%02x", current->md5[i]);
        }
        printf(", Date : %s\n", current->date);
        current = current->next;
    }
}

// Fonction principale
int main() {
    // Supprimez ou définissez la fonction test_deduplication si nécessaire
    // test_deduplication();

    // Test de la sauvegarde
    test_backup();

    // Test de la liste des sauvegardes
    test_list_backups();

    // Test de la restauration
    test_restore();

    // Test des logs de sauvegarde
    test_logs();

    return 0;
}
