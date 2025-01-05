#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include "file_handler.h"
#include "deduplication.h"


// La structure du fichier backup est la suivante : 
// YYYY-MM-DD-hh:mm:ss.sss/folder1/file1;mtime;md5

// Fonction permettant de lire un élément du fichier .backup_log
log_t read_backup_log(const char *logfile) {
    log_t logs = {NULL, NULL};  // Initialisation de la liste chaînée
    FILE *file = fopen(logfile, "r");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier log");
        return logs;
    }

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        char date[30], path[256], mtime[30], md5_str[MD5_DIGEST_LENGTH * 2 + 1];
        unsigned char md5[MD5_DIGEST_LENGTH];

        // Lecture et découpage de la ligne
        if (sscanf(line, "%29[^/]/%255[^;];%29[^;];%32s", date, path, mtime, md5_str) != 4) {
            continue;  // Ignore si le format de ligne est incorrect
        }

        // Convertir la chaîne MD5 en tableau de bytes
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
            sscanf(&md5_str[i * 2], "%2hhx", &md5[i]);
        }

        // Créer un nouvel élément log
        log_element *new_elt = malloc(sizeof(log_element));
        if (!new_elt) {
            continue;  // Si l'allocation échoue, ignorer cette ligne
        }

        new_elt->path = path;
        new_elt->date = mtime;
        memcpy(new_elt->md5, md5, MD5_DIGEST_LENGTH);
        new_elt->next = NULL;
        new_elt->prev = logs.tail;

        // Ajouter l'élément à la liste
        if (!logs.head) {
            logs.head = new_elt;
        } else {
            logs.tail->next = new_elt;
        }
        logs.tail = new_elt;
    }

    fclose(file);
    return logs;
}

// Fonction permettant de mettre à jour une ligne du fichier .backup_log
void update_backup_log(const char *logfile, log_t *logs) {
    /* Implémenter la logique de modification du fichier ".backup_log" directement dans cette fonction
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

        // 1. Obtenir la date actuelle au format YYYY-MM-DD-hh:mm:ss.sss
        time_t t = time(NULL);
        struct tm tm_info;
        char date_str[30];
        localtime_r(&t, &tm_info);
        strftime(date_str, sizeof(date_str), "%Y-%m-%d-%H:%M:%S.", &tm_info);

        // Ajouter la partie millisecondes (ajusté selon le besoin)
        snprintf(date_str + 19, 4, "%03d", 123);  // Utiliser un temps d'exemple (123 ms)

        // 2. Conversion MD5 en chaîne hexadécimale
        char md5_str[2 * MD5_DIGEST_LENGTH + 1];
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
            sprintf(&md5_str[i * 2], "%02x", current->md5[i]);
        }

        // 3. Écrire dans le fichier log au format "date chemin;date_modification;md5"
        fprintf(file, "%s%s;%s;%s\n", date_str, current->path, current->date, md5_str);

        // Passer à l'élément suivant de la liste
        current = current->next; 
    }

    fclose(file);  // Fermer le fichier après avoir écrit toutes les lignes.
}


void write_log_element(log_element *elt, FILE *logfile){
  /* Implémenter la logique pour écrire un élément log de la liste chaînée log_element dans le fichier .backup_log
   * @param: elt - un élément log à écrire sur une ligne
   *         logfile - le chemin du fichier .backup_log
   */
    if (elt == NULL || logfile == NULL) {
        return;
    }

    // Obtenir la date actuelle au format YYYY-MM-DD-hh:mm:ss.sss
    time_t t = time(NULL);
    struct tm tm_info;
    char date_str[30];
    localtime_r(&t, &tm_info);
    strftime(date_str, sizeof(date_str), "%Y-%m-%d-%H:%M:%S.", &tm_info);

    // Ajouter la partie millisecondes (ajusté selon le besoin)
    snprintf(date_str + 19, 4, "%03d", 123);  // Utiliser un temps d'exemple (123 ms)

    // Conversion MD5 en chaîne hexadécimale et écriture dans le fichier log
    char md5_str[2 * MD5_DIGEST_LENGTH + 1];
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        sprintf(&md5_str[i * 2], "%02x", elt->md5[i]);
    }

    // Écrire dans le fichier log
    fprintf(logfile, "%s%s;%s;%s\n", date_str, elt->path, elt->date, md5_str);
}

void list_files(const char *path)
{
    DIR *dir;
    char full_path[PATH_MAX];
    struct dirent *entry;
    struct stat entry_stat;
         
    if (!(dir = opendir(path)))
    {
        perror("Erreur d'ouverture du répertoire");
        return;
    }
    while ((entry = readdir(dir)))
    {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        if (stat(full_path, &entry_stat) == -1)
        {
            fprintf(stderr, "Erreur lors de stat : %s\n", full_path);
            continue;
        }
        if (S_ISDIR(entry_stat.st_mode))
            list_files(full_path);
        else if (S_ISREG(entry_stat.st_mode))
            printf("Fichier : %s\n", full_path);
    }
    if (closedir(dir) == -1)
        perror("Erreur lors de la fermeture du répertoire");
}

