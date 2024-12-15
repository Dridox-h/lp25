#ifndef DEDUPLICATION_H
#define DEDUPLICATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include <dirent.h>

// Taille d'un chunk (4096 octets)
#define CHUNK_SIZE 4096

// Taille de la table de hachage
#define HASH_TABLE_SIZE 1000

// Structure pour un chunk
typedef struct {
    unsigned char md5[MD5_DIGEST_LENGTH]; // MD5 du chunk
    void *data;                           // Données du chunk
    size_t size;                          // Taille du chunk
} Chunk;

// Table de hachage pour stocker les MD5 et leurs index
typedef struct {
    unsigned char md5[MD5_DIGEST_LENGTH];
    int index;
} Md5Entry;

// Fonction de hachage MD5 pour l'indexation dans la table de hachage
unsigned int hash_md5(unsigned char *md5);
void compute_md5(void *data, size_t len, unsigned char *md5_out);
int find_md5(Md5Entry *hash_table, unsigned char *md5);
void add_md5(Md5Entry *hash_table, unsigned char *md5, int index);
void deduplicate_file(FILE *file, Chunk *chunks, Md5Entry *hash_table);

#endif // DEDUPLICATION_H