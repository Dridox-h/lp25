#include "deduplication.h"
#include "file_handler.h"
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
    const EVP_MD *md = EVP_md5();

    EVP_DigestInit_ex(ctx, md, NULL);
    EVP_DigestUpdate(ctx, data, len);
    EVP_DigestFinal_ex(ctx, md5_out, NULL);

    EVP_MD_CTX_free(ctx);
}

// Fonction permettant de chercher un MD5 dans la table de hachage
int find_md5(Md5Entry *hash_table, unsigned char *md5) {
    unsigned int hash = hash_md5(md5);
    if (memcmp(hash_table[hash].md5, md5, MD5_DIGEST_LENGTH) == 0) {
        return hash_table[hash].index;
    }
    return -1;
}

// Ajouter un MD5 dans la table de hachage
void add_md5(Md5Entry *hash_table, unsigned char *md5, int index) {
    unsigned int hash = hash_md5(md5);
    memcpy(hash_table[hash].md5, md5, MD5_DIGEST_LENGTH);
    hash_table[hash].index = index;
}

// Fonction pour convertir un fichier non dédupliqué en tableau de chunks
void deduplicate_file(FILE *file, Chunk *chunks, Md5Entry *hash_table) {
    unsigned char buffer[CHUNK_SIZE];
    size_t bytes_read;
    int chunk_index = 0;

    while ((bytes_read = fread(buffer, 1, CHUNK_SIZE, file)) > 0) {
        // Calculer le MD5 du chunk
        unsigned char md5[MD5_DIGEST_LENGTH];
        compute_md5(buffer, bytes_read, md5);

        // Vérifier si ce chunk a déjà été ajouté dans la table de hachage
        int index = find_md5(hash_table, md5);
        if (index == -1) {
            // Ce chunk est unique, ajouter à la table de hachage et le stocker dans le tableau de chunks
            add_md5(hash_table, md5, chunk_index);
            chunks[chunk_index].data = malloc(bytes_read);
            memcpy(chunks[chunk_index].data, buffer, bytes_read);
            memcpy(chunks[chunk_index].md5, md5, MD5_DIGEST_LENGTH);
            chunk_index++;
        }
    }
}

// Fonction permettant de charger un fichier dédupliqué en table de chunks
void undeduplicate_file(FILE *file, Chunk **chunks, int *chunk_count) {
    unsigned char buffer[MD5_DIGEST_LENGTH];
    size_t bytes_read;

    // Le fichier dédupliqué contient des références (index) pour chaque chunk
    while ((bytes_read = fread(buffer, 1, MD5_DIGEST_LENGTH, file)) > 0) {
        unsigned char md5[MD5_DIGEST_LENGTH];
        memcpy(md5, buffer, MD5_DIGEST_LENGTH);  // Récupérer le MD5 du chunk

        // Trouver l'index du chunk à restaurer
        int index = find_md5(hash_md5, md5);  // Recherche dans la table de hachage
        if (index != -1) {
            // Utiliser les données du chunk comme nécessaire
            // Exemple : afficher les données
            printf("Restoring chunk %d with MD5: ", index);
            for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
                printf("%02x", md5[i]);
            }
            printf("\n");
        } else {
            fprintf(stderr, "Erreur : Chunk non trouvé pour le MD5 : ");
            for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
                printf("%02x", md5[i]);
            }
            printf("\n");
        }
    }
}
