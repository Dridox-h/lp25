#ifndef DEDUPLICATION_H
#define DEDUPLICATION_H

#include <stdio.h>
#include <stddef.h>
#include <openssl/md5.h>

#define CHUNK_SIZE 1024
#define HASH_TABLE_SIZE 1024

// Structure pour représenter un chunk
typedef struct {
    void *data;
    size_t size;
    unsigned char md5[MD5_DIGEST_LENGTH];
} Chunk;

// Structure pour l'entrée de la table de hachage
typedef struct {
    unsigned char md5[MD5_DIGEST_LENGTH];
    int index;
} Md5Entry;

// Prototypes des fonctions
void compute_md5(void *data, size_t len, unsigned char *md5_out);
void deduplicate_file(FILE *file, Chunk *chunks, Md5Entry *hash_table, int *chunk_count);
void undeduplicate_file(FILE *backup_file, Chunk **chunks, int *chunk_count);
void free_chunks(Chunk *chunks, int chunk_count);

#endif // DEDUPLICATION_H
