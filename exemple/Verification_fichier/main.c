#include "deduplication.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    // Initialisation de la table de hachage
    Md5Entry hash_table[HASH_TABLE_SIZE] = {0}; // La table de hachage est initialisée à zéro

    // Ouverture du fichier à tester
    const char *filename = "test_file.txt";  // Changez le chemin de fichier si nécessaire
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier");
        return 1;
    }

    // Création d'un tableau de chunks pour stocker les chunks extraits du fichier
    Chunk chunks[1000];  // Allouer assez de place pour stocker 1000 chunks

    // Appel de la fonction de déduplication pour convertir le fichier en chunks
    deduplicate_file(file, chunks, hash_table);
    fclose(file);  // Fermer le fichier après traitement

    // Affichage des résultats
    printf("Chunks dédupliqués :\n");
    for (int i = 0; i < 1000; i++) {
        if (chunks[i].data != NULL) { // Vérifier si le chunk est valide
            printf("Chunk %d : MD5 : ", i);
            for (int j = 0; j < MD5_DIGEST_LENGTH; j++) {
                printf("%02x", chunks[i].md5[j]);
            }
            printf("\n");
        }
    }

    // Tester la fonction de recherche MD5
    unsigned char test_md5[MD5_DIGEST_LENGTH];
    compute_md5("Some test data", strlen("Some test data"), test_md5);

    int index = find_md5(hash_table, test_md5);
    if (index != -1) {
        printf("Le MD5 a été trouvé à l'index %d.\n", index);
    } else {
        printf("Le MD5 n'a pas été trouvé.\n");
    }

    return 0;
}
