#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include "file_handler.h"
#include "deduplication.h"


// La structure du fichier backup est la suivante : 
// YYYY-MM-DD-hh:mm:ss.sss/folder1/file1;mtime;md5

// Fonction permettant de lire un élément du fichier .backup_log
log_t read_backup_log(const char *logfile){
    /* Implémenter la logique pour la lecture d'une ligne du fichier ".backup_log"
    * @param: logfile - le chemin vers le fichier .backup_log
    * @return: une structure log_t
    */
    log_t logs = {NULL, NULL}; // Initialise une liste vide pour stocker les logs.
    FILE *file = fopen(logfile, "r"); 
    if (!file) { 
        perror("Erreur d'ouverture du fichier de log"); 
        return logs; // Retourne une liste vide en cas d'erreur.
    }

    char line[1024]; // Buffer pour stocker une ligne du fichier.
    while (fgets(line, sizeof(line), file)) { // Lit chaque ligne du fichier jusqu'à la fin.
        log_element *new_element = malloc(sizeof(log_element)); // Alloue un nouvel élément pour la liste chaînée.
        if (!new_element) { 
            perror("Erreur d'allocation mémoire");
            fclose(file); 
            return logs;
        }

        // Découpage de la ligne en trois parties : chemin, MD5 et date.
        char *token = strtok(line, ";"); 
        new_element->path = strdup(token); // Copie le chemin dans l'élément.

        token = strtok(NULL, ";");
        memcpy(new_element->md5, token, MD5_DIGEST_LENGTH); // Copie le MD5 dans l'élément.

        token = strtok(NULL, "\n");
        new_element->date = strdup(token); // Copie la date dans l'élément.

        // Ajoute le nouvel élément à la liste chaînée.
        new_element->next = NULL; // L'élément suivant est NULL (fin de liste pour l'instant).
        new_element->prev = logs.tail; // Le précédent élément devient la queue actuelle.

        if (logs.tail) {
            logs.tail->next = new_element; // Lie l'élément précédent au nouvel élément.
        } else {
            logs.head = new_element; // Si la liste est vide, le nouvel élément devient la tête.
        }
        logs.tail = new_element; // Met à jour la queue avec le nouvel élément.
    }

    fclose(file); 
    return logs; // Retourne la liste chaînée des logs.
}


// Fonction permettant de mettre à jour une ligne du fichier .backup_log
void update_backup_log(const char *logfile, log_t *logs){
  /* Implémenter la logique de modification d'une ligne du fichier ".backup_log"
  * @param: logfile - le chemin vers le fichier .backup_log
  *         logs - qui est la liste de toutes les lignes du fichier .backup_log sauvegardée dans une structure log_t
  */
    FILE *file = fopen(logfile, "w"); // Ouvre le fichier de log en mode écriture.
    if (!file) { 
        perror("Erreur d'ouverture du fichier de log");
        return;
    }

    log_element *current = logs->head; // Commence par le premier élément de la liste.
    while (current) { // Parcourt tous les éléments de la liste chaînée.
        // Écrit les informations de l'élément actuel dans le fichier.
        fprintf(file, "%s;%s;%s\n", current->path, current->md5, current->date);
        current = current->next; 
    }

    fclose(file); 
}


void write_log_element(log_element *elt, FILE *logfile){
  /* Implémenter la logique pour écrire un élément log de la liste chaînée log_element dans le fichier .backup_log
   * @param: elt - un élément log à écrire sur une ligne
   *         logfile - le chemin du fichier .backup_log
   */
    fprintf(logfile, "%s;%s;", elt->path, elt->date);

    // Convertir le MD5 en chaîne hexadécimale
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        fprintf(logfile, "%02x", elt->md5[i]);
    }

    fprintf(logfile, "\n");
}

void list_files(const char *path){
  /* Implémenter la logique pour lister les fichiers présents dans un répertoire
  */
}

