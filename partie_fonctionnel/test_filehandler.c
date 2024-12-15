#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "file_handler.h"

// Fonction pour obtenir la date et l'heure actuelles dans le format souhaité
void get_current_datetime(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d-%H:%M:%S", tm_info);
    // Ajouter la partie millisecondes
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    snprintf(buffer + strlen(buffer), size - strlen(buffer), ".%03ld", ts.tv_nsec / 1000000);
}

// Fonction pour générer un MD5 fictif pour un fichier
void generate_fake_md5(unsigned char *md5) {
    // Valeur fictive de MD5 pour ce test-
    unsigned char fake_md5[MD5_DIGEST_LENGTH] = {0x5f, 0x36, 0x7f, 0x93, 0x11, 0xa1, 0xe3, 0xf7, 0x8f, 0x9b, 0xd3, 0x4f, 0x8d, 0xd4, 0x57, 0x6d};
    memcpy(md5, fake_md5, MD5_DIGEST_LENGTH);
}

// Fonction pour tester la mise à jour du fichier .backup_log
void test_update_backup_log(const char *logfile) {
    printf("=== Test de mise à jour du fichier .backup_log ===\n");

    // Création d'un élément de log fictif
    log_element elt;
    unsigned char fake_md5[MD5_DIGEST_LENGTH];
    generate_fake_md5(fake_md5);
    elt.path = "folder1/file1";
    
    // Génération de la date de dernière modification (mtime)
    char mtime[64];
    get_current_datetime(mtime, sizeof(mtime));
    elt.date = strdup(mtime); // Date courante dans le format YYYY-MM-DD-hh:mm:ss.sss
    
    memcpy(elt.md5, fake_md5, MD5_DIGEST_LENGTH);
    elt.next = NULL;
    elt.prev = NULL;

    // Ajout à une structure log_t
    log_t logs = {NULL, NULL};
    logs.head = &elt;
    logs.tail = &elt;

    // Mise à jour du fichier .backup_log
    update_backup_log(logfile, &logs);
    printf("Le fichier %s a été mis à jour avec un nouvel élément.\n", logfile);
}

// Fonction pour tester la lecture du fichier .backup_log
void test_read_backup_log(const char *logfile) {
    printf("=== Test de lecture du fichier .backup_log ===\n");
    log_t logs = read_backup_log(logfile);
    
    if (logs.head == NULL) {
        printf("Aucun élément trouvé dans le fichier de log.\n");
        return;
    }
    
    log_element *elt = logs.head;
    while (elt) {
        printf("Chemin: %s\n", elt->path);
        printf("Date: %s\n", elt->date);
        printf("MD5: ");
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
            printf("%02x", elt->md5[i]);
        }
        printf("\n");
        elt = elt->next;
    }
}

void test_list_files(const char *path) {
    printf("=== Test de list_files ===\n");
    list_files(path);
}

void test_copy_file(const char *src, const char *dest) {
    printf("=== Test de copy_file ===\n");
    copy_file(src, dest);
    printf("Fichier copié de %s vers %s.\n", src, dest);
}

int main() {
    const char *logfile = "backup_log.txt"; // Nom du fichier de log pour tester
    const char *test_dir = "."; // Dossier actuel pour tester list_files
    const char *src_file = "file1.txt"; // Fichier source pour tester copy_file
    const char *dest_file = "destination.txt"; // Fichier destination pour tester copy_file
    
    // Test des fonctions
    test_update_backup_log(logfile);  // Test de la mise à jour du fichier .backup_log
    test_read_backup_log(logfile);    // Test de la lecture du fichier .backup_log
    test_list_files(test_dir);        // Test de la fonction list_files (lister les fichiers du répertoire actuel)
    test_copy_file(src_file, dest_file); // Test de la fonction copy_file
    
    return 0;
}
