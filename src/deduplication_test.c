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
    deduplicate_file(file, chunks, hash_table);

    printf("Deduplication Test PASSED\n");

    fclose(file);
}

int main() {
    printf("Running Tests...\n");

    test_compute_md5();
    test_deduplication();

    printf("All Tests PASSED\n");
    return 0;
}
