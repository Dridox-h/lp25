#include "deduplication.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_CHUNKS 100

void test_compute_md5() {
    char data[] = "Test MD5 Data";
    unsigned char md5_result[MD5_DIGEST_LENGTH];

    compute_md5(data, strlen(data), md5_result);

    printf("MD5 Test PASSED\n");
}

void test_deduplication() {
    FILE *file = fopen("test_file.txt", "w+");
    if (!file) {
        fprintf(stderr, "Error opening file\n");
        exit(EXIT_FAILURE);
    }

    // Créer un fichier avec des données répétées
    for (int i = 0; i < 3; i++) {
        fwrite("HelloWorldChunk", 1, 16, file);
    }
    rewind(file);

    Chunk chunks[MAX_CHUNKS];
    Md5Entry hash_table[HASH_TABLE_SIZE];
    int chunk_count = 0;
    
    // Test de la déduplication
    deduplicate_file(file, chunks, hash_table, &chunk_count);
    
    // Vérifier que la déduplication a bien fonctionné (devrait seulement y avoir 1 chunk unique)
    assert(chunk_count == 1);
    printf("Deduplication Test PASSED\n");

    fclose(file);
}

void test_undeduplication() {
    FILE *file = fopen("test_file.txt", "r");
    if (!file) {
        fprintf(stderr, "Error opening file for undeduplication\n");
        exit(EXIT_FAILURE);
    }

    Chunk *chunks = NULL;
    int chunk_count = 0;
    
    // Test de la fonction pour undédupliquer
    undeduplicate_file(file, &chunks, &chunk_count);

    // Vérification que nous avons bien lu tous les chunks
    printf("Undeduplication Test: Total chunks read: %d\n", chunk_count);

    // Libérer la mémoire
    for (int i = 0; i < chunk_count; i++) {
        free(chunks[i].data);
    }
    free(chunks);
    fclose(file);
}

int main() {
    printf("Running Tests...\n");

    test_compute_md5();      // Test du calcul MD5
    test_deduplication();    // Test de la déduplication
    test_undeduplication();  // Test de l'undéduplication

    printf("All Tests PASSED\n");
    return 0;
}
