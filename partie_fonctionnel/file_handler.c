#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/sendfile.h>
#include <dirent.h>  // Ajouté pour opendir, readdir, closedir
#include <sys/time.h>  // Ajouté pour gettimeofday
#include <openssl/md5.h>
#include "file_handler.h"


// Fonction pour obtenir la date actuelle avec `gettimeofday`, `localtime` et `strftime`
void get_current_datetime(char *buffer, size_t size) {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    struct tm *tm_info = localtime(&tv.tv_sec);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", tm_info);
    snprintf(buffer + strlen(buffer), size - strlen(buffer), ".%03ld", tv.tv_usec / 1000);
}

// Fonction pour lire une ligne du fichier ".backup_log"
log_t read_backup_log(const char *logfile) {
    log_t logs = {NULL, NULL};
    FILE *file = fopen(logfile, "r");
    if (!file) {
        perror("Erreur d'ouverture du fichier de log");
        return logs;
    }

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        log_element *elt = (log_element *)malloc(sizeof(log_element));
        if (!elt) {
            perror("Erreur d'allocation mémoire");
            fclose(file);
            return logs;
        }

        // Analyser la ligne
        char *path = strtok(line, ";");
        char *date = strtok(NULL, ";");
        char *md5_str = strtok(NULL, "\n");

        elt->path = strdup(path);
        elt->date = strdup(date);
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
            sscanf(md5_str + (i * 2), "%2hhx", &elt->md5[i]);
        }

        elt->next = NULL;
        if (logs.tail) {
            logs.tail->next = elt;
            elt->prev = logs.tail;
        } else {
            logs.head = elt;
            elt->prev = NULL;
        }
        logs.tail = elt;
    }

    fclose(file);
    return logs;
}

// Fonction pour mettre à jour le fichier ".backup_log"
void update_backup_log(const char *logfile, log_t *logs) {
    FILE *file = fopen(logfile, "a");
    if (!file) {
        perror("Erreur d'ouverture du fichier de log pour mise à jour");
        return;
    }

    log_element *elt = logs->head;
    while (elt) {
        write_log_element(elt, file);
        elt = elt->next;
    }

    fclose(file);
}

// Fonction pour écrire un élément log dans le fichier ".backup_log"
void write_log_element(log_element *elt, FILE *logfile) {
    fprintf(logfile, "%s;%s;", elt->path, elt->date);

    // Convertir le MD5 en chaîne hexadécimale
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        fprintf(logfile, "%02x", elt->md5[i]);
    }

    fprintf(logfile, "\n");
}

// Fonction pour lister les fichiers dans un répertoire
void list_files(const char *path) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror("Erreur d'ouverture du répertoire");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.') { // Ignorer les fichiers/dossiers cachés
            printf("%s/%s\n", path, entry->d_name);
        }
    }

    closedir(dir);
}

// Fonction pour copier un fichier avec `sendfile`
void copy_file_with_sendfile(const char *src, const char *dest) {
    int source = open(src, O_RDONLY);
    if (source == -1) {
        perror("Erreur d'ouverture du fichier source");
        return;
    }

    int destination = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (destination == -1) {
        perror("Erreur d'ouverture du fichier de destination");
        close(source);
        return;
    }

    off_t offset = 0;
    struct stat stat_buf;
    fstat(source, &stat_buf);

    // Copier avec sendfile
    ssize_t bytes_sent = sendfile(destination, source, &offset, stat_buf.st_size);
    if (bytes_sent == -1) {
        perror("Erreur de transfert avec sendfile");
    }

    close(source);
    close(destination);
}

// Fonction pour supprimer un fichier avec `unlink`
void delete_file(const char *path) {
    if (unlink(path) == -1) {
        perror("Erreur lors de la suppression du fichier");
    } else {
        printf("Fichier supprimé avec succès : %s\n", path);
    }
}

// Fonction pour copier un fichier avec un lien dur
void copy_file_with_link(const char *src, const char *dest) {
    if (link(src, dest) == -1) {
        perror("Erreur lors de la création du lien dur");
    } else {
        printf("Lien dur créé avec succès : %s -> %s\n", src, dest);
    }
}

int main() {
    // Exemple d'utilisation des fonctions
    const char *src_file = "source.txt";    // Chemin vers le fichier source
    const char *dest_file = "destination.txt";  // Chemin vers le fichier de destination
    const char *logfile = ".backup_log";    // Fichier log

    // Copie du fichier avec `sendfile`
    copy_file_with_sendfile(src_file, dest_file);

    // Création d'un lien dur
    copy_file_with_link(src_file, "hard_link_to_source.txt");

    // Suppression du fichier source
    delete_file(src_file);

    // Lecture et mise à jour du fichier de log
    log_t logs = read_backup_log(logfile);
    update_backup_log(logfile, &logs);

    return 0;
}
