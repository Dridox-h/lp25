#ifndef DEDUPLICATION_H
#define DEDUPLICATION_H

#include <stddef.h> // Pour size_t
#include <openssl/md5.h> // Pour les fonctions MD5

// Taille du hachage MD5
#define HASH_SIZE MD5_DIGEST_LENGTH

// Taille d'un chunk (bloc de données)
#define CHUNK_SIZE 4096

// Structure pour stocker les chunks et leur hachage
typedef struct {
    unsigned char hash[HASH_SIZE]; // Hachage MD5 du chunk
    long offset;                   // Offset dans le fichier de destination
} FileChunk;

/**
 * @brief Calcule le hachage MD5 d'un bloc de données.
 *
 * @param data Pointeur vers les données à hacher.
 * @param length Longueur des données à hacher.
 * @param md5_sum Pointeur vers un tableau où le hachage MD5 sera stocké.
 */
void calculate_md5(const unsigned char *data, size_t length, unsigned char *md5_sum);

/**
 * @brief Effectue la déduplication des fichiers en lisant le fichier source par chunks.
 *
 * @param source Chemin vers le fichier source à dédupliquer.
 * @param destination Chemin vers le fichier de destination où les données dédupliquées seront sauvegardées.
 */
void deduplicate_files(const char *source, const char *destination);

#endif // DEDUPLICATION_H