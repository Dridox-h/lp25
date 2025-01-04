#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>

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

// Fonction pour afficher le hash MD5 en format hexadécimal
void print_md5(unsigned char *md5) {
    for (int i = 0; i < EVP_MD_size(EVP_md5()); i++) {
        printf("%02x", md5[i]);
    }
    printf("\n");
}

// Programme principal pour tester la fonction
int main() {
    const char *test_data = "Bonjour, ceci est un test!";
    unsigned char md5_result[EVP_MAX_MD_SIZE]; // Taille max pour les hachages

    // Calcule le MD5
    compute_md5((void *)test_data, strlen(test_data), md5_result);

    // Affiche le résultat
    printf("MD5 pour '%s': ", test_data);
    print_md5(md5_result);

    return 0;
}
