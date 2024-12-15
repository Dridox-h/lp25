#include "deduplication.h"
#include "file_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/md5.h>

#define HASH_TABLE_SIZE 1000   // Taille de la table de hachage

// Fonction pour obtenir la date et l'heure courantes
void get_current_datetime(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d-%H:%M:%S", tm_info);
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    snprintf(buffer + strlen(buffer), size - strlen(buffer), ".%03ld", ts.tv_nsec / 1000000);
}

// Fonction pour générer un MD5 fictif (pour les tests uniquement)
void generate_fake_md5(unsigned char *md5) {
    unsigned char fake_md5[MD5_DIGEST_LENGTH] = {0x5f, 0x36, 0x7f, 0x93, 0x11, 0xa1, 0xe3, 0xf7, 
                                                 0x8f, 0x9b, 0xd3, 0x4f, 0x8d, 0xd4, 0x57, 0x6d};
    memcpy(md5, fake_md5, MD5_DIGEST_LENGTH);
}

// Fonction de test : déduplication d'un fichier en chunks
void test_deduplicate_file(const char *src_file) {
    printf("\n=== Test de déduplication du fichier %s ===\n", src_file);

    // Initialiser la table de hachage et le tableau de chunks
    Md5Entry hash_table[HASH_TABLE_SIZE] = {0};
    Chunk chunks[HASH_TABLE_SIZE] = {0};

    FILE *file = fopen(src_file, "rb");
    if (!file) {
        perror("Erreur d'ouverture du fichier source");
        return;
    }

    deduplicate_file(file, chunks, hash_table);

    printf("Chunks uniques extraits du fichier :\n");
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        if (chunks[i].data != NULL) {
            printf("Chunk %d - MD5: ", i);
            for (int j = 0; j < MD5_DIGEST_LENGTH; j++) {
                printf("%02x", chunks[i].md5[j]);
            }
            printf("\n");
            free(chunks[i].data); // Libérer la mémoire
        }
    }

    fclose(file);
}

// Fonction de test : reconstitution d'un fichier dédupliqué
void test_undeduplicate_file(const char *dedup_file) {
    printf("\n=== Test de reconstitution du fichier dédupliqué %s ===\n", dedup_file);

    Chunk *chunks = NULL;
    int chunk_count = 0;

    FILE *file = fopen(dedup_file, "rb");
    if (!file) {
        perror("Erreur d'ouverture du fichier dédupliqué");
        return;
    }

    undeduplicate_file(file, &chunks, &chunk_count);

    printf("Fichier reconstitué avec %d chunks.\n", chunk_count);

    // Libérer la mémoire allouée
    if (chunks) {
        for (int i = 0; i < chunk_count; i++) {
            free(chunks[i].data);
        }
        free(chunks);
    }

    fclose(file);
}

// Fonction de test : mise à jour du fichier de log avec déduplication
void test_update_log_with_deduplication(const char *logfile, const char *file_path) {
    printf("\n=== Test de mise à jour du log avec déduplication ===\n");

    log_element elt;
    unsigned char fake_md5[MD5_DIGEST_LENGTH];
    generate_fake_md5(fake_md5);

    elt.path = strdup(file_path);
    char mtime[64];
    get_current_datetime(mtime, sizeof(mtime));
    elt.date = strdup(mtime);
    memcpy(elt.md5, fake_md5, MD5_DIGEST_LENGTH);
    elt.next = NULL;
    elt.prev = NULL;

    log_t logs = {NULL, NULL};
    logs.head = &elt;
    logs.tail = &elt;

    update_backup_log(logfile, &logs);

    printf("Le fichier %s a été mis à jour avec un MD5 dédupliqué.\n", logfile);

    free(elt.path);
    free(elt.date);
}

// Fonction de test : lecture du fichier de log
void test_read_backup_log(const char *logfile) {
    printf("\n=== Test de lecture du fichier log %s ===\n", logfile);

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

int main() {
    const char *src_file = "file1.txt";       // Fichier source pour tester la déduplication
    const char *dedup_file = "file2.dat"; // Fichier dédupliqué
    const char *logfile = "backup_log.txt";   // Fichier log

    // Test des fonctionnalités
    test_deduplicate_file(src_file);                // Déduplication d'un fichier
    test_update_log_with_deduplication(logfile, src_file); // Mise à jour du log avec déduplication
    test_read_backup_log(logfile);                  // Lecture du fichier log
    test_undeduplicate_file(dedup_file);            // Reconstitution d'un fichier dédupliqué

    return 0;
}
