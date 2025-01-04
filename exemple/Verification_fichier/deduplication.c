#include "deduplication.h"
#include "file_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
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
        if (data == NULL || md5_out == NULL) {
        fprintf(stderr, "Données ou sortie MD5 invalides\n");
        return;
    }

    EVP_MD_CTX *ctx = EVP_MD_CTX_new(); 
    if (ctx == NULL) {
        fprintf(stderr, "Erreur de création du contexte EVP\n");
        return;
    }

    if (EVP_DigestInit_ex(ctx, EVP_md5(), NULL) != 1) {
        fprintf(stderr, "Erreur d'initialisation de l'algorithme MD5\n");
        EVP_MD_CTX_free(ctx);
        return;
    }

    if (EVP_DigestUpdate(ctx, data, len) != 1) {
        fprintf(stderr, "Erreur lors de la mise à jour du hachage\n");
        EVP_MD_CTX_free(ctx);
        return;
    }

    unsigned int md5_length = 0;
    if (EVP_DigestFinal_ex(ctx, md5_out, &md5_length) != 1) {
        fprintf(stderr, "Erreur lors de la finalisation du hachage\n");
        EVP_MD_CTX_free(ctx);
        return;
    }

    EVP_MD_CTX_free(ctx);
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
    if (!hash_table || !md5) {
        fprintf(stderr, "Table de hachage ou MD5 invalide\n");
        return;
    }

    // Calcul de l'index de hachage
    unsigned int hash = hash_md5(md5);

    // Créer un nouvel élément
    Md5Entry *new_entry = (Md5Entry *)malloc(sizeof(Md5Entry));
    if (!new_entry) {
        perror("Erreur d'allocation mémoire pour un nouvel élément");
        return;
    }

    memcpy(new_entry->md5, md5, MD5_DIGEST_LENGTH);
    new_entry->index = index;
    new_entry->next = NULL;

    // Ajouter à la table de hachage
    if (hash_table[hash].next == NULL) {
        // Aucun élément à cet index, ajouter directement
        hash_table[hash] = *new_entry;
        free(new_entry);
    } else {
        // Gestion des collisions : insertion en début de liste
        new_entry->next = hash_table[hash].next;
        hash_table[hash].next = new_entry;
    }
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
}

