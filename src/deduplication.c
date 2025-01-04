#include "deduplication.h"
#include "file_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include <dirent.h>

// Fonction de hachage MD5 pour l'indexation
// dans la table de hachage
unsigned int hash_md5(unsigned char *md5) {
    unsigned int hash = 0;
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        hash = (hash << 5) + hash + md5[i];
    }
    return hash % HASH_TABLE_SIZE;
}

// Fonction pour calculer le MD5 d'un chunk
void compute_md5(void *data, size_t len, unsigned char *md5_out) {
}

// Fonction permettant de chercher un MD5 dans la table de hachage
int find_md5(Md5Entry *hash_table, unsigned char *md5) {
    /* @param: hash_table est le tableau de hachage qui contient les MD5 et l'index des chunks unique
    *           md5 est le md5 du chunk dont on veut déterminer l'unicité
    *  @return: retourne l'index s'il trouve le md5 dans le tableau et -1 sinon
    */
    
}

// Ajouter un MD5 dans la table de hachage
void add_md5(Md5Entry *hash_table, unsigned char *md5, int index) {
}

// Fonction pour convertir un fichier non dédupliqué en tableau de chunks
void deduplicate_file(FILE *file, Chunk *chunks, Md5Entry *hash_table){
    /* @param:  file est le fichier qui sera dédupliqué
    *           chunks est le tableau de chunks initialisés qui contiendra les chunks issu du fichier
    *           hash_table est le tableau de hachage qui contient les MD5 et l'index des chunks unique
    */

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
// en remplaçant les références par les données correspondantes
void undeduplicate_file(FILE *file, Chunk **chunks, int *chunk_count) {
    /* @param: file est le nom du fichier dédupliqué présent dans le répertoire de sauvegarde
    *           chunks représente le tableau de chunk qui contiendra les chunks restauré depuis filename
    *           chunk_count est un compteur du nombre de chunk restauré depuis le fichier filename
    */
    if (!file) {
        fprintf(stderr, "Le fichier fourni est invalide.\n");
        return;
    }

    // Lit le nombre total de chunks depuis le fichier.
    if (fread(chunk_count, sizeof(int), 1, file) != 1) {
        fprintf(stderr, "Erreur lors de la lecture du nombre de chunks.\n");
        return;
    }

    *chunks = malloc(*chunk_count * sizeof(Chunk));
    if (!*chunks) {
        fprintf(stderr, "Erreur d'allocation mémoire pour les chunks.\n");
        return;
    }

    // Parcourt chaque chunk pour les charger depuis le fichier.
    for (int i = 0; i < *chunk_count; i++) {
        // Lit le MD5 du chunk.
        if (fread((*chunks)[i].md5, MD5_DIGEST_LENGTH, 1, file) != 1) {
            fprintf(stderr, "Erreur lors de la lecture du MD5 du chunk %d.\n", i);
            free(*chunks); 
            return;
        }

        // Alloue de la mémoire pour les données du chunk.
        (*chunks)[i].data = malloc(CHUNK_SIZE);
        if (!(*chunks)[i].data) {
            fprintf(stderr, "Erreur d'allocation mémoire pour les données du chunk %d.\n", i);
            free(*chunks); 
            return;
        }

        // Lit les données du chunk.
        if (fread((*chunks)[i].data, 1, CHUNK_SIZE, file) != CHUNK_SIZE) {
            fprintf(stderr, "Erreur lors de la lecture des données du chunk %d.\n", i);
            free((*chunks)[i].data); 
            free(*chunks); 
            return;
        }
    }

    printf("Restauration réussie de %d chunks.\n", *chunk_count);
}