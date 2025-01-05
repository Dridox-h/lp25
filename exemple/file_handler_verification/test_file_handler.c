#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include "file_handler.h"

// Fonction pour calculer le MD5 d'un fichier
int calculate_file_md5(const char *filename, unsigned char *md5_result) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        perror("Erreur d'ouverture du fichier pour calculer le MD5");
        return -1;
    }

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (mdctx == NULL) {
        perror("Erreur d'allocation du contexte MD5");
        fclose(file);
        return -1;
    }

    // Initialiser l'algorithme MD5
    if (EVP_DigestInit_ex(mdctx, EVP_md5(), NULL) != 1) {
        perror("Erreur d'initialisation du digest");
        EVP_MD_CTX_free(mdctx);
        fclose(file);
        return -1;
    }

    unsigned char buffer[1024];
    int bytes_read;
    
    // Lire le fichier et mettre à jour le digest
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (EVP_DigestUpdate(mdctx, buffer, bytes_read) != 1) {
            perror("Erreur de mise à jour du digest");
            EVP_MD_CTX_free(mdctx);
            fclose(file);
            return -1;
        }
    }

    // Obtenir le résultat MD5
    unsigned int md5_len;
    if (EVP_DigestFinal_ex(mdctx, md5_result, &md5_len) != 1) {
        perror("Erreur de finalisation du digest");
        EVP_MD_CTX_free(mdctx);
        fclose(file);
        return -1;
    }

    // Libérer le contexte MD5 et fermer le fichier
    EVP_MD_CTX_free(mdctx);
    fclose(file);

    return 0;
}

// Fonction pour tester l'écriture d'un log avec update_backup_log
void test_update_backup_log() {
    // Créer un élément log à écrire
    log_element elt;
    elt.path = "folder1/file1";
    elt.date = "2025-01-05-14:30:25";  // Date simulée pour l'exemple

    // Utiliser un fichier temporaire pour tester le calcul du MD5
    const char *test_filename = "test_file.txt";

    // Créer un fichier temporaire avec du contenu
    FILE *temp_file = fopen(test_filename, "w");
    if (!temp_file) {
        perror("Erreur de création du fichier temporaire");
        return;
    }
    const char *content = "Exemple de contenu pour calculer MD5";
    fwrite(content, 1, strlen(content), temp_file);
    fclose(temp_file);

    // Calculer le MD5 du fichier
    unsigned char md5_result[MD5_DIGEST_LENGTH];
    if (calculate_file_md5(test_filename, md5_result) != 0) {
        return;
    }

    // Copier le MD5 dans l'élément log
    memcpy(elt.md5, md5_result, MD5_DIGEST_LENGTH);

    // Créer une structure log_t pour contenir l'élément log
    log_t logs = {NULL, NULL};
    log_element *new_element = malloc(sizeof(log_element));
    if (new_element == NULL) {
        perror("Erreur d'allocation mémoire");
        return;
    }

    // Copier les données de l'élément dans la structure log_t
    new_element->path = elt.path;
    new_element->date = elt.date;
    memcpy(new_element->md5, elt.md5, MD5_DIGEST_LENGTH);
    new_element->next = NULL;
    new_element->prev = logs.tail;

    // Ajouter l'élément dans la structure log_t
    if (logs.tail) {
        logs.tail->next = new_element;
    } else {
        logs.head = new_element;
    }
    logs.tail = new_element;

    // Utiliser update_backup_log pour écrire dans le fichier
    update_backup_log("test_backup_log.txt", &logs);

    // Libérer la mémoire allouée pour la structure log_t
    free(new_element);

    // Supprimer le fichier temporaire après test
    remove(test_filename);

    printf("Test terminé. Vérifiez le contenu de 'test_backup_log.txt'.\n");
}

// Fonction pour tester la lecture du fichier de logs
void test_read_backup_log() {
    // Lire les logs du fichier
    log_t logs = read_backup_log("test_backup_log.txt");

    // Vérifier que la lecture a fonctionné correctement
    log_element *current = logs.head;
    while (current) {
        printf("Log lu : %s;%s;%s\n", current->path, current->md5, current->date);
        current = current->next;
    }
}

int main() {
    // Lancer le test d'écriture
    test_update_backup_log();
    
    // Lancer le test de lecture
    test_read_backup_log();
    
    return 0;
}
