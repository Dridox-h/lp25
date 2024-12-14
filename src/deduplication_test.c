#include "deduplication.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_CHUNKS 1000000

void create_original_file() {
    FILE *file = fopen("original_file.txt", "wb");  // Mode binaire pour écriture
    if (!file) {
        fprintf(stderr, "Error opening original file\n");
        exit(EXIT_FAILURE);
    }

    // Créer un fichier avec des données répétées
    for (int i = 0; i < 5; i++) {
        fwrite("HelloWorldChunk", 1, 16, file);
    }

    fclose(file);
    printf("Original file created\n");
}

void test_deduplication_and_save() {
    FILE *file = fopen("original_file.txt", "rb");  // Mode binaire pour lecture
    if (!file) {
        fprintf(stderr, "Error opening original file for deduplication\n");
        exit(EXIT_FAILURE);
    }

    // Variables pour déduplication
    Chunk chunks[MAX_CHUNKS];
    Md5Entry hash_table[HASH_TABLE_SIZE];
    int chunk_count = 0;

    // Appliquer la déduplication
    deduplicate_file(file, chunks, hash_table, &chunk_count);

    // Vérifier que la déduplication a bien fonctionné (devrait seulement y avoir 1 chunk unique)
    assert(chunk_count == 1);
    printf("Deduplication Test PASSED\n");

    fclose(file);

    // Sauvegarder les chunks dans un fichier dupliqué
    file = fopen("duplicated_file.txt", "wb");  // Mode binaire pour écriture
    if (!file) {
        fprintf(stderr, "Error opening duplicated file\n");
        exit(EXIT_FAILURE);
    }

    // Écrire les chunks dans le fichier dupliqué
    for (int i = 0; i < chunk_count; i++) {
        fwrite(chunks[i].md5, 1, MD5_DIGEST_LENGTH, file);  // Écrire le MD5
        fwrite(chunks[i].data, 1, 16, file);  // Assumer que chaque chunk fait 16 octets
    }

    fclose(file);
    printf("Duplicated file created\n");
}

void test_undeduplication() {
    FILE *file = fopen("duplicated_file.txt", "rb");  // Mode binaire pour lecture
    if (!file) {
        fprintf(stderr, "Error opening duplicated file for undeduplication\n");
        exit(EXIT_FAILURE);
    }

    // Variables pour la gestion des chunks
    Chunk *chunks = NULL;
    int chunk_count = 0;

    // Appliquer l'undéduplication
    undeduplicate_file(file, &chunks, &chunk_count);

    // Vérifier que nous avons bien lu un chunk unique
    assert(chunk_count == 1);  // Il devrait y avoir un seul chunk après undéduplication
    printf("Undeduplication Test PASSED: Total chunks read: %d\n", chunk_count);

    // Libérer la mémoire
    for (int i = 0; i < chunk_count; i++) {
        free(chunks[i].data);
    }
    free(chunks);
    fclose(file);
}

int main() {
    printf("Running Tests...\n");

    // 1. Créer un fichier original
    create_original_file();

    // 2. Appliquer la déduplication et créer le fichier dupliqué
    test_deduplication_and_save();

    // 3. Appliquer l'undéduplication sur le fichier dupliqué
    test_undeduplication();

    printf("All Tests PASSED\n");
    return 0;
}
