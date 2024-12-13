#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h> // Nécessaire pour MD5_DIGEST_LENGTH
#include <assert.h>
#include "file_handler.h" // Inclut tes fonctions
#include "deduplication.h" // Assure-toi d'avoir les structures log_t et log_element

#define TEST_LOG_FILE "test_backup_log.log"

// Fonction pour générer un hachage MD5 factice (à utiliser pour les tests)
void generate_dummy_md5(unsigned char *md5) {
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        md5[i] = i; // Remplit le tableau avec des valeurs simples pour le test
    }
}

// Test de la fonction read_backup_log() et update_backup_log()
void test_read_update_backup_log() {
    printf("\nTest: read_backup_log() et update_backup_log()\n");

    // Création manuelle d'une liste log_t
    log_t logs = { .head = NULL, .tail = NULL };

    log_element *elt1 = malloc(sizeof(log_element));
    elt1->path = strdup("/chemin/fichier1.txt");
    elt1->date = strdup("2024-06-01");
    generate_dummy_md5(elt1->md5);
    elt1->next = NULL;
    elt1->prev = NULL;

    logs.head = logs.tail = elt1;

    log_element *elt2 = malloc(sizeof(log_element));
    elt2->path = strdup("/chemin/fichier2.txt");
    elt2->date = strdup("2024-06-02");
    generate_dummy_md5(elt2->md5);
    elt2->next = NULL;
    elt2->prev = logs.tail;

    logs.tail->next = elt2;
    logs.tail = elt2;

    // Mise à jour du fichier log avec les données créées
    update_backup_log(TEST_LOG_FILE, &logs);

    // Lecture du fichier log
    log_t read_logs = read_backup_log(TEST_LOG_FILE);

    // Assertions pour vérifier l'intégrité des données
    log_element *read_elt = read_logs.head;
    assert(strcmp(read_elt->path, "/chemin/fichier1.txt") == 0);
    assert(strcmp(read_elt->date, "2024-06-01") == 0);

    read_elt = read_elt->next;
    assert(strcmp(read_elt->path, "/chemin/fichier2.txt") == 0);
    assert(strcmp(read_elt->date, "2024-06-02") == 0);

    printf("read_backup_log() et update_backup_log() ont réussi !\n");

    // Libération de la mémoire
    free(elt1->path);
    free(elt1->date);
    free(elt1);
    free(elt2->path);
    free(elt2->date);
    free(elt2);
}

// Test de la fonction list_files()
void test_list_files() {
    printf("\nTest: list_files()\n");

    // Utilise le répertoire courant pour le test
    const char *test_directory = ".";

    printf("Liste des fichiers dans le répertoire courant :\n");
    list_files(test_directory);

    printf("list_files() a été exécuté avec succès.\n");
}

int main() {
    printf("=== Instructions pour les tests ===\n");
    printf("1. Compile le programme avec la commande suivante :\n");
    printf("   gcc -o test_file_handler file_handler.c test_file_handler.c -lssl -lcrypto\n");
    printf("\n2. Exécute le programme avec la commande suivante :\n");
    printf("   ./test_file_handler\n");
    printf("\n3. Vérifie que les sorties des tests sont correctes.\n");

    printf("\n=== Début des tests pour file_handler ===\n");

    // Test des fonctions de log
    test_read_update_backup_log();

    // Test de la liste des fichiers
    test_list_files();

    printf("\n=== Tous les tests ont réussi avec succès ! ===\n");
    return 0;
}
