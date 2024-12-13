#include "deduplication.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
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
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (ctx == NULL) {
        fprintf(stderr, "Error creating EVP_MD_CTX\n");
        exit(EXIT_FAILURE);
    }

    if (EVP_DigestInit_ex(ctx, EVP_md5(), NULL) != 1 ||
        EVP_DigestUpdate(ctx, data, len) != 1 ||
        EVP_DigestFinal_ex(ctx, md5_out, NULL) != 1) {
        fprintf(stderr, "Error calculating MD5\n");
        EVP_MD_CTX_free(ctx);
        exit(EXIT_FAILURE);
    }

    EVP_MD_CTX_free(ctx);
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
        hash_index = (hash_index + 1) % HASH_TABLE_SIZE;
    }
    memcpy(hash_table[hash_index].md5, md5, MD5_DIGEST_LENGTH);
    hash_table[hash_index].index = index;
}

// Fonction pour convertir un fichier non dédupliqué en tableau de chunks
void deduplicate_file(FILE *file, Chunk *chunks, Md5Entry *hash_table) {
    unsigned char buffer[CHUNK_SIZE];
    int chunk_count = 0;

    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        hash_table[i].index = -1;
    }

    while (!feof(file)) {
        size_t bytes_read = fread(buffer, 1, CHUNK_SIZE, file);
        if (bytes_read == 0) break;

        unsigned char md5[MD5_DIGEST_LENGTH];
        compute_md5(buffer, bytes_read, md5);

        int existing_index = find_md5(hash_table, md5);
        if (existing_index == -1) {
            add_md5(hash_table, md5, chunk_count);

            chunks[chunk_count].data = malloc(bytes_read);
            if (chunks[chunk_count].data == NULL) {
                fprintf(stderr, "Memory allocation error\n");
                exit(EXIT_FAILURE);
            }
            memcpy(chunks[chunk_count].data, buffer, bytes_read);
            memcpy(chunks[chunk_count].md5, md5, MD5_DIGEST_LENGTH);

            chunk_count++;
        }
    }
}

void undeduplicate_file(FILE *backup_file, Chunk **chunks, int *chunk_count) {
    // Exemple d'implémentation simple qui lit les chunks depuis le fichier de sauvegarde

    *chunk_count = 0;
    *chunks = malloc(1000 * sizeof(Chunk)); // Allouer de l'espace pour 1000 chunks (exemple)

    while (!feof(backup_file)) {
        Chunk *chunk = &((*chunks)[*chunk_count]);

        // Lire le MD5 et les données du chunk (exemple simplifié)
        fread(chunk->md5, 1, MD5_DIGEST_LENGTH, backup_file);
        chunk->data = malloc(CHUNK_SIZE);
        size_t read_size = fread(chunk->data, 1, CHUNK_SIZE, backup_file);

        if (read_size > 0) {
            (*chunk_count)++;
        } else {
            free(chunk->data);
        }
    }
}