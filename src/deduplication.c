#include "deduplication.h"
#include "file_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include <dirent.h>

// Fonction de hachage MD5 pour l'indexation dans la table de hachage
unsigned int hash_md5(unsigned char *md5) {
    unsigned int hash = 0;
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        hash = (hash << 5) + hash + md5[i];
    }
    return hash % HASH_TABLE_SIZE;
}

// Fonction pour calculer le MD5 d'un chunk
void compute_md5(void *data, size_t len, unsigned char *md5_out) {
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, data, len);
    MD5_Final(md5_out, &ctx);
}

// Fonction permettant de chercher un MD5 dans la table de hachage
int find_md5(Md5Entry *hash_table, unsigned char *md5) {
    unsigned int index = hash_md5(md5);
    while (hash_table[index].index != -1) {
        if (memcmp(hash_table[index].md5, md5, MD5_DIGEST_LENGTH) == 0) {
            return hash_table[index].index;
        }
        index = (index + 1) % HASH_TABLE_SIZE; // Probing linéaire en cas de collision
    }
    return -1; // Non trouvé
}

// Ajouter un MD5 dans la table de hachage
void add_md5(Md5Entry *hash_table, unsigned char *md5, int index) {
    unsigned int hash_index = hash_md5(md5);
    while (hash_table[hash_index].index != -1) {
        hash_index = (hash_index + 1) % HASH_TABLE_SIZE; // Probing linéaire en cas de collision
    }
    memcpy(hash_table[hash_index].md5, md5, MD5_DIGEST_LENGTH);
    hash_table[hash_index].index = index;
}

// Fonction pour convertir un fichier non dédupliqué en tableau de chunks
void deduplicate_file(FILE *file, Chunk *chunks, Md5Entry *hash_table) {
    unsigned char buffer[CHUNK_SIZE];
    int chunk_index = 0;
    int chunk_count = 0;

    // Initialisation de la table de hachage
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        hash_table[i].index = -1;
    }

    while (!feof(file)) {
        size_t bytes_read = fread(buffer, 1, CHUNK_SIZE, file);
        if (bytes_read == 0) break;

        // Calculer le MD5 du chunk
        unsigned char md5[MD5_DIGEST_LENGTH];
        compute_md5(buffer, bytes_read, md5);

        // Vérifier si le chunk est déjà dans la table de hachage
        int existing_index = find_md5(hash_table, md5);
        if (existing_index == -1) {
            // Chunk unique : l'ajouter à la table de hachage et au tableau
            add_md5(hash_table, md5, chunk_count);

            chunks[chunk_count].data = malloc(bytes_read);
            memcpy(chunks[chunk_count].data, buffer, bytes_read);
            memcpy(chunks[chunk_count].md5, md5, MD5_DIGEST_LENGTH);

            chunk_count++;
        }

        chunk_index++;
    }
}

// Fonction permettant de charger un fichier dédupliqué en table de chunks
// en remplaçant les références par les données correspondantes
void undeduplicate_file(FILE *file, Chunk **chunks, int *chunk_count) {
    fread(chunk_count, sizeof(int), 1, file);

    *chunks = malloc(*chunk_count * sizeof(Chunk));

    for (int i = 0; i < *chunk_count; i++) {
        // Lire la taille et les données du chunk
        size_t chunk_size;
        fread(&chunk_size, sizeof(size_t), 1, file);

        (*chunks)[i].data = malloc(chunk_size);
        fread((*chunks)[i].data, 1, chunk_size, file);

        // Lire le MD5 du chunk
        fread((*chunks)[i].md5, 1, MD5_DIGEST_LENGTH, file);
    }
}
