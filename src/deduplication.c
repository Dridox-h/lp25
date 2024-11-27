#include "deduplication.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>

#define HASH_SIZE MD5_DIGEST_LENGTH
#define CHUNK_SIZE 4096

typedef struct {
    unsigned char hash[HASH_SIZE];
    long offset;
} FileChunk;

void deduplicate_files(const char *source, const char *destination) {
    FILE *sourceFile = fopen(source, "rb");
    if (sourceFile == NULL) {
        perror("Erreur lors de l'ouverture du fichier source");
        return;
    }

    FILE *destFile = fopen(destination, "wb");
    if (destFile == NULL) {
        perror("Erreur lors de l'ouverture du fichier de destination");
        fclose(sourceFile);
        return;
    }

    unsigned char *buffer = malloc(CHUNK_SIZE);
    if (buffer == NULL) {
        perror("Erreur d'allocation mémoire");
        fclose(sourceFile);
        fclose(destFile);
        return;
    }

    FileChunk *chunks = NULL;
    size_t chunkCount = 0;
    size_t bytesRead;
    long offset = 0;

    while ((bytesRead = fread(buffer, 1, CHUNK_SIZE, sourceFile)) > 0) {
        unsigned char md5_sum[HASH_SIZE];
        calculate_md5(buffer, bytesRead, md5_sum);

        int isDuplicate = 0;
        for (size_t i = 0; i < chunkCount; i++) {
            if (memcmp(chunks[i].hash, md5_sum, HASH_SIZE) == 0) {
                // Chunk dupliqué trouvé
                fwrite(&chunks[i].offset, sizeof(long), 1, destFile);
                isDuplicate = 1;
                break;
            }
        }

        if (!isDuplicate) {
            // Nouveau chunk
            chunks = realloc(chunks, (chunkCount + 1) * sizeof(FileChunk));
            if (chunks == NULL) {
                perror("Erreur de réallocation mémoire");
                break;
            }
            memcpy(chunks[chunkCount].hash, md5_sum, HASH_SIZE);
            chunks[chunkCount].offset = offset;
            chunkCount++;

            fwrite(buffer, 1, bytesRead, destFile);
            offset += bytesRead;
        }
    }

    free(buffer);
    free(chunks);
    fclose(sourceFile);
    fclose(destFile);
}